/** BaseScreen.cc file for the fluxbox compositor. */

// Copyright (c) 2011 Gediminas Liktaras (gliktaras at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "BaseScreen.hh"

#include "CompositorConfig.hh"
#include "Logging.hh"

#include "FbTk/App.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#ifdef XINERAMA
    #include <X11/extensions/Xinerama.h>
#endif  // XINERAMA

#include <algorithm>
#include <sstream>
#include <ostream>

using namespace FbCompositor;


//--- MACROS -------------------------------------------------------------------

// Macro for plugin iteration.
#define forEachPlugin(i, plugin)                                               \
    (plugin) = ((pluginManager().plugins().size() > 0)                         \
                   ? ((pluginManager().plugins()[0]))                          \
                   : NULL);                                                    \
    for(size_t (i) = 0;                                                        \
        ((i) < pluginManager().plugins().size());                              \
        (i)++,                                                                 \
        (plugin) = (((i) < pluginManager().plugins().size())                   \
                       ? (pluginManager().plugins()[(i)])                      \
                       : NULL))


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseScreen::BaseScreen(int screenNumber, PluginType pluginType, const CompositorConfig &config) :
    m_display(FbTk::App::instance()->display()),
    m_pluginManager(pluginType, *this),
    m_screenNumber(screenNumber),
    m_rootWindow(*this, XRootWindow(m_display, m_screenNumber)) {   // TODO: Is passing half-initialized *this safe here?

    // Set up properties.
    m_activeWindowXID = m_rootWindow.singlePropertyValue<Window>(Atoms::activeWindowAtom(), 0);
    m_currentWorkspace = m_rootWindow.singlePropertyValue<long>(Atoms::workspaceAtom(), 0);
    m_reconfigureRect.x = m_reconfigureRect.y = m_reconfigureRect.width = m_reconfigureRect.height = 0;
    m_workspaceCount = m_rootWindow.singlePropertyValue<long>(Atoms::workspaceCountAtom(), 1);

    // Set up plugins.
    for(size_t i = 0; i < config.plugins().size(); i++) {
        m_pluginManager.createPluginObject(config.plugins()[i].first, config.plugins()[i].second);
    }

    // Set up root window.
    long eventMask = PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask;
    m_rootWindow.setEventMask(eventMask);

    XCompositeRedirectSubwindows(m_display, m_rootWindow.window(), CompositeRedirectManual);

    // Initial head init.
    initHeads(Heads_One);
}

// Destructor.
BaseScreen::~BaseScreen() {
    std::list<BaseCompWindow*>::iterator it = m_windows.begin();
    while (it != m_windows.end()) {
        delete *it;
        ++it;
    }
}


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initializes heads on the current screen.
void BaseScreen::initHeads(HeadMode headMode) {
    m_heads.clear();

#ifdef XINERAMA
    if (headMode == Heads_Xinerama) {
        int nHeads;
        XineramaScreenInfo *xHeads = XineramaQueryScreens(display(), &nHeads);  // Are the screens in order?

        m_heads.reserve(nHeads);
        for (int i = 0; i < nHeads; i++) {
            XRectangle h = { xHeads[i].x_org, xHeads[i].y_org, xHeads[i].width, xHeads[i].height };
            m_heads.push_back(h);
        }

        if (xHeads) {
            XFree(xHeads);
        }
    } else
#endif  // XINERAMA

    if (headMode == Heads_One) {
        XRectangle h = { 0, 0, rootWindow().width(), rootWindow().height() };
        m_heads.push_back(h);
    } else {
        throw InitException("Unknown screen head mode given.");
    }
}

// Initializes all of the windows on the screen.
void BaseScreen::initWindows() {
    Window root;
    Window parent;
    Window *children = 0;
    unsigned int childCount;

    XQueryTree(display(), rootWindow().window(), &root, &parent, &children, &childCount);
    for (unsigned int i = 0; i < childCount; i++) {
        createWindow(children[i]);
    }

    if (children) {
        XFree(children);
    }
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Creates a new window and inserts it into the list of windows.
void BaseScreen::createWindow(Window window) {
    if (isWindowIgnored(window)) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it == m_windows.end()) {
        BaseCompWindow *newWindow = NULL;
        try {
            newWindow = createWindowObject(window);
        } catch(const InitException &e) {
            std::stringstream ss;
            ss << "Could not create window " << std::hex << window << " (" << e.what() << ")";
            throw WindowException(ss.str());
        }

        if (newWindow->depth() == 0) {
            delete newWindow;
            return;     // If the window is already destroyed.
        }

        newWindow->setEventMask(PropertyChangeMask);
        m_windows.push_back(newWindow);

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowCreated(*newWindow);
        }
    } else {
        fbLog_warn << "Attempted to create a window twice (" << std::hex << window << ")" << std::endl;
    }
}

