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

#include "config.h"

#ifdef USE_XRENDER_COMPOSITING


#include "BaseCompWindow.hh"
#include "Exceptions.hh"
#include "XRenderUtility.hh"

#include <X11/extensions/Xrender.h>


namespace FbCompositor {

    class BaseCompWindow;
    class BaseScreen;
    class InitException;
    class RuntimeException;
    class XRenderWindow;


    /**
     * Manages windows in XRender rendering mode.
     */
    class XRenderWindow : public BaseCompWindow {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderWindow(const BaseScreen &screen, Window windowXID, const char *pictFilter) throw(InitException);

        /** Destructor. */
        ~XRenderWindow() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns an object, holding the window's contents as an XRender picture. */
        XRenderPicturePtr contentPicture() const throw();
        
        /** \returns an object, the window's mask picture. */
        XRenderPicturePtr maskPicture() const throw();


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Update the window's contents. */
        void updateContents() throw(RuntimeException);

        /** Update window's property. */
        void updateProperty(Atom property, int state) throw(RuntimeException);


    protected:
        //--- PROTECTED WINDOW MANIPULATION ------------------------------------

        /** Update the window's clip shape. */
        void updateShape() throw(RuntimeException);


    private:
        //--- CONVENIENCE ACCESSORS --------------------------------------------

        /** \returns the window's contents as an XRender picture. */
        Picture direct_contentPicture() const throw();

        /** \returns the window's mask picture. */
        Picture direct_maskPicture() const throw();


        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Update the window's mask picture. */
        void updateMaskPicture() throw();


        //--- RENDERING RELATED ------------------------------------------------

        /** The window's mask pixmap. */
        Pixmap m_maskPixmap;


        /** The window's content picture. */
        XRenderPicturePtr m_contentPicture;

        /** The window's mask picture. */
        XRenderPicturePtr m_maskPicture;


        /** The picture filter. */
        const char *m_pictFilter;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the window's contents as an XRender picture.
    inline XRenderPicturePtr XRenderWindow::contentPicture() const throw() {
        return m_contentPicture;
    }

    // Returns the window's mask picture.
    inline XRenderPicturePtr XRenderWindow::maskPicture() const throw() {
        return m_maskPicture;
    }


    // Returns the window's contents as an XRender picture.
    inline Picture XRenderWindow::direct_contentPicture() const throw() {
        return m_contentPicture->picture();
    }

    // Returns the window's mask picture.
    inline Picture XRenderWindow::direct_maskPicture() const throw() {
        return m_maskPicture->picture();
    }
}

#endif  // USE_XRENDER_COMPOSITING

#endif  // FBCOMPOSITOR_XRENDERWINDOW_HH
