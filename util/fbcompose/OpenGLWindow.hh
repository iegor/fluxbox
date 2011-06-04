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

#include <GL/glew.h>
#include <GL/gl.h>


namespace FbCompositor {

    /**
     * Manages windows in OpenGL rendering mode.
     */
    class OpenGLWindow : public BaseCompWindow {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLWindow(Window windowXID);

        /** Destructor. */
        virtual ~OpenGLWindow();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the window's contents as a OpenGL texture. */
        GLuint contentTexture() const throw();

        /** \returns the element buffer. */
        GLuint elementBuffer() const throw();

        /** \returns the texture position buffer. */
        GLuint texturePosBuffer() const throw();

        /** \returns the window position buffer. */
        GLuint windowPosBuffer() const throw();


        //--- WINDOW UPDATE FUNCTIONS ------------------------------------------

        /** Update the appropriate window's arrays. */
        void updateArrays();

        /** Updates the window's contents. */
        void updateContents();


    private :
        //--- RENDERING-RELATED VARIABLES --------------------------------------

        /** Window's content texture. */
        GLuint m_contentTexture;

        /** Window's element array. */
        GLushort m_elementArray[4];

        /** Window's element buffer. */
        GLuint m_elementBuffer;

        /** Window texture position array. */
        GLfloat m_texturePosArray[8];

        /** Window texture position buffer. */
        GLuint m_texturePosBuffer;

        /** Window position array. */
        GLfloat m_windowPosArray[8];

        /** Window position buffer. */
        GLuint m_windowPosBuffer;


        //--- OTHER VARIABLES --------------------------------------------------

        /** Width of the root window of the window's screen. */
        int m_rootWidth;

        /** Height of the root window of the window's screen. */
        int m_rootHeight;
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns the window's contents as a OpenGL texture.
    inline GLuint OpenGLWindow::contentTexture() const throw() {
        return m_contentTexture;
    }

    // Returns the element buffer.
    inline GLuint OpenGLWindow::elementBuffer() const throw() {
        return m_elementBuffer;
    }

    // Returns the texture position buffer.
    inline GLuint OpenGLWindow::texturePosBuffer() const throw() {
        return m_texturePosBuffer;
    }

    // Returns the window position buffer.
    inline GLuint OpenGLWindow::windowPosBuffer() const throw() {
        return m_windowPosBuffer;
    }
}

#endif  // FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