// Damages a window on this screen.
void BaseScreen::damageWindow(Window window) {
    if (isWindowIgnored(window)) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->addDamage();

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowDamaged(**it);
        }
    } else {
        if (window != m_rootWindow.window()) {
            fbLog_warn << "Attempted to damage an untracked window (" << std::hex << window << ")" << std::endl;
        }
    }
}

// Destroys a window on this screen.
void BaseScreen::destroyWindow(Window window) {
    if (isWindowIgnored(window)) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowDestroyed(**it);
        }

        delete *it;
        m_windows.erase(it);
    } else {
        fbLog_warn << "Attempted to destroy an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Maps a window on this screen.
void BaseScreen::mapWindow(Window window) {
    if (isWindowIgnored(window)) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setMapped();

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowMapped(**it);
        }
    } else {
        fbLog_warn << "Attempted to map an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Updates window's configuration.
void BaseScreen::reconfigureWindow(const XConfigureEvent &event) {
    if (isWindowIgnored(event.window)) {
        return;
    }

    if (event.window == m_rootWindow.window()) {
        m_rootWindow.updateGeometry(event);
        setRootWindowSizeChanged();

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowReconfigured(m_rootWindow);
        }

        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(event.window);
    if (it != m_windows.end()) {
        (*it)->updateGeometry(event);

        BaseCompWindow *currentWindow = *it;
        m_windows.erase(it);

        if (event.above == None) {
            m_windows.push_front(currentWindow);
        } else {
            it = getWindowIterator(event.above);
            if (it == m_windows.end()) {
                it = getFirstManagedAncestorIterator(getParentWindow(currentWindow->window()));
            }

            if (it != m_windows.end()) {
                ++it;
                m_windows.insert(it, currentWindow);
            } else {
                m_windows.push_back(currentWindow);
            }
        }

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowReconfigured(**it);
        }
    } else {
        fbLog_warn << "Attempted to reconfigure an untracked window (" << std::hex << event.window << ")" << std::endl;
    }
}

// Reparents a window.
void BaseScreen::reparentWindow(Window window, Window parent) {
    if (parent == rootWindow().window()) {
        createWindow(window);
    } else {
        destroyWindow(window);
    }
}

// Updates window's shape.
void BaseScreen::updateShape(Window window) {
    if (isWindowIgnored(window)) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setClipShapeChanged();

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowShapeChanged(**it);
        }
    } else {
        fbLog_warn << "Attempted to update the shape of an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Unmaps a window on this screen.
void BaseScreen::unmapWindow(Window window) {
    if (isWindowIgnored(window)) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setUnmapped();

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowUnmapped(**it);
        }
    } else {
        fbLog_warn << "Attempted to unmap an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Updates the value of some window's property.
void BaseScreen::updateWindowProperty(Window window, Atom property, int state) {
    if (isWindowIgnored(window)) {
        return;
    }
    
    if ((window == m_rootWindow.window()) && (property != None) && (state == PropertyNewValue)) {
        if (property == Atoms::activeWindowAtom()) {
            Window activeWindow = m_rootWindow.singlePropertyValue<Window>(Atoms::activeWindowAtom(), None);
            std::list<BaseCompWindow*>::iterator it = getFirstManagedAncestorIterator(activeWindow);

            if (it != m_windows.end()) {
                m_activeWindowXID = (*it)->window();
            } else {
                m_activeWindowXID = None;
            }
        } else if (property == Atoms::reconfigureRectAtom()) {
            std::vector<long> data = m_rootWindow.propertyValue<long>(Atoms::reconfigureRectAtom());
            if (data.size() != 4) {
                m_reconfigureRect.x = m_reconfigureRect.y = m_reconfigureRect.width = m_reconfigureRect.height = 0;
            } else {
                m_reconfigureRect.x = data[0];
                m_reconfigureRect.y = data[1];
                m_reconfigureRect.width = data[2];
                m_reconfigureRect.height = data[3];
            }
        } else if (property == Atoms::rootPixmapAtom()) {
            setRootPixmapChanged();
        } else if (property == Atoms::workspaceAtom()) {
            m_currentWorkspace = m_rootWindow.singlePropertyValue<long>(Atoms::workspaceAtom(), 0);
        } else if (property == Atoms::workspaceCountAtom()) {
            m_workspaceCount = m_rootWindow.singlePropertyValue<long>(Atoms::workspaceCountAtom(), 1);
        }

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowPropertyChanged(m_rootWindow, property, state);
        }
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->updateProperty(property, state);

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowPropertyChanged(**it, property, state);
        }
    } else {
        if (window != rootWindow().window()) {
            fbLog_warn << "Attempted to set the property of an untracked window (" << std::hex << window << ")" << std::endl;
        }
    }
}


