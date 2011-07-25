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
#include "OpenGLScreen.hh"
#include "OpenGLUtility.hh"
#include "Utility.hh"

#include <X11/Xutil.h>

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLWindow::OpenGLWindow(const OpenGLScreen &screen, Window windowXID) :
    BaseCompWindow((const BaseScreen&)(screen), windowXID) {

    m_contentTexturePartition = new OpenGL2DTexturePartition(screen, true);
    m_shapeTexturePartition = new OpenGL2DTexturePartition(screen, false);

    updateWindowPosArray();
}

// Destructor.
OpenGLWindow::~OpenGLWindow() { }


//--- WINDOW UPDATE FUNCTIONS ------------------------------------------

// Updates the window's contents.
void OpenGLWindow::updateContents() {
    if (isWindowBad()) {
        return;
    }

    updateContentPixmap();
    if (contentPixmap()) {
        m_contentTexturePartition->setPixmap(contentPixmap(), false, realWidth(), realHeight(), depth());
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

    Pixmap shapePixmap = XCreatePixmap(display(), window(), realWidth(), realHeight(), depth());

    GC gc = XCreateGC(display(), shapePixmap, 0, 0);
    XSetGraphicsExposures(display(), gc, False);
    XSetPlaneMask(display(), gc, 0xffffffff);

    XSetForeground(display(), gc, 0x00000000);
    XFillRectangle(display(), shapePixmap, gc, 0, 0, realWidth(), realHeight());

    XSetForeground(display(), gc, 0xffffffff);
    // TODO: Fix rectangle ordering mismatch.
    XSetClipRectangles(display(), gc, 0, 0, clipShapeRects(), clipShapeRectCount(), Unsorted);
    XFillRectangle(display(), shapePixmap, gc, 0, 0, realWidth(), realHeight());

    XFreeGC(display(), gc);

    m_shapeTexturePartition->setPixmap(shapePixmap, true, realWidth(), realHeight(), depth());
}

// Updates the window position vertex array.
void OpenGLWindow::updateWindowPosArray() {
    GLfloat xLow, xHigh, yLow, yHigh;

    int maxTextureSize = ((const OpenGLScreen&)(screen())).maxTextureSize();

    int unitWidth = ((realWidth() - 1) / maxTextureSize) + 1;
    int unitHeight = ((realHeight() - 1) / maxTextureSize) + 1;
    int totalUnits = unitWidth * unitHeight;

    while ((size_t)(totalUnits) > m_windowPosBuffer.size()) {
        m_windowPosBuffer.push_back(OpenGLBufferPtr(new OpenGLBuffer((const OpenGLScreen&)(screen()), GL_ARRAY_BUFFER)));
    }

    for (int i = 0; i < unitHeight; i++) {
        for (int j = 0; j < unitWidth; j++) {
            int idx = i * unitWidth + j;
            int partX = x() + j * maxTextureSize;
            int partY = y() + i * maxTextureSize;
            int partHeight = std::min(int(realHeight() - i * maxTextureSize), maxTextureSize);
            int partWidth = std::min(int(realWidth() - j * maxTextureSize), maxTextureSize);

            toOpenGLCoordinates(screen().rootWindow().width(), screen().rootWindow().height(),
                                partX, partY, partWidth, partHeight, &xLow, &xHigh, &yLow, &yHigh);
            m_windowPosArray[0] = m_windowPosArray[4] = xLow;
            m_windowPosArray[2] = m_windowPosArray[6] = xHigh;
            m_windowPosArray[1] = m_windowPosArray[3] = yLow;
            m_windowPosArray[5] = m_windowPosArray[7] = yHigh;
            
            m_windowPosBuffer[idx]->bufferData(sizeof(m_windowPosArray), (const GLvoid*)(m_windowPosArray), GL_STATIC_DRAW);
        }
    }
}
