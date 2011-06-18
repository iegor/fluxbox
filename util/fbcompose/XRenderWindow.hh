/** XRenderWindow.hh file for the fluxbox compositor. */

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

#ifndef FBCOMPOSITOR_XRENDERWINDOW_HH
#define FBCOMPOSITOR_XRENDERWINDOW_HH

#include "BaseCompWindow.hh"
#include "Exceptions.hh"

#include <X11/extensions/Xrender.h>


namespace FbCompositor {

    class BaseCompWindow;
    class InitException;
    class XRenderWindow;


    /**
     * Manages windows in XRender rendering mode.
     */
    class XRenderWindow : public BaseCompWindow {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderWindow(Window windowXID) throw(InitException);

        /** Destructor. */
        ~XRenderWindow() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the window's contents as an XRender picture. */
        Picture contentPicture() const throw();


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Update the window's contents. */
        void updateContents();


    protected:
        //--- PROTECTED WINDOW MANIPULATION ------------------------------------

        /** Update the window's clip shape. */
        void updateShape();


    private:
        //--- RENDERING RELATED ------------------------------------------------

        /** The window's picture format. */
        XRenderPictFormat *m_pictFormat;

        /** The window's picture. */
        Picture m_picture;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the window's contents as an XRender picture.
    inline Picture XRenderWindow::contentPicture() const throw() {
        return m_picture;
    }
}

#endif  // FBCOMPOSITOR_XRENDERWINDOW_HH
