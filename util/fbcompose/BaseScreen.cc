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
#include "Logging.hh"

#include "FbTk/App.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>

#include <algorithm>
#include <ostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseScreen::BaseScreen(int screenNumber) :
    m_display(FbTk::App::instance()->display()),
    m_screenNumber(screenNumber),
    m_rootWindow(XRootWindow(m_display, m_screenNumber)) {

    // Set up atoms and properties.
    m_activeWindowAtom = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", False);
    m_rootPixmapAtom = XInternAtom(m_display, "_XROOTPMAP_ID", False);
    m_workspaceAtom = XInternAtom(m_display, "_WIN_WORKSPACE", False);
    m_workspaceCountAtom = XInternAtom(m_display, "_WIN_WORKSPACE_COUNT", False);

    m_activeWindowXID = m_rootWindow.singlePropertyValue<Window>(m_activeWindowAtom, 0);
    m_currentWorkspace = m_rootWindow.singlePropertyValue<long>(m_workspaceAtom, 0);
    m_workspaceCount = m_rootWindow.singlePropertyValue<long>(m_workspaceCountAtom, 1);

    // Set up root window.
    long eventMask = PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask;
    m_rootWindow.setEventMask(eventMask);

    XCompositeRedirectSubwindows(m_display, m_rootWindow.window(), CompositeRedirectManual);
}

// Destructor
BaseScreen::~BaseScreen() { }


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initializes all of the windows on the screen.
void BaseScreen::initWindows() {
    Window root;
    Window parent;
    Window *children;
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
    if (find(m_ignoreList.begin(), m_ignoreList.end(), window) != m_ignoreList.end()) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it == m_windows.end()) {
        BaseCompWindow *newWindow = createWindowObject(window);
        m_windows.push_back(newWindow);
    } else {
        fbLog_warn << "Attempted to create a window twice (" << std::hex << window << ")" << std::endl;
    }
}

// Damages a window on this screen.
void BaseScreen::damageWindow(Window window, XRectangle area) {
    if (find(m_ignoreList.begin(), m_ignoreList.end(), window) != m_ignoreList.end()) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->addDamage(area);
    } else {
        if (window != m_rootWindow.window()) {
            fbLog_warn << "Attempted to damage an untracked window (" << std::hex << window << ")" << std::endl;
        }
    }
}

// Destroys a window on this screen.
void BaseScreen::destroyWindow(Window window) {
    if (find(m_ignoreList.begin(), m_ignoreList.end(), window) != m_ignoreList.end()) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        delete *it;
        m_windows.erase(it);
    } else {
        fbLog_warn << "Attempted to destroy an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Maps a window on this screen.
void BaseScreen::mapWindow(Window window) {
    if (find(m_ignoreList.begin(), m_ignoreList.end(), window) != m_ignoreList.end()) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setMapped();
    } else {
        fbLog_warn << "Attempted to map an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Updates window's configuration.
void BaseScreen::reconfigureWindow(const XConfigureEvent &event) {
    if (find(m_ignoreList.begin(), m_ignoreList.end(), event.window) != m_ignoreList.end()) {
        return;
    }

    if (event.window == m_rootWindow.window()) {
        m_rootWindow.updateGeometry(event);
        setRootWindowChanged();
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
                // TODO: Optimize.
                Window parent = getParentWindow(currentWindow->window());
                while (!isWindowManaged(parent) && (parent != None) && (parent != rootWindow().window())) {
                    parent = getParentWindow(parent);
                }

                it = getWindowIterator(parent);
                if (it != m_windows.end()) {
                    ++it;
                    m_windows.insert(it, currentWindow);
                } else {
                    m_windows.push_back(currentWindow);
                }
            } else {
                ++it;
                m_windows.insert(it, currentWindow);
            }
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
    if (find(m_ignoreList.begin(), m_ignoreList.end(), window) != m_ignoreList.end()) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setClipShapeChanged();
    } else {
        fbLog_warn << "Attempted to update the shape of an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Unmaps a window on this screen.
void BaseScreen::unmapWindow(Window window) {
    if (find(m_ignoreList.begin(), m_ignoreList.end(), window) != m_ignoreList.end()) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setUnmapped();
    } else {
        fbLog_warn << "Attempted to unmap an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Updates the value of some window's property.
void BaseScreen::updateWindowProperty(Window window, Atom property, int state) {
    if (find(m_ignoreList.begin(), m_ignoreList.end(), window) != m_ignoreList.end()) {
        return;
    }
    
    if ((window == m_rootWindow.window()) && (property != None) && (state == PropertyNewValue)) {
        if (property == m_activeWindowAtom) {
            m_activeWindowXID = m_rootWindow.singlePropertyValue<Window>(m_activeWindowAtom, 0);

            while (!isWindowManaged(m_activeWindowXID) && (m_activeWindowXID != None)
                    && (m_activeWindowXID != rootWindow().window())) {
                m_activeWindowXID = getParentWindow(m_activeWindowXID);
            }
        } else if (property == m_rootPixmapAtom) {
            setBackgroundChanged();
        } else if (property == m_workspaceAtom) {
            m_currentWorkspace = m_rootWindow.singlePropertyValue<long>(m_workspaceAtom, 0);
        } else if (property == m_workspaceCountAtom) {
            m_workspaceCount = m_rootWindow.singlePropertyValue<long>(m_workspaceCountAtom, 1);
        }
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->updateProperty(property, state);
    } else {
        if (window != rootWindow().window()) {
            fbLog_warn << "Attempted to set the property of an untracked window (" << std::hex << window << ")" << std::endl;
        }
    }
}


// Adds a window to ignore list, stops tracking it if it is being tracked.
void BaseScreen::addWindowToIgnoreList(Window window) {
    if (find(m_ignoreList.begin(), m_ignoreList.end(), window) == m_ignoreList.end()) {
        destroyWindow(window);
        m_ignoreList.push_back(window);
    }
}

// Checks whether a given window is managed by the current screen.
bool BaseScreen::isWindowManaged(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    return (it != m_windows.end());
}


//--- SCREEN MANIPULATION ----------------------------------------------

// Notifies the screen of the background change.
void BaseScreen::setBackgroundChanged() { }

// Notifies the screen of a root window change.
void BaseScreen::setRootWindowChanged() { }


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
