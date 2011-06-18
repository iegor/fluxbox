/** XRenderScreen.cc file for the fluxbox compositor. */

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

#include "XRenderScreen.hh"
#include "XRenderWindow.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
XRenderScreen::XRenderScreen(int screenNumber) :
    BaseScreen(screenNumber) {
}

// Destructor.
XRenderScreen::~XRenderScreen() { }


//--- SCREEN MANIPULATION ------------------------------------------------------

// Notifies the screen of a background change.
void XRenderScreen::setBackgroundChanged() { }

// Notifies the screen of a root window change.
void XRenderScreen::setRootWindowChanged() { }


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void XRenderScreen::renderScreen() { }


//--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS --------------------------------

// Creates a window object from its XID.
BaseCompWindow *XRenderScreen::createWindowObject(Window window) {
    XRenderWindow *newWindow = new XRenderWindow(window);
    return newWindow;
}