// Adds a window to ignore list, stops tracking it if it is being tracked.
void BaseScreen::addWindowToIgnoreList(Window window) {
    if (!isWindowIgnored(window)) {
        m_ignoreList.push_back(window);

        std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
        if (it != m_windows.end()) {
            delete *it;
            m_windows.erase(it);
        }
    }
}

// Checks whether a given window is managed by the current screen.
bool BaseScreen::isWindowManaged(Window window) {
    return (getWindowIterator(window) != m_windows.end());
}


//--- SCREEN MANIPULATION ----------------------------------------------

// Notifies the screen of the background change.
void BaseScreen::setRootPixmapChanged() {
    BasePlugin *plugin = NULL;
    forEachPlugin(i, plugin) {
        plugin->setRootPixmapChanged();
    }
}

// Notifies the screen of a root window change.
void BaseScreen::setRootWindowSizeChanged() {
    BasePlugin *plugin = NULL;
    forEachPlugin(i, plugin) {
        plugin->setRootWindowSizeChanged();
    }
}


//--- CONVENIENCE FUNCTIONS ----------------------------------------------------

// Returns the parent of a given window.
Window BaseScreen::getParentWindow(Window window) {
    Window root;
    Window parent;
    Window *children = 0;
    unsigned int childCount;

    XQueryTree(display(), window, &root, &parent, &children, &childCount);
    if (children) {
        XFree(children);
    }

    return parent;
}

// Returns an iterator of m_windows that points to the given window.
std::list<BaseCompWindow*>::iterator BaseScreen::getWindowIterator(Window window) {
    std::list<BaseCompWindow*>::iterator it = m_windows.begin();
    while (it != m_windows.end()) {
        if (window == (*it)->window()) {
            break;
        }
        ++it;
    }
    return it;
}

// Returns the first managed ancestor of a window.
std::list<BaseCompWindow*>::iterator BaseScreen::getFirstManagedAncestorIterator(Window window) {
    if (window == None) {
        return m_windows.end();
    }

    Window currentWindow = window;
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);

    while (it == m_windows.end()) {
        currentWindow = getParentWindow(currentWindow);
        if ((currentWindow == None) || (currentWindow == rootWindow().window())) {
            return m_windows.end();
        }
        it = getWindowIterator(currentWindow);
    }
    return it;
}

// Returns whether the given window is in the ignore list.
bool BaseScreen::isWindowIgnored(Window window) {
    return (find(m_ignoreList.begin(), m_ignoreList.end(), window) != m_ignoreList.end());
}


//--- FRIEND OPERATORS -------------------------------------------------

// << output stream operator for the BaseScreen class.
std::ostream &FbCompositor::operator<<(std::ostream& out, const BaseScreen& s) {
    out << "SCREEN NUMBER " << std::dec << s.m_screenNumber << ":" << std::endl
        << "  Properties" << std::endl
        << "    Active window XID: " << std::hex << s.m_activeWindowXID << std::endl
        << "    Number of workspaces: " << std::dec << s.m_workspaceCount << std::endl
        << "    Current workspace: " << std::dec << s.m_currentWorkspace << std::endl
        << "  Windows" << std::endl;

    std::list<BaseCompWindow*>::const_iterator it = s.m_windows.begin();
    while (it != s.m_windows.end()) {
        if ((*it)->isMapped()) {
            out << "    " << **it << std::endl;
        }
        ++it;
    }

    out << "  Ignore list" << std::endl << "    ";
    std::vector<Window>::const_iterator it2 = s.m_ignoreList.begin();
    while (it2 != s.m_ignoreList.end()) {
        out << std::hex << *it2 << " ";
        ++it2;
    }
    out << std::endl;

    return out;
}
