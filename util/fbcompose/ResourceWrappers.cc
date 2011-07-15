/** ResourceWrappers.cc file for the fluxbox compositor. */

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


#include "ResourceWrappers.hh"

using namespace FbCompositor;


#ifdef USE_OPENGL_COMPOSITING

//--- OPENGL BUFFER WRAPPER ----------------------------------------------------

// Constructor.
OpenGLBufferWrapper::OpenGLBufferWrapper() {
    glGenBuffers(1, &m_buffer);
}

// Destructor.
OpenGLBufferWrapper::~OpenGLBufferWrapper() {
    glDeleteBuffers(1, &m_buffer);
}


//--- OPENGL TEXTURE WRAPPER ---------------------------------------------------

// Constructor.
OpenGLTextureWrapper::OpenGLTextureWrapper() {
    glGenTextures(1, &m_texture);
}

// Destructor.
OpenGLTextureWrapper::~OpenGLTextureWrapper() {
    glDeleteTextures(1, &m_texture);
}

#endif  // USE_OPENGL_COMPOSITING


#ifdef USE_XRENDER_COMPOSITING

//--- XRENDER PICTURE WRAPPER --------------------------------------------------

// Constructor.
XRenderPictureWrapper::XRenderPictureWrapper(Display *display, XRenderPictFormat *pictFormat,
                                             const char *pictFilter) :
    m_display(display),
    m_picture(None),
    m_pictFilter(pictFilter),
    m_pictFormat(pictFormat) {
}

// Destructor.
XRenderPictureWrapper::~XRenderPictureWrapper() {
    if (m_picture) {
        XRenderFreePicture(m_display, m_picture);
    }
}

// (Re)associate the picture with the given pixmap.
void XRenderPictureWrapper::setPixmap(Pixmap pixmap, XRenderPictureAttributes pa, long paMask) {
    if (m_picture) {
        XRenderFreePicture(m_display, m_picture);
        m_picture = None;
    }

    m_picture = XRenderCreatePicture(m_display, pixmap, m_pictFormat, paMask, &pa);
    XRenderSetPictureFilter(m_display, m_picture, m_pictFilter, NULL, 0);
}

#endif  // USE_XRENDER_COMPOSITING
