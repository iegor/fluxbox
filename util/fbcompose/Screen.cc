/** Screen.cc file for the fluxbox compositor. */

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


#include "Screen.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseScreen::BaseScreen(Display *display, int screenNumber) :
    m_rootWindow(display, XRootWindow(display, screenNumber)) {

    m_display = display;
    m_screenNumber = screenNumber;

    // Registering events.
    XSelectInput(m_display, m_rootWindow.window(),
                 ExposureMask | PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask);

    // Fetching all top level windows.
    Window root;
    Window parent;
    Window* children;
    unsigned int childCount;

    XQueryTree(m_display, m_rootWindow.window(), &root, &parent, &children, &childCount);

    for(unsigned int i = 0; i < childCount; i++) {
        m_windows.push_back(BaseCompWindow(m_display, children[i]));
    }
    if(children) {
        XFree(children);
    }
}

// Destructor
BaseScreen::~BaseScreen() { }


//--- WINDOW MANIPULATION ----------------------------------------------

// Creates a new window and inserts it into the list of windows.
void BaseScreen::createWindow(const XCreateWindowEvent &event) {
    m_windows.push_back(BaseCompWindow(m_display, event.window));
}

// Destroys a window on this screen.
void BaseScreen::destroyWindow(const XDestroyWindowEvent &event) {
    m_windows.erase(getWindowIterator(event.window));
}

// Maps a window on this screen.
void BaseScreen::mapWindow(const XMapEvent &event) {
    getWindowIterator(event.window)->setMapped();
}

// Unmaps a window on this screen.
void BaseScreen::unmapWindow(const XUnmapEvent &event) {
    getWindowIterator(event.window)->setUnmapped();
}


//--- INTERNAL CONVENIENCE FUNCTIONS -----------------------------------

/** Returns an iterator of m_windows that points to the given window. */
std::list<BaseCompWindow>::iterator BaseScreen::getWindowIterator(Window window) {
    std::list<BaseCompWindow>::iterator it = m_windows.begin();
    while(it != m_windows.end()) {
        if(window == (*it).window()) {
            break;
        }
        it++;
    }
    return it;
}
