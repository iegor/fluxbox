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


#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif  // HAVE_CONFIG_H

#include "BaseScreen.hh"

#include "Atoms.hh"
#include "BasePlugin.hh"
#include "CompositorConfig.hh"
#include "Logging.hh"
#include "Utility.hh"

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
BaseScreen::BaseScreen(int screenNumber, PluginType pluginType, const CompositorConfig &/*config*/) :
    m_display(FbTk::App::instance()->display()),
    m_pluginManager(pluginType, *this),
    m_screenNumber(screenNumber),
    m_rootWindow(*this, XRootWindow(m_display, m_screenNumber), false) {

    m_screenDamage = XFixesCreateRegion(display(), NULL, 0);

    m_activeWindowXID = None;
    m_currentIconbarItem = None;
    updateCurrentWorkspace();
    updateReconfigureRect();
    updateWorkspaceCount();

    m_rootWindowPixmap = None;
    m_wmSetRootWindowPixmap = true;
    updateRootWindowPixmap();

    long eventMask = PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask;
    m_rootWindow.setEventMask(eventMask);

    XCompositeRedirectSubwindows(m_display, m_rootWindow.window(), CompositeRedirectManual);

    updateHeads(Heads_One);
}

// Destructor.
BaseScreen::~BaseScreen() {
    if (m_screenDamage) {
        XFixesDestroyRegion(display(), m_screenDamage);
    }

    std::list<BaseCompWindow*>::iterator it = m_windows.begin();
    while (it != m_windows.end()) {
        delete *it;
        ++it;
    }
}


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initializes the screen's plugins.
void BaseScreen::initPlugins(const CompositorConfig &config) {
    for(int i = 0; i < config.pluginCount(); i++) {
        m_pluginManager.createPluginObject(config.pluginName(i), config.pluginArgs(i));
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

    updateActiveWindow();
    updateCurrentIconbarItem();
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Circulates a window on this screen.
void BaseScreen::circulateWindow(Window window, int place) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        BaseCompWindow *curWindow = *it;
        m_windows.erase(it);

        if (place == PlaceOnTop) {
            m_windows.push_back(curWindow);
        } else {
            m_windows.push_front(curWindow);
        }

        if (!curWindow->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowCirculated(*curWindow, place);
            }
        }
    } else {
        if (window != m_rootWindow.window()) {
            fbLog_info << "Attempted to circulate an untracked window (" << std::hex << window << ")" << std::endl;
        }
    }
}

// Creates a new window and inserts it into the list of windows.
void BaseScreen::createWindow(Window window) {
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

        newWindow->setEventMask(PropertyChangeMask);
        m_windows.push_back(newWindow);

        if (newWindow->depth() == 0) {      // If the window is already destroyed, do not render it.
            newWindow->setIgnored(true);
        }
        if (isWindowIgnored(window)) {
            newWindow->setIgnored(true);
        } 
        
        if (!newWindow->isIgnored()) {
            damageScreenArea(newWindow->dimensions());

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowCreated(*newWindow);
            }
        }
    } else {
        fbLog_info << "Attempted to create a window twice (" << std::hex << window << ")" << std::endl;
    }
}

// Damages a window on this screen.
void BaseScreen::damageWindow(Window window, const XRectangle &area) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->addDamage();

        if (!(*it)->isIgnored()) {
            damageWindowArea((*it), area);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowDamaged(**it);
            }
        }
    } else {
        if (window != m_rootWindow.window()) {
            fbLog_info << "Attempted to damage an untracked window (" << std::hex << window << ")" << std::endl;
        }
    }
}

