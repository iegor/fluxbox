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

#include <GL/glxew.h>
#include <GL/glx.h>
#include <GL/gl.h>


namespace FbCompositor {

    class BaseCompWindow;
    class OpenGLWindow;
    class RuntimeException;


    /**
     * Manages windows in OpenGL rendering mode.
     */
    class OpenGLWindow : public BaseCompWindow {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLWindow(Window windowXID, GLXFBConfig fbConfig) throw();

        /** Destructor. */
        virtual ~OpenGLWindow() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the window's contents as a OpenGL texture. */
        GLuint contentTexture() const throw();

        /** \returns the window position buffer. */
        GLuint windowPosBuffer() const throw();


        //--- WINDOW UPDATE FUNCTIONS ------------------------------------------

        /** Updates the window's contents. */
        void updateContents() throw(RuntimeException);

        /** Updates window's geometry. */
        void updateGeometry(const XConfigureEvent &event) throw();


    private :
        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Updates the window position vertex array. */
        void updateWindowPosArray() throw();


        //--- RENDERING-RELATED VARIABLES --------------------------------------

        /** Window's content texture. */
        GLuint m_contentTexture;

        /** Screen's FBConfig. */
        GLXFBConfig m_fbConfig;

        /** Window position array. */
        GLfloat m_windowPosArray[8];

        /** Window position buffer. */
        GLuint m_windowPosBuffer;


        //--- texture_from_pixmap EXTENSION SPECIFIC ---------------------------

#ifdef GLXEW_EXT_texture_from_pixmap
        /** Attributes of the contents' GLX pixmap. */
        static const int TEX_PIXMAP_ATTRIBUTES[];

        /** The GLX pixmap of window's contents. */
        GLXPixmap m_glxContents;
#endif  // GLXEW_EXT_texture_from_pixmap
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns the window's contents as a OpenGL texture.
    inline GLuint OpenGLWindow::contentTexture() const throw() {
        return m_contentTexture;
    }

    // Returns the window position buffer.
    inline GLuint OpenGLWindow::windowPosBuffer() const throw() {
        return m_windowPosBuffer;
    }
}

#endif  // FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
