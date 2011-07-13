/** ResourceWrappers.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_RESOURCEWRAPPERS_HH
#define FBCOMPOSITOR_RESOURCEWRAPPERS_HH


#include "config.h"

#include "FbTk/RefCount.hh"

#ifdef USE_OPENGL_COMPOSITING
    #include <GL/glxew.h>
    #include <GL/glx.h>
#endif  // USE_OPENGL_COMPOSITING

#ifdef USE_XRENDER_COMPOSITING
    #include <X11/extensions/Xrender.h>
#endif  // USE_XRENDER_COMPOSITING

#include <X11/Xlib.h>


namespace FbCompositor {


#ifdef USE_OPENGL_COMPOSITING

    //--- OPENGL BUFFER WRAPPER ------------------------------------------------

    /**
     * OpenGL buffer wrapper.
     *
     * Provides automatic cleanup and other convenience operations for an
     * OpenGL buffer.
     */
    class OpenGLBufferWrapper {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLBufferWrapper() throw();

        /** Destructor. */
        ~OpenGLBufferWrapper() throw();

        //--- ACCESSORS --------------------------------------------------------

        /** \returns the buffer held. */
        GLuint unwrap() const throw();

    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        OpenGLBufferWrapper(const OpenGLBufferWrapper &other) throw();

        /** Assignment operator. */
        OpenGLBufferWrapper &operator=(const OpenGLBufferWrapper &other) throw();

        //--- INTERNALS --------------------------------------------------------

        /** The buffer in question. */
        GLuint m_buffer;
    };

    // Returns the buffer held.
    inline GLuint OpenGLBufferWrapper::unwrap() const throw() {
        return m_buffer;
    }

    /** OpenGL buffer wrapper smart pointer. */
    typedef FbTk::RefCount<OpenGLBufferWrapper> OpenGLBufferWrapperPtr;


    //--- OPENGL TEXTURE HOLDER ------------------------------------------------

    /**
     * OpenGL texture wrapper.
     *
     * Provides automatic cleanup and other convenience operations for an
     * OpenGL texture.
     */
    class OpenGLTextureWrapper {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLTextureWrapper() throw();

        /** Destructor. */
        ~OpenGLTextureWrapper() throw();

        //--- ACCESSORS --------------------------------------------------------

        /** \returns the texture held. */
        GLuint unwrap() const throw();

    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        OpenGLTextureWrapper(const OpenGLTextureWrapper &other) throw();

        /** Assignment operator. */
        OpenGLTextureWrapper &operator=(const OpenGLTextureWrapper &other) throw();

        //--- INTERNALS --------------------------------------------------------

        /** The texture in question. */
        GLuint m_texture;
    };

    // Returns the texture held.
    inline GLuint OpenGLTextureWrapper::unwrap() const throw() {
        return m_texture;
    }

    /** OpenGL texture wrapper smart pointer. */
    typedef FbTk::RefCount<OpenGLTextureWrapper> OpenGLTextureWrapperPtr;

#endif  // USE_OPENGL_COMPOSITING


#ifdef USE_XRENDER_COMPOSITING

    //--- XRENDER PICTURE HOLDER -----------------------------------------------

    /**
     * XRender picture wrapper.
     *
     * Provides automatic cleanup and other convenience operations for a
     * XRender picture.
     */
    class XRenderPictureWrapper {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderPictureWrapper(Display *display, const XRenderPictFormat *pictFormat, const char *pictFilter) throw();

        /** Destructor. */
        ~XRenderPictureWrapper() throw();

        //--- ACCESSORS --------------------------------------------------------

        /** \returns the picture held. */
        Picture unwrap() const throw();

        //--- MUTATORS ---------------------------------------------------------

        /** (Re)associate the picture with the given pixmap. */
        void setPixmap(Pixmap pixmap, XRenderPictureAttributes pa = XRenderPictureAttributes(), long paMask = 0) throw();

    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        XRenderPictureWrapper(const XRenderPictureWrapper &other) throw();

        /** Assignment operator. */
        XRenderPictureWrapper &operator=(const XRenderPictureWrapper &other) throw();

        //--- INTERNALS --------------------------------------------------------

        /** Pointer to the current X connection. */
        Display *m_display;

        /** The picture in question. */
        Picture m_picture;

        /** Picture filter to use. */
        const char *m_pictFilter;

        /** Picture format to use. */
        const XRenderPictFormat *m_pictFormat;
    };

    // Returns the picture held.
    inline Picture XRenderPictureWrapper::unwrap() const throw() {
        return m_picture;
    }

    /** XRender picture wrapper smart pointer. */
    typedef FbTk::RefCount<XRenderPictureWrapper> XRenderPictureWrapperPtr;

#endif  // USE_XRENDER_COMPOSITING

}

#endif  // FBCOMPOSITOR_RESOURCEWRAPPERS_HH