// Destroys a window on this screen.
void BaseScreen::destroyWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowDestroyed(**it);
            }
        }

        delete *it;
        m_windows.erase(it);
    } else {
        fbLog_info << "Attempted to destroy an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Maps a window on this screen.
void BaseScreen::mapWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setMapped();

        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowMapped(**it);
            }
        }
    } else {
        fbLog_info << "Attempted to map an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Updates window's configuration.
void BaseScreen::reconfigureWindow(const XConfigureEvent &event) {
    if (event.window == m_rootWindow.window()) {
        m_rootWindow.updateGeometry();
        setRootWindowSizeChanged();

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowReconfigured(m_rootWindow);
        }
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(event.window);
    if (it != m_windows.end()) {
        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);
        }

        (*it)->updateGeometry();
        restackWindow(it, event.above);

        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowReconfigured(**it);
            }
        }
    } else {
        fbLog_info << "Attempted to reconfigure an untracked window (" << std::hex << event.window << ")" << std::endl;
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
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setClipShapeChanged();

        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowShapeChanged(**it);
            }
        }
    } else {
        fbLog_info << "Attempted to update the shape of an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Unmaps a window on this screen.
void BaseScreen::unmapWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setUnmapped();

        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowUnmapped(**it);
            }
        }
    } else {
        fbLog_info << "Attempted to unmap an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Updates the value of some window's property.
void BaseScreen::updateWindowProperty(Window window, Atom property, int state) {
    if ((window == m_rootWindow.window()) && (property != None) && (state == PropertyNewValue)) {
        if (property == Atoms::activeWindowAtom()) {
            updateActiveWindow();
        } else if (property == Atoms::currentIconbarItemAtom()) {
            updateCurrentIconbarItem();
        } else if (property == Atoms::reconfigureRectAtom()) {
            damageReconfigureRect();    // Damage, so that previous rectangle can be removed.
            updateReconfigureRect();
            damageReconfigureRect();    // Damage, so that new rectangle can be drawn.
        } else if (property == Atoms::workspaceAtom()) {
            updateCurrentWorkspace();
        } else if (property == Atoms::workspaceCountAtom()) {
            updateWorkspaceCount();
        }

        std::vector<Atom> rootPixmapAtoms = Atoms::rootPixmapAtoms();
        for (size_t i = 0; i < rootPixmapAtoms.size(); i++) {
            if (property == rootPixmapAtoms[i]) {
                Pixmap newRootPixmap = m_rootWindow.singlePropertyValue<Pixmap>(rootPixmapAtoms[i], None);
                updateRootWindowPixmap(newRootPixmap);
                setRootPixmapChanged();     // We don't want this called in the constructor, keep it here.
            }
        }

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowPropertyChanged(m_rootWindow, property, state);
        }

    } else {
        std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
        if (it != m_windows.end()) {
            (*it)->updateProperty(property, state);

            if (!(*it)->isIgnored()) {
                if (property == Atoms::opacityAtom()) {
                    damageWholeWindowArea(*it);
                }

                BasePlugin *plugin = NULL;
                forEachPlugin(i, plugin) {
                    plugin->windowPropertyChanged(**it, property, state);
                }
            }
        } else {
            if (window != rootWindow().window()) {
                fbLog_info << "Attempted to set the property of an untracked window (" << std::hex << window << ")" << std::endl;
            }
        }
    }
}


// Marks a particular window as ignored.
void BaseScreen::ignoreWindow(Window window) {
    if (isWindowIgnored(window)) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setIgnored(true);

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowBecameIgnored(**it);
        }
    }

    m_ignoreList.push_back(window);
}

// Checks whether a given window is managed by the current screen.
bool BaseScreen::isWindowManaged(Window window) {
    return (getWindowIterator(window) != m_windows.end());
}


//--- SCREEN MANIPULATION ------------------------------------------------------

// Removes all accumulated damage from the screen.
void BaseScreen::clearScreenDamage() {
    m_damagedScreenRects.clear();
}

// Reconfigure heads on the current screen.
void BaseScreen::updateHeads(HeadMode headMode) {
    m_heads.clear();

#ifdef XINERAMA
    if (headMode == Heads_Xinerama) {
        int nHeads;
        XineramaScreenInfo *xHeads = XineramaQueryScreens(display(), &nHeads);

        m_heads.reserve(nHeads);
        for (int i = 0; i < nHeads; i++) {
            XRectangle head = { xHeads[i].x_org, xHeads[i].y_org, xHeads[i].width, xHeads[i].height };
            m_heads.push_back(head);
        }

        if (xHeads) {
            XFree(xHeads);
        }
    } else
#endif  // XINERAMA

    if (headMode == Heads_One) {
        XRectangle head = { 0, 0, rootWindow().width(), rootWindow().height() };
        m_heads.push_back(head);
    } else {
        throw InitException("Unknown screen head mode given.");
    }
}


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


//--- ACCESSORS ----------------------------------------------------------------

// Returns the damaged screen area.
XserverRegion BaseScreen::damagedScreenArea() {
    XFixesSetRegion(display(), m_screenDamage, (XRectangle*)(m_damagedScreenRects.data()), m_damagedScreenRects.size());
    return m_screenDamage;
}


//--- PROPERTY UPDATE FUNCTIONS ------------------------------------------------

