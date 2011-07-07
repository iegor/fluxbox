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

#include "config.h"

#ifdef USE_OPENGL_COMPOSITING


#include "BaseCompWindow.hh"
#include "Exceptions.hh"
#include "OpenGLUtility.hh"

#include <GL/glxew.h>
#include <GL/glx.h>
#include <GL/gl.h>


namespace FbCompositor {

    class BaseCompWindow;
    class BaseScreen;
    class InitException;
    class OpenGLWindow;
    class RuntimeException;


    /**
     * Manages windows in OpenGL rendering mode.
     */
    class OpenGLWindow : public BaseCompWindow {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLWindow(const BaseScreen &screen, Window windowXID, GLXFBConfig fbConfig) throw(InitException);

        /** Destructor. */
        virtual ~OpenGLWindow() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns an object, holding the window's contents as an OpenGL texture. */
        OpenGLTexturePtr contentTexture() const throw();

        /** \returns an object, holding the window's shape as an OpenGL texture. */
        OpenGLTexturePtr shapeTexture() const throw();

        /** \returns an object, holding the window position buffer. */
        OpenGLBufferPtr windowPosBuffer() const throw();


        //--- WINDOW UPDATE FUNCTIONS ------------------------------------------

        /** Updates the window's contents. */
        void updateContents() throw(RuntimeException);

        /** Updates window's geometry. */
        void updateGeometry(const XConfigureEvent &event) throw();

        /** Updates the window's shape. */
        void updateShape() throw(RuntimeException);

        /** Updates the window position vertex array. */
        void updateWindowPosArray() throw();


    private :
        //--- CONVENIENCE ACCESSORS --------------------------------------------

        /** \returns the window's contents as a OpenGL texture. */
        GLuint direct_contentTexture() const throw();

        /** \returns the window's shape as an OpenGL texture. */
        GLuint direct_shapeTexture() const throw();

        /** \returns the window position buffer. */
        GLuint direct_windowPosBuffer() const throw();


        //--- RENDERING-RELATED VARIABLES --------------------------------------

        /** Screen's FBConfig. */
        GLXFBConfig m_fbConfig;


        /** Window's content texture holder. */
        OpenGLTexturePtr m_contentTexturePtr;

        /** Window's shape texture holder. */
        OpenGLTexturePtr m_shapeTexturePtr;

        /** Window position array. */
        GLfloat m_windowPosArray[8];

        /** Window position buffer holder. */
        OpenGLBufferPtr m_windowPosBufferPtr;


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
    inline OpenGLTexturePtr OpenGLWindow::contentTexture() const throw() {
        return m_contentTexturePtr;
    }

    // Returns an object, holding the window's shape as an OpenGL texture.
    inline OpenGLTexturePtr OpenGLWindow::shapeTexture() const throw() {
        return m_shapeTexturePtr;
    }

    // Returns the window position buffer.
    inline OpenGLBufferPtr OpenGLWindow::windowPosBuffer() const throw() {
        return m_windowPosBufferPtr;
    }


    // Returns the window's contents as an OpenGL texture.
    inline GLuint OpenGLWindow::direct_contentTexture() const throw() {
        return m_contentTexturePtr->texture();
    }

    // Returns the window's shape as an OpenGL texture.
    inline GLuint OpenGLWindow::direct_shapeTexture() const throw() {
        return m_shapeTexturePtr->texture();
    }

    // Returns the window position buffer.
    inline GLuint OpenGLWindow::direct_windowPosBuffer() const throw() {
        return m_windowPosBufferPtr->buffer();
    }
}

#endif  // USE_OPENGL_COMPOSITING

#endif  // FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
