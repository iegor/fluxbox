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

        /** \returns the element buffer. */
        GLuint elementBuffer() const throw();

        /** \returns the vertex buffer. */
        GLuint vertexBuffer() const throw();


        //--- WINDOW UPDATE FUNCTIONS ------------------------------------------

        /** Update window's vertex and element arrays. */
        void updateArrays();

    private :
        //--- RENDERING-RELATED VARIABLES --------------------------------------

        /** Window's vertex array. */
        GLfloat m_vertexArray[8];

        /** Window's vertex buffer. */
        GLuint m_vertexBuffer;

        /** Window's element array. */
        GLushort m_elementArray[4];

        /** Window's element buffer. */
        GLuint m_elementBuffer;


        //--- OTHER VARIABLES --------------------------------------------------

        /** Width of the root window of the window's screen. */
        int m_rootWidth;

        /** Height of the root window of the window's screen. */
        int m_rootHeight;
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns the element buffer.
    inline GLuint OpenGLWindow::elementBuffer() const throw() {
        return m_elementBuffer;
    }

    // Returns the vertex buffer.
    inline GLuint OpenGLWindow::vertexBuffer() const throw() {
        return m_vertexBuffer;
    }
}

#endif  // FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
