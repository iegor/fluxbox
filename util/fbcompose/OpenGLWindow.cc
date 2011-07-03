/** OpenGLWindow.cc file for the fluxbox compositor. */

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


#include "OpenGLWindow.hh"

#ifdef USE_OPENGL_COMPOSITING


#include "BaseScreen.hh"
#include "Logging.hh"
#include "OpenGLUtility.hh"

#include <X11/Xutil.h>

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

namespace {

#ifdef GLXEW_EXT_texture_from_pixmap
    // Attributes of the contents' GLX pixmap.
    static const int TEX_PIXMAP_ATTRIBUTES[] = {
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
        None
    };
#endif  // GLXEW_EXT_texture_from_pixmap

}


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLWindow::OpenGLWindow(const BaseScreen &screen, Window windowXID, GLXFBConfig fbConfig) throw(InitException) :
    BaseCompWindow(screen, windowXID) {

    m_fbConfig = fbConfig;
    m_shapePixmap = None;
    m_rootWidth = screen.rootWindow().width();
    m_rootHeight = screen.rootWindow().height();

    // Create OpenGL elements.
    m_contentTexturePtr = new OpenGLTextureHolder();
    m_windowPosBufferPtr = new OpenGLBufferHolder();

    // Fill window position array.
    updateWindowPosArray();

    // Initialize the content texture.
    glBindTexture(GL_TEXTURE_2D, rawContentTexture());

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef GLXEW_EXT_texture_from_pixmap
    m_glxContents = 0;
#endif
}

// Destructor.
OpenGLWindow::~OpenGLWindow() throw() {
    if (m_shapePixmap) {
        XFreePixmap(display(), m_shapePixmap);
    }

#ifdef GLXEW_EXT_texture_from_pixmap
    if (m_glxContents) {
        glXDestroyPixmap(display(), m_glxContents);
    }
#endif
}


//--- WINDOW UPDATE FUNCTIONS ------------------------------------------

// Updates the window's contents.
void OpenGLWindow::updateContents() throw(RuntimeException) {
    if (isWindowBad()) {
        return;
    }

    updateContentPixmap();
    if (clipShapeChanged()) {
        updateShape();
    }

    if (contentPixmap() && m_shapePixmap) {
        GC gc = XCreateGC(display(), m_shapePixmap, 0, 0);
        XSetGraphicsExposures(display(), gc, False);

        XSetPlaneMask(display(), gc, 0x00ffffff);
        if (!XCopyArea(display(), contentPixmap(), m_shapePixmap, gc, 0, 0, realWidth(), realHeight(), 0, 0)) {
            return;
        }

        XFreeGC(display(), gc);

#ifdef GLXEW_EXT_texture_from_pixmap
        glBindTexture(GL_TEXTURE_2D, rawContentTexture());
        glXReleaseTexImageEXT(display(), m_glxContents, GLX_BACK_LEFT_EXT);

        // Bind the pixmap to a GLX texture.
        if (m_glxContents) {
            glXDestroyPixmap(display(), m_glxContents);
            m_glxContents = 0;
        }
        m_glxContents = glXCreatePixmap(display(), m_fbConfig, m_shapePixmap, TEX_PIXMAP_ATTRIBUTES);

        glXBindTexImageEXT(display(), m_glxContents, GLX_BACK_LEFT_EXT, NULL);

#else
        // Convert the content pixmap to an XImage to access its raw contents.
        XImage *image = XGetImage(display(), m_shapePixmap, 0, 0, realWidth(), realHeight(), AllPlanes, ZPixmap);
        if (!image) {
            fbLog_warn << "Cannot create XImage for window " << std::hex << window()
                       << ". It's probably nothing." << std::endl;
            return;
        }

        // Update the texture.
        glBindTexture(GL_TEXTURE_2D, rawContentTexture());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, realWidth(), realHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));

        XDestroyImage(image);

#endif  // GLXEW_EXT_texture_from_pixmap
    }

    clearDamage();
}

// Updates window's geometry.
void OpenGLWindow::updateGeometry(const XConfigureEvent &event) throw() {
    BaseCompWindow::updateGeometry(event);
    updateWindowPosArray();
}

// Updates the window's shape.
void OpenGLWindow::updateShape() throw(RuntimeException) {
    BaseCompWindow::updateShape();

    if (m_shapePixmap) {
        XFreePixmap(display(), m_shapePixmap);
        m_shapePixmap = None;
    }
    m_shapePixmap = XCreatePixmap(display(), window(), realWidth(), realHeight(), depth());
    if (!m_shapePixmap) {
        return;     // In case the window was unmapped/destroyed.
    }

    GC gc = XCreateGC(display(), m_shapePixmap, 0, 0);
    XSetGraphicsExposures(display(), gc, False);
    XSetPlaneMask(display(), gc, 0xffffffff);

    XSetForeground(display(), gc, 0x00000000);
    XFillRectangle(display(), m_shapePixmap, gc, 0, 0, realWidth(), realHeight());

    XSetForeground(display(), gc, 0xff000000);
    // TODO: Fix rectangle ordering mismatch.
    XSetClipRectangles(display(), gc, 0, 0, clipShapeRects(), clipShapeRectCount(), Unsorted);
    XFillRectangle(display(), m_shapePixmap, gc, 0, 0, realWidth(), realHeight());

    XFreeGC(display(), gc);
}

// Updates the window position vertex array.
void OpenGLWindow::updateWindowPosArray() throw() {
    GLfloat xLow, xHigh, yLow, yHigh;
    toOpenGLCoordinates(screen().rootWindow().width(), screen().rootWindow().height(),
                        x(), y(), realWidth(), realHeight(), &xLow, &xHigh, &yLow, &yHigh);

    m_windowPosArray[0] = m_windowPosArray[4] = xLow;
    m_windowPosArray[2] = m_windowPosArray[6] = xHigh;
    m_windowPosArray[1] = m_windowPosArray[3] = yLow;
    m_windowPosArray[5] = m_windowPosArray[7] = yHigh;

    glBindBuffer(GL_ARRAY_BUFFER, rawWindowPosBuffer());
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_windowPosArray), (const GLvoid*)(m_windowPosArray), GL_STATIC_DRAW);
}

#endif  // USE_OPENGL_COMPOSITING
