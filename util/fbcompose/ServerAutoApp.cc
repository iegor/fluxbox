/** ServerAutoApp.cc file for the fluxbox compositor. */

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


#include "ServerAutoApp.hh"

#include "Atoms.hh"
#include "CompositorConfig.hh"

#include <X11/extensions/Xcomposite.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <ctime>
#include <sstream>

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

// How many microseconds to sleep before restarting the event loop.
const int ServerAutoApp::SLEEP_TIME = 10000;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
ServerAutoApp::ServerAutoApp(const CompositorConfig &config) throw(InitException) :
    App(config.displayName().c_str()) {

    if (config.renderingMode() != RM_ServerAuto) {
        throw InitException("ServerAutoApp provides only the \"serverauto\" renderer.");
    }
    initComposite();
    initScreens();

    XFlush(display());
}

// Destructor.
ServerAutoApp::~ServerAutoApp() throw() { }


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Initialize Composite extension.
void ServerAutoApp::initComposite() throw(InitException) {
    int eventBase;
    int errorBase;
    int majorVer;
    int minorVer;

    if (!XCompositeQueryExtension(display(), &eventBase, &errorBase)) {
        throw InitException("Composite extension not found.");
    }
    if (!XCompositeQueryVersion(display(), &majorVer, &minorVer)) {
        throw InitException("Could not query the version of the Composite extension.");
    }
    if ((majorVer < 0) || ((majorVer == 0) && (minorVer < 1))) {
        std::stringstream ss;
        ss << "Unsupported Composite extension version (required >=0.1, got "
           << majorVer << "." << minorVer << ").";
        throw InitException(ss.str());
    }
}

// Prepare screens.
void ServerAutoApp::initScreens() throw(InitException) {
    int screenCount = XScreenCount(display());
    for (int i = 0; i < screenCount; i++) {
        XCompositeRedirectSubwindows(display(), XRootWindow(display(), i), CompositeRedirectAutomatic);

        Atom cmAtom = Atoms::compositingSelectionAtom(i);
        Window curOwner = XGetSelectionOwner(display(), cmAtom);
        if (curOwner != None) {
            // TODO: More detailed message - what is the other program?
            throw InitException("Another compositing manager is running.");
        }

        // TODO: Better way of obtaining program's name in SetWMProperties.
        curOwner = XCreateSimpleWindow(display(), XRootWindow(display(), i), -10, -10, 1, 1, 0, None, None);
        XmbSetWMProperties(display(), curOwner, "fbcompose", "fbcompose", NULL, 0, NULL, NULL, NULL);
        XSetSelectionOwner(display(), cmAtom, curOwner, CurrentTime);
    }
}


//--- EVENT LOOP ---------------------------------------------------------------

// Enters the event loop.
void ServerAutoApp::eventLoop() throw() {
    timespec sleepTimespec = { 0, SLEEP_TIME * 1000 };
    while (!done()) {
        nanosleep(&sleepTimespec, NULL);
    }
}
