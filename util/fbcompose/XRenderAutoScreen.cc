/** XRenderAutoScreen.cc file for the fluxbox compositor. */

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


#include "XRenderAutoScreen.hh"
#include "XRenderAutoWindow.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
XRenderAutoScreen::XRenderAutoScreen(int screenNumber) :
    BaseScreen(screenNumber) {

    // Fetching all top level windows.
    Window root;
    Window parent;
    Window* children;
    unsigned int childCount;

    XQueryTree(display(), rootWindow().window(), &root, &parent, &children, &childCount);

    for (unsigned int i = 0; i < childCount; i++) {
        createWindow(children[i]);
    }
    if (children) {
        XFree(children);
    }
}

// Destructor.
XRenderAutoScreen::~XRenderAutoScreen() { }


//--- WINDOW MANIPULATION ------------------------------------------------------

// Creates a window object from its XID.
BaseCompWindow *XRenderAutoScreen::createWindowObject(Window window) {
    XRenderAutoWindow *newWindow = new XRenderAutoWindow(window);
    return newWindow;
}

// Cleans up a window object before it is deleted.
void XRenderAutoScreen::cleanupWindowObject(BaseCompWindow &window) { }

// Damages a window object.
void XRenderAutoScreen::damageWindowObject(BaseCompWindow &window) {
    window.setDamaged();
}

// Maps a window object.
void XRenderAutoScreen::mapWindowObject(BaseCompWindow &window) {
    window.setMapped();
}

// Unmaps a window object.
void XRenderAutoScreen::unmapWindowObject(BaseCompWindow &window) {
    window.setUnmapped();
}