// Update stored active window.
void BaseScreen::updateActiveWindow() {
    Window activeWindow = m_rootWindow.singlePropertyValue<Window>(Atoms::activeWindowAtom(), None);
    std::list<BaseCompWindow*>::iterator it = getFirstManagedAncestorIterator(activeWindow);

    if (it != m_windows.end()) {
        m_activeWindowXID = (*it)->window();
    } else {
        m_activeWindowXID = None;
    }
}

// Update the current iconbar item.
void BaseScreen::updateCurrentIconbarItem() {
    Window currentItem = m_rootWindow.singlePropertyValue<Window>(Atoms::currentIconbarItemAtom(), None);
    std::list<BaseCompWindow*>::iterator it = getFirstManagedAncestorIterator(currentItem);

    if (it != m_windows.end()) {
        m_currentIconbarItem = (*it)->window();
    } else {
        m_currentIconbarItem = None;
    }
}

// Update the current workspace index.
void BaseScreen::updateCurrentWorkspace() {
    m_currentWorkspace = m_rootWindow.singlePropertyValue<long>(Atoms::workspaceAtom(), 0);
}

// Update stored reconfigure rectangle.
void BaseScreen::updateReconfigureRect() {
    std::vector<long> data = m_rootWindow.propertyValue<long>(Atoms::reconfigureRectAtom());

    if (data.size() != 4) {
        m_reconfigureRect.x = m_reconfigureRect.y = m_reconfigureRect.width = m_reconfigureRect.height = 0;
    } else {
        m_reconfigureRect.x = data[0];
        m_reconfigureRect.y = data[1];
        m_reconfigureRect.width = data[2];
        m_reconfigureRect.height = data[3];
    }
}

// Update stored root window pixmap.
void BaseScreen::updateRootWindowPixmap(Pixmap newPixmap) {
    if (m_rootWindowPixmap && !m_wmSetRootWindowPixmap) {
        XFreePixmap(display(), m_rootWindowPixmap);
        m_rootWindowPixmap = None;
    }

    if (!newPixmap) {
        m_rootWindowPixmap = rootWindow().firstSinglePropertyValue<Pixmap>(Atoms::rootPixmapAtoms(), None);
    } else {
        m_rootWindowPixmap = newPixmap;
    }
    m_wmSetRootWindowPixmap = true;

    if (!m_rootWindowPixmap) {
        fbLog_info << "Cannot find background pixmap, using plain black." << std::endl;
        m_rootWindowPixmap = createSolidPixmap(*this, rootWindow().width(), rootWindow().height(), 0x00000000);
        m_wmSetRootWindowPixmap = false;
    }
}

// Update the number of workspaces.
void BaseScreen::updateWorkspaceCount() {
    m_workspaceCount = m_rootWindow.singlePropertyValue<long>(Atoms::workspaceCountAtom(), 1);
}


//--- SCREEN DAMAGE FUNCTIONS --------------------------------------------------

// Damages the reconfigure rectangle on the screen.
void BaseScreen::damageReconfigureRect() {
    damageScreenArea(m_reconfigureRect);
}

// Damages the given rectangle on the screen.
void BaseScreen::damageScreenArea(XRectangle area) {
    area.height = std::min(area.height + 1, static_cast<int>(rootWindow().height()));
    area.width = std::min(area.width + 1, static_cast<int>(rootWindow().width()));
    m_damagedScreenRects.push_back(area);
}

// Damages the area in the given window.
void BaseScreen::damageWindowArea(BaseCompWindow *window, XRectangle area) {
    area.x += window->x();
    area.y += window->y();
    damageScreenArea(area);
}

// Damages the area taken by the given window.
void BaseScreen::damageWholeWindowArea(BaseCompWindow *window) {
    XRectangle area = { window->x(), window->y(), window->realWidth() + 2, window->realHeight() + 2 };
    area.height = std::min(area.height, static_cast<short unsigned int>(rootWindow().height()));
    area.width = std::min(area.width, static_cast<short unsigned int>(rootWindow().width()));
    m_damagedScreenRects.push_back(area);
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

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

// Puts a window to a new location on the stack.
void BaseScreen::restackWindow(std::list<BaseCompWindow*>::iterator &windowIt, Window above) {
    BaseCompWindow* window = *windowIt;
    m_windows.erase(windowIt);

    std::list<BaseCompWindow*>::iterator it = getFirstManagedAncestorIterator(above);
    if (it != m_windows.end()) {
        ++it;
        m_windows.insert(it, window);
        windowIt = it;
    } else {    // Window is just above root.
        m_windows.push_front(window);
        windowIt = m_windows.begin();
    }
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
        out << "    " << **it << std::endl;
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
