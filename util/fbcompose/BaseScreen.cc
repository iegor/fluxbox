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

#include "FbTk/App.hh"

#include <X11/extensions/Xcomposite.h>

#include <ostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseScreen::BaseScreen(int screenNumber) :
    m_display(FbTk::App::instance()->display()),
    m_screenNumber(screenNumber),
    m_rootWindow(XRootWindow(m_display, m_screenNumber)) {

    m_activeWindowAtom = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", false);
    m_workspaceAtom = XInternAtom(m_display, "_WIN_WORKSPACE", false);
    m_workspaceCountAtom = XInternAtom(m_display, "_WIN_WORKSPACE_COUNT", false);

    if ((m_activeWindowAtom == None)
            || (m_workspaceAtom == None)
            || (m_workspaceCountAtom == None)) {
        // TODO: Do something.
    }

    m_activeWindowXID = m_rootWindow.windowProperty(m_activeWindowAtom)[0];
    m_currentWorkspace = m_rootWindow.cardinalProperty(m_workspaceAtom)[0];
    m_workspaceCount = m_rootWindow.cardinalProperty(m_workspaceCountAtom)[0];

    long eventMask = ExposureMask | PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask;
    m_rootWindow.setEventMask(eventMask);

    XCompositeRedirectSubwindows(m_display, m_rootWindow.window(), CompositeRedirectManual);
}

// Destructor
BaseScreen::~BaseScreen() { }


//--- WINDOW MANIPULATION ----------------------------------------------

// Creates a new window and inserts it into the list of windows.
void BaseScreen::createWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it == m_windows.end()) {
        BaseCompWindow *newWindow = createWindowObject(window);
        m_windows.push_back(newWindow);
    } else {
        // TODO: Throw something.
    }
}

// Damages a window on this screen.
void BaseScreen::damageWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        damageWindowObject(**it);
    } else {
        // TODO: Throw something.
    }
}

// Destroys a window on this screen.
void BaseScreen::destroyWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        cleanupWindowObject(**it);
        delete *it;
        m_windows.erase(it);
    } else {
        // TODO: Throw something.
    }
}

// Maps a window on this screen.
void BaseScreen::mapWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        mapWindowObject(**it);
    } else {
        // TODO: Throw something.
    }
}

// Updates window's configuration.
void BaseScreen::reconfigureWindow(const XConfigureEvent &event) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(event.window);
    if (it != m_windows.end()) {
        reconfigureWindowObject(**it);

        BaseCompWindow *currentWindow = *it;
        m_windows.erase(it);

        if (event.above == None) {
            m_windows.push_front(currentWindow);
        } else {
            it = getWindowIterator(event.above);
            m_windows.insert(it, currentWindow);
        }
    } else {
        // TODO: Throw something.
    }
}

// Unmaps a window on this screen.
void BaseScreen::unmapWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        unmapWindowObject(**it);
    } else {
        // TODO: Throw something.
    }
}

// Updates the value of some window's property.
void BaseScreen::updateWindowProperty(Window window, Atom property, int state) {
    // TODO: Should we check for existence of values? It is a rather sensible
    // assumption that the root window will have all the needed properties.

    if ((window == m_rootWindow.window()) && (state == PropertyNewValue)) {
        if (property == m_activeWindowAtom) {
            m_activeWindowXID = m_rootWindow.windowProperty(m_activeWindowAtom)[0];
        } else if (property == m_workspaceAtom) {
            m_currentWorkspace = m_rootWindow.cardinalProperty(m_workspaceAtom)[0];
        } else if (property == m_workspaceCountAtom) {
            m_workspaceCount = m_rootWindow.cardinalProperty(m_workspaceCountAtom)[0];
        }
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        updateWindowObjectProperty(**it, property, state);
    } else {
        // TODO: Throw something.
    }
}


//--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS --------------------------------

// Creates a new window object from its XID.
BaseCompWindow *BaseScreen::createWindowObject(Window window) {
    BaseCompWindow *newWindow = new BaseCompWindow(window);
    return newWindow;
}

// Cleans up a window object before it is deleted.
void BaseScreen::cleanupWindowObject(BaseCompWindow &window) { }

// Damages a window object.
void BaseScreen::damageWindowObject(BaseCompWindow &window) {
    window.setDamaged();
}

// Maps a window object.
void BaseScreen::mapWindowObject(BaseCompWindow &window) {
    window.setMapped();
}

// Updates configuration of a window object.
// TODO: Improve reconfiguration - take values from XConfigureEvent object,
// rather than make a separate X call to get window's parameters.
void BaseScreen::reconfigureWindowObject(BaseCompWindow &window) {
    window.updateGeometry();
}

// Unmaps a window object.
void BaseScreen::unmapWindowObject(BaseCompWindow &window) {
    window.setUnmapped();
}

// Updates the value of some window's property.
void BaseScreen::updateWindowObjectProperty(BaseCompWindow &window, Atom property, int state) { }


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Returns an iterator of m_windows that points to the given window.
std::list<BaseCompWindow*>::iterator BaseScreen::getWindowIterator(Window window) {
    std::list<BaseCompWindow*>::iterator it = m_windows.begin();
    while (it != m_windows.end()) {
        if (window == (*it)->window()) {
            break;
        }
        it++;
    }
    return it;
}


//--- FRIEND OPERATORS -------------------------------------------------

// << output stream operator for the BaseScreen class.
std::ostream &FbCompositor::operator<<(std::ostream& out, const BaseScreen& s) {
    out << "SCREEN NUMBER " << s.m_screenNumber << ":" << std::endl
        << "  Properties" << std::endl
        << "    Active window XID: " << s.m_activeWindowXID << std::endl
        << "    Number of workspaces: " << s.m_workspaceCount << std::endl
        << "    Current workspace: " << s.m_currentWorkspace << std::endl
        << "  Windows" << std::endl;

    std::list<BaseCompWindow*>::const_iterator it = s.m_windows.begin();
    while(it != s.m_windows.end()) {
        out << "    " << **it << std::endl;
        it++;
    }

    return out;
}
