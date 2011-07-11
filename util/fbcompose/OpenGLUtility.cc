/** OpenGLUtility.cc file for the fluxbox compositor. */

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


#include "OpenGLUtility.hh"

#include "Logging.hh"
#include "Utility.hh"

using namespace FbCompositor;


//--- OPENGL RESOURCE WRAPPERS -------------------------------------------------

// Buffer holder constructor.
OpenGLBufferHolder::OpenGLBufferHolder() throw() {
    glGenBuffers(1, &m_buffer);
}

// Buffer holder destructor.
OpenGLBufferHolder::~OpenGLBufferHolder() throw() {
    glDeleteBuffers(1, &m_buffer);
}


// Texture holder constructor.
OpenGLTextureHolder::OpenGLTextureHolder() throw() {
    glGenTextures(1, &m_texture);
}

// Texture holder destructor.
OpenGLTextureHolder::~OpenGLTextureHolder() throw() {
    glDeleteTextures(1, &m_texture);
}


//--- FUNCTIONS ----------------------------------------------------------------

/** Converts an X pixmap to an OpenGL texture. */
void FbCompositor::pixmapToTexture(Display *display, Pixmap pixmap, GLuint texture, GLXFBConfig fbConfig,
                                   GLXPixmap glxPixmap, unsigned int width, unsigned int height,
                                   const int *ATTRS) throw() {
#ifdef GLXEW_EXT_texture_from_pixmap
    glBindTexture(GL_TEXTURE_2D, texture);

    if (glxPixmap) {
        glXReleaseTexImageEXT(display, glxPixmap, GLX_BACK_LEFT_EXT);
        glXDestroyPixmap(display, glxPixmap);
        glxPixmap = 0;
    }
    glxPixmap = glXCreatePixmap(display, fbConfig, pixmap, ATTRS);

    if (!glxPixmap) {
        fbLog_warn << "Could not create GLX pixmap for pixmap to texture conversion." << std::endl;
        return;
    } else {
        glXBindTexImageEXT(display, glxPixmap, GLX_BACK_LEFT_EXT, NULL);
    }

    MARK_PARAMETER_UNUSED(height);
    MARK_PARAMETER_UNUSED(width);

#else
    XImage *image = XGetImage(display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
    if (!image) {
        fbLog_warn << "Could not create XImage for pixmap to texture conversion." << std::endl;
        return;
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));

    XDestroyImage(image);

    MARK_PARAMETER_UNUSED(ATTRS);
    MARK_PARAMETER_UNUSED(fbConfig);
    MARK_PARAMETER_UNUSED(glxPixmap);

#endif  // GLXEW_EXT_texture_from_pixmap
}

// Converts screen coordinates to OpenGL coordinates.
void FbCompositor::toOpenGLCoordinates(int screenWidth, int screenHeight, int x, int y, int width, int height,
                                       GLfloat *xLow_gl, GLfloat *xHigh_gl, GLfloat *yLow_gl,
                                       GLfloat *yHigh_gl) throw() {
    *xLow_gl  = ((x * 2.0) / screenWidth) - 1.0;
    *xHigh_gl = (((x + width) * 2.0) / screenWidth) - 1.0;
    *yLow_gl  = 1.0 - ((y * 2.0) / screenHeight);
    *yHigh_gl = 1.0 - (((y + height) * 2.0) / screenHeight);
}
