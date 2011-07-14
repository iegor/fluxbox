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
#include "ResourceWrappers.hh"

#include <X11/extensions/Xrender.h>


namespace FbCompositor {

    class BaseCompWindow;
    class BaseScreen;
    class InitException;
    class XRenderWindow;


    /**
     * Manages windows in XRender rendering mode.
     */
    class XRenderWindow : public BaseCompWindow {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderWindow(const BaseScreen &screen, Window windowXID, const char *pictFilter);

        /** Destructor. */
        ~XRenderWindow();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns an object, holding the window's contents as an XRender picture. */
        XRenderPictureWrapperPtr contentPicture() const;
        
        /** \returns an object, the window's mask picture. */
        XRenderPictureWrapperPtr maskPicture() const;


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Update the window's contents. */
        void updateContents();

        /** Update window's property. */
        void updateProperty(Atom property, int state);


    protected:
        //--- PROTECTED WINDOW MANIPULATION ------------------------------------

        /** Update the window's clip shape. */
        void updateShape();


    private:
        //--- CONVENIENCE ACCESSORS --------------------------------------------

        /** \returns the window's contents as an XRender picture. */
        Picture direct_contentPicture() const;

        /** \returns the window's mask picture. */
        Picture direct_maskPicture() const;


        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Update the window's mask picture. */
        void updateMaskPicture();


        //--- RENDERING RELATED ------------------------------------------------

        /** The window's mask pixmap. */
        Pixmap m_maskPixmap;


        /** The window's content picture. */
        XRenderPictureWrapperPtr m_contentPicture;

        /** The window's mask picture. */
        XRenderPictureWrapperPtr m_maskPicture;


        /** The picture filter. */
        const char *m_pictFilter;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the window's contents as an XRender picture.
    inline XRenderPictureWrapperPtr XRenderWindow::contentPicture() const {
        return m_contentPicture;
    }

    // Returns the window's mask picture.
    inline XRenderPictureWrapperPtr XRenderWindow::maskPicture() const {
        return m_maskPicture;
    }


    // Returns the window's contents as an XRender picture.
    inline Picture XRenderWindow::direct_contentPicture() const {
        return m_contentPicture->unwrap();
    }

    // Returns the window's mask picture.
    inline Picture XRenderWindow::direct_maskPicture() const {
        return m_maskPicture->unwrap();
    }
}

#endif  // FBCOMPOSITOR_XRENDERWINDOW_HH
