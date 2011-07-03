/** OpenGLUtility.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLUTILITY_HH
#define FBCOMPOSITOR_OPENGLUTILITY_HH

#include "config.h"

#ifdef USE_OPENGL_COMPOSITING


#include "FbTk/RefCount.hh"

#include <GL/glew.h>
#include <GL/gl.h>


namespace FbCompositor {

    //--- OPENGL BUFFER HOLDER -------------------------------------------------

    /**
     * OpenGL buffer holder.
     *
     * Use this class together with FbTk::RefCount or any other pointer wrapper
     * when you need to provide access to an OpenGL buffer from multiple
     * locations in the code.
     */
    class OpenGLBufferHolder {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLBufferHolder() throw();

        /** Destructor. */
        ~OpenGLBufferHolder() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the buffer held. */
        GLuint buffer() const throw();

    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        OpenGLBufferHolder(const OpenGLBufferHolder &other) throw();

        /** Assignment operator. */
        OpenGLBufferHolder &operator=(const OpenGLBufferHolder &other) throw();


        //--- INTERNALS --------------------------------------------------------

        /** The buffer in question. */
        GLuint m_buffer;
    };

    // Returns the buffer held.
    inline GLuint OpenGLBufferHolder::buffer() const throw() {
        return m_buffer;
    }


    /** OpenGL buffer holder pointer. Use this if you want to provide an OpenGL
     *  buffer to other places in the code. */
    typedef FbTk::RefCount<OpenGLBufferHolder> OpenGLBufferPtr;


    //--- OPENGL TEXTURE HOLDER ------------------------------------------------

    /**
     * OpenGL texture holder.
     *
     * Use this class together with FbTk::RefCount or any other pointer wrapper
     * when you need to provide access to an OpenGL texture from multiple
     * locations in the code.
     */
    class OpenGLTextureHolder {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLTextureHolder() throw();

        /** Destructor. */
        ~OpenGLTextureHolder() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the texture held. */
        GLuint texture() const throw();

    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        OpenGLTextureHolder(const OpenGLTextureHolder &other) throw();

        /** Assignment operator. */
        OpenGLTextureHolder &operator=(const OpenGLTextureHolder &other) throw();


        //--- INTERNALS --------------------------------------------------------

        /** The texture in question. */
        GLuint m_texture;
    };

    // Returns the texture held.
    inline GLuint OpenGLTextureHolder::texture() const throw() {
        return m_texture;
    }


    /** OpenGL texture holder pointer. Use this if you want to provide an OpenGL
     *  texture to other places in the code. */
    typedef FbTk::RefCount<OpenGLTextureHolder> OpenGLTexturePtr;


    //--- FUNCTIONS ------------------------------------------------------------

    /** Converts screen coordinates to OpenGL coordinates. */
    void toOpenGLCoordinates(int screenWidth, int screenHeight, int x, int y, int width, int height,
                             GLfloat *xLow_gl, GLfloat *xHigh_gl, GLfloat *yLow_gl, GLfloat *yHigh_gl);

}

#endif  // USE_OPENGL_COMPOSITING

#endif  // FBCOMPOSITOR_OPENGLUTILITY_HH
