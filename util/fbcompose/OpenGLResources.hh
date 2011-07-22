/** OpenGLResources.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLRESOURCES_HH
#define FBCOMPOSITOR_OPENGLRESOURCES_HH


#include "FbTk/RefCount.hh"

#include <GL/glxew.h>
#include <GL/glx.h>

#include <X11/Xlib.h>


namespace FbCompositor {

    class OpenGLScreen;
    

    //--- OPENGL BUFFER WRAPPER ------------------------------------------------

    /**
     * A wrapper for OpenGL buffers.
     */
    class OpenGLBuffer {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLBuffer(const OpenGLScreen &screen, GLenum targetBuffer);

        /** Destructor. */
        ~OpenGLBuffer();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the handle to the buffer held. */
        GLuint handle() const;

        /** \returns the target of the buffer. */
        GLenum target() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Bind the buffer to its target. */
        void bind();

        /** Loads the given data into the buffer. */
        void bufferData(int elementSize, const GLvoid *data, GLenum usageHint);


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        OpenGLBuffer(const OpenGLBuffer &other);

        /** Assignment operator. */
        OpenGLBuffer &operator=(const OpenGLBuffer &other);


        //--- INTERNALS --------------------------------------------------------

        /** The buffer in question. */
        GLuint m_buffer;

        /** The target buffer. */
        GLenum m_target;


        /** Current connection to the X server. */
        Display *m_display;

        /** Screen that manages this buffer. */
        const OpenGLScreen &m_screen;
    };

    // Bind the buffer to its target.
    inline void OpenGLBuffer::bind() {
        glBindBuffer(m_target, m_buffer);
    }

    // Loads the given data into the buffer.
    inline void OpenGLBuffer::bufferData(int elementSize, const GLvoid *data, GLenum usageHint) {
        bind();
        glBufferData(m_target, elementSize, data, usageHint);
    }

    // Returns the handle to the buffer held.
    inline GLuint OpenGLBuffer::handle() const {
        return m_buffer;
    }

    // Returns the target of the buffer.
    inline GLenum OpenGLBuffer::target() const {
        return m_target;
    }



    //--- OPENGL TEXTURE WRAPPER -----------------------------------------------

    /**
     * OpenGL texture wrapper.
     */
    class OpenGLTexture {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLTexture(const OpenGLScreen &screen, GLenum targetTexture, bool swizzleAlphaToOne);

        /** Destructor. */
        ~OpenGLTexture();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the handle to the texture held. */
        GLuint handle() const;

        /** \returns the target of the buffer. */
        GLenum target() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Bind the texture to its target. */
        void bind();

        /** Sets the texture's contents to the given pixmap. */
        void setPixmap(Pixmap pixmap, int width, int height, bool forceDirect = false);


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        OpenGLTexture(const OpenGLTexture &other);

        /** Assignment operator. */
        OpenGLTexture &operator=(const OpenGLTexture &other);


        //--- INTERNALS --------------------------------------------------------

        /** GLX pixmap of the texture's contents. */
        GLXPixmap m_glxPixmap;

        /** The target texture. */
        GLuint m_target;

        /** The texture in question. */
        GLuint m_texture;


        /** Current connection to the X server. */
        Display *m_display;

        /** Screen that manages this texture. */
        const OpenGLScreen &m_screen;
    };

    // Bind the texture to its target.
    inline void OpenGLTexture::bind() {
        glBindTexture(m_target, m_texture);
    }

    // Returns the handle to the texture held.
    inline GLuint OpenGLTexture::handle() const {
        return m_texture;
    }

    // Returns the target of the buffer.
    inline GLenum OpenGLTexture::target() const {
        return m_target;
    }


    //--- TYPEDEFS -------------------------------------------------------------

    /** OpenGL buffer wrapper smart pointer. */
    typedef FbTk::RefCount<OpenGLBuffer> OpenGLBufferPtr;

    /** OpenGL texture wrapper smart pointer. */
    typedef FbTk::RefCount<OpenGLTexture> OpenGLTexturePtr;
}

#endif  // FBCOMPOSITOR_OPENGLRESOURCES_HH
