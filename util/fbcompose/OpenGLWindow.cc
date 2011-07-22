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


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLWindow::OpenGLWindow(const OpenGLScreen &screen, Window windowXID) :
    BaseCompWindow((const BaseScreen&)(screen), windowXID) {

    m_shapePixmap = None;

    m_contentTexturePtr = new OpenGLTexture(screen, GL_TEXTURE_2D, true);
    m_shapeTexturePtr = new OpenGLTexture(screen, GL_TEXTURE_2D, false);
    m_windowPosBufferPtr = new OpenGLBuffer(screen, GL_ARRAY_BUFFER);

    updateWindowPosArray();
}

// Destructor.
OpenGLWindow::~OpenGLWindow() {
    if (m_shapePixmap) {
        XFreePixmap(display(), m_shapePixmap);
    }
}


//--- WINDOW UPDATE FUNCTIONS ------------------------------------------

// Updates the window's contents.
void OpenGLWindow::updateContents() {
    if (isWindowBad()) {
        return;
    }

    updateContentPixmap();
    if (contentPixmap()) {
        m_contentTexturePtr->setPixmap(contentPixmap(), realWidth(), realHeight());
    }

    if (clipShapeChanged()) {
        updateShape();
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

    m_shapeTexturePtr->setPixmap(m_shapePixmap, realWidth(), realHeight());
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

    m_windowPosBufferPtr->bufferData(sizeof(m_windowPosArray), (const GLvoid*)(m_windowPosArray), GL_STATIC_DRAW);
}
