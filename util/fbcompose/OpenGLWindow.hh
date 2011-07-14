/** OpenGLWindow.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
#define FBCOMPOSITOR_XRENDERAUTOWINDOW_HH


#include "BaseCompWindow.hh"
#include "Exceptions.hh"
#include "OpenGLUtility.hh"
#include "ResourceWrappers.hh"

#include <GL/glxew.h>
#include <GL/glx.h>
#include <GL/gl.h>


namespace FbCompositor {

    class BaseCompWindow;
    class BaseScreen;
    class InitException;
    class OpenGLWindow;


    /**
     * Manages windows in OpenGL rendering mode.
     */
    class OpenGLWindow : public BaseCompWindow {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLWindow(const BaseScreen &screen, Window windowXID, GLXFBConfig fbConfig);

        /** Destructor. */
        virtual ~OpenGLWindow();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns an object, holding the window's contents as an OpenGL texture. */
        OpenGLTextureWrapperPtr contentTexture() const;

        /** \returns an object, holding the window's shape as an OpenGL texture. */
        OpenGLTextureWrapperPtr shapeTexture() const;

        /** \returns an object, holding the window position buffer. */
        OpenGLBufferWrapperPtr windowPosBuffer() const;


        //--- WINDOW UPDATE FUNCTIONS ------------------------------------------

        /** Updates the window's contents. */
        void updateContents();

        /** Updates window's geometry. */
        void updateGeometry(const XConfigureEvent &event);

        /** Updates the window's shape. */
        void updateShape();

        /** Updates the window position vertex array. */
        void updateWindowPosArray();


    private :
        //--- CONVENIENCE ACCESSORS --------------------------------------------

        /** \returns the window's contents as a OpenGL texture. */
        GLuint direct_contentTexture() const;

        /** \returns the window's shape as an OpenGL texture. */
        GLuint direct_shapeTexture() const;

        /** \returns the window position buffer. */
        GLuint direct_windowPosBuffer() const;


        //--- RENDERING-RELATED VARIABLES --------------------------------------

        /** Screen's FBConfig. */
        GLXFBConfig m_fbConfig;


        /** Window's content texture holder. */
        OpenGLTextureWrapperPtr m_contentTexturePtr;

        /** Window's shape texture holder. */
        OpenGLTextureWrapperPtr m_shapeTexturePtr;

        /** Window position array. */
        GLfloat m_windowPosArray[8];

        /** Window position buffer holder. */
        OpenGLBufferWrapperPtr m_windowPosBufferPtr;


        /** Window's shape pixmap. */
        Pixmap m_shapePixmap;


        //--- texture_from_pixmap EXTENSION SPECIFIC ---------------------------

        /** The GLX pixmap of window's contents. */
        GLXPixmap m_glxContents;

        /** The GLX pixmap of window's shape. */
        GLXPixmap m_glxShape;
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns the window's contents as an OpenGL texture.
    inline OpenGLTextureWrapperPtr OpenGLWindow::contentTexture() const {
        return m_contentTexturePtr;
    }

    // Returns an object, holding the window's shape as an OpenGL texture.
    inline OpenGLTextureWrapperPtr OpenGLWindow::shapeTexture() const {
        return m_shapeTexturePtr;
    }

    // Returns the window position buffer.
    inline OpenGLBufferWrapperPtr OpenGLWindow::windowPosBuffer() const {
        return m_windowPosBufferPtr;
    }


    // Returns the window's contents as an OpenGL texture.
    inline GLuint OpenGLWindow::direct_contentTexture() const {
        return m_contentTexturePtr->unwrap();
    }

    // Returns the window's shape as an OpenGL texture.
    inline GLuint OpenGLWindow::direct_shapeTexture() const {
        return m_shapeTexturePtr->unwrap();
    }

    // Returns the window position buffer.
    inline GLuint OpenGLWindow::direct_windowPosBuffer() const {
        return m_windowPosBufferPtr->unwrap();
    }
}

#endif  // FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
