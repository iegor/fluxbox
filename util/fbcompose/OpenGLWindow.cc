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

#include "BaseScreen.hh"
#include "Logging.hh"
#include "OpenGLUtility.hh"

#include <X11/Xutil.h>

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

namespace {

    // Attributes of the contents' GLX pixmap.
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


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLWindow::OpenGLWindow(const BaseScreen &screen, Window windowXID, GLXFBConfig fbConfig) :
    BaseCompWindow(screen, windowXID) {

    m_glxContents = None;
    m_glxShape = None;
    m_fbConfig = fbConfig;
    m_shapePixmap = None;

    // Create OpenGL elements.
    m_contentTexturePtr = new OpenGLTextureWrapper();
    m_shapeTexturePtr = new OpenGLTextureWrapper();
    m_windowPosBufferPtr = new OpenGLBufferWrapper();

    // Fill window position array.
    updateWindowPosArray();

    // Initialize the content texture.
    glBindTexture(GL_TEXTURE_2D, direct_contentTexture());

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef GL_ARB_texture_swizzle
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
#else
#ifdef GL_EXT_texture_swizzle
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A_EXT, GL_ONE);
#endif  // GL_EXT_texture_swizzle
#endif  // GL_ARB_texture_swizzle

    // Initialize the shape texture.
    glBindTexture(GL_TEXTURE_2D, direct_shapeTexture());

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// Destructor.
OpenGLWindow::~OpenGLWindow() {
    if (m_shapePixmap) {
        XFreePixmap(display(), m_shapePixmap);
    }
    if (m_glxContents) {
        glXDestroyPixmap(display(), m_glxContents);
    }
    if (m_glxShape) {
        glXDestroyPixmap(display(), m_glxShape);
    }
}


//--- WINDOW UPDATE FUNCTIONS ------------------------------------------

// Updates the window's contents.
void OpenGLWindow::updateContents() {
    if (isWindowBad()) {
        return;
    }

    updateContentPixmap();
    if (clipShapeChanged()) {
        updateShape();
    }

    if (contentPixmap()) {
        pixmapToTexture(display(), contentPixmap(), direct_contentTexture(), m_fbConfig,
                        m_glxContents, realWidth(), realHeight(), TEX_PIXMAP_ATTRIBUTES);
    }

    clearDamage();
}

// Updates window's geometry.
void OpenGLWindow::updateGeometry(const XConfigureEvent &event) {
    BaseCompWindow::updateGeometry(event);
    updateWindowPosArray();
}

// Updates the window's shape.
void OpenGLWindow::updateShape() {
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

    XSetForeground(display(), gc, 0xffffffff);
    // TODO: Fix rectangle ordering mismatch.
    XSetClipRectangles(display(), gc, 0, 0, clipShapeRects(), clipShapeRectCount(), Unsorted);
    XFillRectangle(display(), m_shapePixmap, gc, 0, 0, realWidth(), realHeight());

    XFreeGC(display(), gc);

    pixmapToTexture(display(), m_shapePixmap, direct_shapeTexture(), m_fbConfig,
                    m_glxShape, realWidth(), realHeight(), TEX_PIXMAP_ATTRIBUTES);
}

// Updates the window position vertex array.
void OpenGLWindow::updateWindowPosArray() {
    GLfloat xLow, xHigh, yLow, yHigh;
    toOpenGLCoordinates(screen().rootWindow().width(), screen().rootWindow().height(),
                        x(), y(), realWidth(), realHeight(), &xLow, &xHigh, &yLow, &yHigh);

    m_windowPosArray[0] = m_windowPosArray[4] = xLow;
    m_windowPosArray[2] = m_windowPosArray[6] = xHigh;
    m_windowPosArray[1] = m_windowPosArray[3] = yLow;
    m_windowPosArray[5] = m_windowPosArray[7] = yHigh;

    glBindBuffer(GL_ARRAY_BUFFER, direct_windowPosBuffer());
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_windowPosArray), (const GLvoid*)(m_windowPosArray), GL_STATIC_DRAW);
}
