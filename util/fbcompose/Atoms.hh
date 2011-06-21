/** Atoms.hh file for the fluxbox compositor. */

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

#ifndef FBCOMPOSITOR_ATOMS_HH
#define FBCOMPOSITOR_ATOMS_HH

#include <X11/Xlib.h>
#include <X11/Xatom.h>


namespace FbCompositor {

    /**
     * The main X atom manager.
     */
    class Atoms {
    public :
        /** \returns the _NET_ACTIVE_WINDOW atom. */
        static Atom activeWindowAtom() throw();

        /** \returns the _NET_WM_WINDOW_OPACITY atom. */
        static Atom opacityAtom() throw();

        /** \returns the _FLUXBOX_RECONFIGURE_RECT atom. */
        static Atom reconfigureRectAtom() throw();

        /** \returns the _XROOTPMAP_ID atom. */
        static Atom rootPixmapAtom() throw();

        /** \returns the _WIN_WORKSPACE atom. */
        static Atom workspaceAtom() throw();

        /** \returns the _WIN_WORKSPACE_COUNT atom. */
        static Atom workspaceCountAtom() throw();
    };
}


#endif  // FBCOMPOSITOR_ATOMS_HH
