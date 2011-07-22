/** OpenGLResources.cc file for the fluxbox compositor. */

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


#include "OpenGLResources.hh"

#include "Logging.hh"
#include "OpenGLScreen.hh"
#include "Utility.hh"

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

namespace {

    // Attributes of the textures' GLX pixmaps.
    static const int TEX_PIXMAP_ATTRIBUTES[] = {
#ifdef GLXEW_EXT_texture_from_pixmap
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
        None
#else
        None
#endif  // GLXEW_EXT_texture_from_pixmap
    };

}


//--- OPENGL BUFFER WRAPPER ----------------------------------------------------

//------- CONSTRUCTORS AND DESTRUCTORS -----------------------------------------

// Constructor.
OpenGLBuffer::OpenGLBuffer(const OpenGLScreen &screen, GLenum targetBuffer) :
    m_screen(screen) {

    m_display = (Display*)(screen.display());
    m_target = targetBuffer;

    glGenBuffers(1, &m_buffer);
}

// Destructor.
OpenGLBuffer::~OpenGLBuffer() {
    glDeleteBuffers(1, &m_buffer);
}


//--- OPENGL TEXTURE WRAPPER ---------------------------------------------------

//------- CONSTRUCTORS AND DESTRUCTORS -----------------------------------------

// Constructor.
OpenGLTexture::OpenGLTexture(const OpenGLScreen &screen, GLenum targetTexture, bool swizzleAlphaToOne) :
    m_screen(screen) {

    m_display = (Display*)(screen.display());
    m_glxPixmap = 0;

    // TODO: Support for all texture targets (GLX pixmap creation is broken).
    m_target = targetTexture;
    if (m_target != GL_TEXTURE_2D) {
        fbLog_warn << "Someone created a non-2D OpenGLTexture object. Expect glitches." << std::endl;
    }

    glGenTextures(1, &m_texture);
    bind();

    glTexParameterf(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (swizzleAlphaToOne) {
#ifdef GL_ARB_texture_swizzle
        glTexParameteri(m_target, GL_TEXTURE_SWIZZLE_A, GL_ONE);
#else
#ifdef GL_EXT_texture_swizzle
        glTexParameteri(m_target, GL_TEXTURE_SWIZZLE_A_EXT, GL_ONE);
#endif  // GL_EXT_texture_swizzle
#endif  // GL_ARB_texture_swizzle
    }
}

// Destructor.
OpenGLTexture::~OpenGLTexture() {
    glDeleteTextures(1, &m_texture);
}


//------- MUTATORS -------------------------------------------------------------

// Sets the texture's contents to the given pixmap.
void OpenGLTexture::setPixmap(Pixmap pixmap, int width, int height) {
    bind();

#ifdef GLXEW_EXT_texture_from_pixmap
    if (m_glxPixmap) {
        glXReleaseTexImageEXT(m_display, m_glxPixmap, GLX_BACK_LEFT_EXT);
        glXDestroyPixmap(m_display, m_glxPixmap);
        m_glxPixmap = 0;
    }
    m_glxPixmap = glXCreatePixmap(m_display, m_screen.fbConfig(), pixmap, TEX_PIXMAP_ATTRIBUTES);

    if (!m_glxPixmap) {
        fbLog_info << "Could not create GLX pixmap for pixmap to texture conversion." << std::endl;
        return;
    } else {
        glXBindTexImageEXT(m_display, m_glxPixmap, GLX_BACK_LEFT_EXT, NULL);
    }

    MARK_PARAMETER_UNUSED(height);
    MARK_PARAMETER_UNUSED(width);

#else
    XImage *image = XGetImage(m_display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
    if (!image) {
        fbLog_info << "Could not create XImage for pixmap to texture conversion." << std::endl;
        return;
    }

    glTexImage2D(m_target, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));
    XDestroyImage(image);

#endif  // GLXEW_EXT_texture_from_pixmap
}
