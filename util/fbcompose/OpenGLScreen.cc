/** OpenGLScreen.cc file for the fluxbox compositor. */

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


#include "OpenGLScreen.hh"
#include "OpenGLWindow.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLScreen::OpenGLScreen(int screenNumber) :
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
OpenGLScreen::~OpenGLScreen() { }


//--- WINDOW MANIPULATION ------------------------------------------------------

// Creates a window object from its XID.
BaseCompWindow *OpenGLScreen::createWindowObject(Window window) {
    OpenGLWindow *newWindow = new OpenGLWindow(window);
    return newWindow;
}

// Cleans up a window object before it is deleted.
void OpenGLScreen::cleanupWindowObject(BaseCompWindow &window) { }

// Damages a window object.
void OpenGLScreen::damageWindowObject(BaseCompWindow &window) {
    window.setDamaged();
}

// Maps a window object.
void OpenGLScreen::mapWindowObject(BaseCompWindow &window) {
    window.setMapped();
}

// Updates window's configuration.
void OpenGLScreen::reconfigureWindowObject(BaseCompWindow &window) {
    window.updateGeometry();
}

// Unmaps a window object.
void OpenGLScreen::unmapWindowObject(BaseCompWindow &window) {
    window.setUnmapped();
}

// Updates the value of some window's property.
void OpenGLScreen::updateWindowObjectProperty(BaseCompWindow &window, Atom property, int state) { }
