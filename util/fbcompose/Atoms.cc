/** Atoms.cc file for the fluxbox compositor. */

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


#include "Atoms.hh"

#include "FbTk/App.hh"

#include <X11/Xatom.h>

#include <sstream>

using namespace FbCompositor;


//--- MAIN METHODS -------------------------------------------------------------

// Returns the _NET_ACTIVE_WINDOW atom.
Atom Atoms::activeWindowAtom() {
    static Atom atom = XInternAtom(FbTk::App::instance()->display(), "_NET_ACTIVE_WINDOW", False);
    return atom;
}

// Returns the _NET_WM_CM_Sxx atoms.
Atom Atoms::compositingSelectionAtom(int screenNumber) {
    std::stringstream ss;
    ss << "_NET_WM_CM_S" << screenNumber;
    return XInternAtom(FbTk::App::instance()->display(), ss.str().c_str(), False);
}

// Returns the _NET_WM_WINDOW_OPACITY atom.
Atom Atoms::opacityAtom() {
    static Atom atom = XInternAtom(FbTk::App::instance()->display(), "_NET_WM_WINDOW_OPACITY", False);
    return atom;
}

// Returns the _FLUXBOX_RECONFIGURE_RECT atom.
Atom Atoms::reconfigureRectAtom() {
    static Atom atom = XInternAtom(FbTk::App::instance()->display(), "_FLUXBOX_RECONFIGURE_RECT", False);
    return atom;
}

// Returns the _XROOTPMAP_ID atom.
Atom Atoms::rootPixmapAtom() {
    static Atom atom = XInternAtom(FbTk::App::instance()->display(), "_XROOTPMAP_ID", False);
    return atom;
}

// Returns the _WIN_WORKSPACE atom.
Atom Atoms::workspaceAtom() {
    static Atom atom = XInternAtom(FbTk::App::instance()->display(), "_WIN_WORKSPACE", False);
    return atom;
}

// Returns the _WIN_WORKSPACE_COUNT atom.
Atom Atoms::workspaceCountAtom() {
    static Atom atom = XInternAtom(FbTk::App::instance()->display(), "_WIN_WORKSPACE_COUNT", False);
    return atom;
}
