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
#include "OpenGLUtility.hh"
#include "Utility.hh"

using namespace FbCompositor;


namespace {
    //--- CONSTANTS ------------------------------------------------------------

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
OpenGL2DTexture::OpenGL2DTexture(const OpenGLScreen &screen, bool swizzleAlphaToOne) :
    m_screen(screen) {

    m_display = (Display*)(screen.display());
    m_glxPixmap = 0;
    m_pixmap = None;

    glGenTextures(1, &m_texture);
    bind();

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (swizzleAlphaToOne) {
#ifdef GL_ARB_texture_swizzle
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
#else
#ifdef GL_EXT_texture_swizzle
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A_EXT, GL_ONE);
#endif  // GL_EXT_texture_swizzle
#endif  // GL_ARB_texture_swizzle
    }
}

// Destructor.
OpenGL2DTexture::~OpenGL2DTexture() {
    glDeleteTextures(1, &m_texture);
    if (m_glxPixmap) {
        glXDestroyPixmap(m_display, m_glxPixmap);
    }
    if (m_pixmap) {
        XFreePixmap(m_display, m_pixmap);
    }
}


//------- MUTATORS -------------------------------------------------------------

// Sets the texture's contents to the given pixmap.
void OpenGL2DTexture::setPixmap(Pixmap pixmap, bool managePixmap, int width, int height, bool forceDirect) {
    bind();
    m_height = height;
    m_width = width;

    if (m_pixmap) {
        XFreePixmap(m_display, m_pixmap);
        m_pixmap = None;
    }
    if (managePixmap) {
        m_pixmap = pixmap;
    }

#ifdef GLXEW_EXT_texture_from_pixmap
    if (m_glxPixmap) {
        glXReleaseTexImageEXT(m_display, m_glxPixmap, GLX_BACK_LEFT_EXT);
        glXDestroyPixmap(m_display, m_glxPixmap);
        m_glxPixmap = 0;
    }

    if (!forceDirect) {
        m_glxPixmap = glXCreatePixmap(m_display, m_screen.fbConfig(), pixmap, TEX_PIXMAP_ATTRIBUTES);

        if (!m_glxPixmap) {
            fbLog_info << "Could not create GLX pixmap for pixmap to texture conversion." << std::endl;
            return;
        } else {
            glXBindTexImageEXT(m_display, m_glxPixmap, GLX_BACK_LEFT_EXT, NULL);
        }
    } else 
#else
    MARK_PARAMETER_UNUSED(forceDirect);
#endif  // GLXEW_EXT_texture_from_pixmap

    {
        XImage *image = XGetImage(m_display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
        if (!image) {
            fbLog_info << "Could not create XImage for pixmap to texture conversion." << std::endl;
            return;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));
        XDestroyImage(image);
    }
}


//--- OPENGL TEXTURE PARTITION -------------------------------------------------

//------- CONSTRUCTORS AND DESTRUCTORS -----------------------------------------

// Constructor.
OpenGL2DTexturePartition::OpenGL2DTexturePartition(const OpenGLScreen &screen, bool swizzleAlphaToOne) :
    m_screen(screen) {

    m_display = (Display*)(screen.display());
    m_maxTextureSize = screen.maxTextureSize();
    m_pixmap = None;
    m_swizzleAlphaToOne = swizzleAlphaToOne;

    m_fullHeight = m_fullWidth = m_unitHeight = m_unitWidth = 0;
}

// Destructor.
OpenGL2DTexturePartition::~OpenGL2DTexturePartition() { }


//------- MUTATORS -------------------------------------------------------------

// Sets the texture's contents to the given pixmap.
void OpenGL2DTexturePartition::setPixmap(Pixmap pixmap, bool managePixmap, int width, int height, int depth, bool forceDirect) {
    // Handle the pixmap and its GC.
    if (m_pixmap) {
        XFreePixmap(m_display, m_pixmap);
        m_pixmap = None;
    }
    if (managePixmap) {
        m_pixmap = pixmap;
    }
    GC gc = XCreateGC(m_display, pixmap, 0, NULL);

    // Set partition's dimensions.
    m_fullHeight = height;
    m_fullWidth = width;
    m_unitHeight = ((height - 1) / m_maxTextureSize) + 1;
    m_unitWidth = ((width - 1) / m_maxTextureSize) + 1;

    int totalUnits = m_unitHeight * m_unitWidth;

    // Adjust number of stored partitions.
    while ((size_t)(totalUnits) > m_partitions.size()) {
        TexturePart partition;
        partition.borders = 0;
        partition.texture = new OpenGL2DTexture(m_screen, m_swizzleAlphaToOne);
        m_partitions.push_back(partition);
    }
    while ((size_t)(totalUnits) < m_partitions.size()) {
        m_partitions.pop_back();
    }

    // Create partitions.
    for (int i = 0; i < m_unitHeight; i++) {
        for (int j = 0; j < m_unitWidth; j++) {
            // Partition's index and extents.
            int idx = partitionIndex(j, i);
            int partHeight = std::min(m_fullHeight - i * m_maxTextureSize, m_maxTextureSize);
            int partWidth = std::min(m_fullWidth - j * m_maxTextureSize, m_maxTextureSize);

            // Create partition's pixmap.
            Pixmap partPixmap = XCreatePixmap(m_display, m_screen.rootWindow().window(), partWidth, partHeight, depth);
            XCopyArea(m_display, pixmap, partPixmap, gc, j * m_maxTextureSize, i * m_maxTextureSize, partWidth, partHeight, 0, 0);

            // Fill adjacent border field.
            int borders = 0;
            borders |= ((i == 0) ? BORDER_NORTH : 0);
            borders |= ((j == 0) ? BORDER_WEST : 0);
            borders |= ((i == (m_unitHeight - 1)) ? BORDER_SOUTH : 0);
            borders |= ((j == (m_unitWidth - 1)) ? BORDER_EAST : 0);

            // Set up the partition.
            m_partitions[idx].borders = borders;
            m_partitions[idx].texture->setPixmap(partPixmap, true, partWidth, partHeight, forceDirect);
        }
    }

    XFreeGC(m_display, gc);
}
