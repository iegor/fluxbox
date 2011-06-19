/** XRenderWindow.cc file for the fluxbox compositor. */

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

#include "XRenderWindow.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
XRenderWindow::XRenderWindow(Window windowXID) throw(InitException) :
    BaseCompWindow(windowXID) {

    m_maskPicture = None;
    m_maskPixmap = None;
    m_pictFormat = XRenderFindVisualFormat(display(), visual());
    m_picture = None;
}

// Destructor.
XRenderWindow::~XRenderWindow() throw() {
    if (m_maskPicture) {
        XRenderFreePicture(display(), m_maskPicture);
    }
    if (m_maskPixmap) {
        XFreePixmap(display(), m_maskPixmap);
    }
    if (m_picture) {
        XRenderFreePicture(display(), m_picture);
    }
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Update the window's contents.
void XRenderWindow::updateContents() {
    if (isWindowBad()) {
        return;
    }

    updateContentPixmap();
    if (!m_maskPicture || clipShapeChanged()) {
        updateShape();
    }

    if (m_picture) {
        XRenderFreePicture(display(), m_picture);
        m_picture = None;
    }

    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long paMask = CPSubwindowMode;

    m_picture = XRenderCreatePicture(display(), contentPixmap(), m_pictFormat, paMask, &pa);

    clearDamage();
}

// Update window's property.
void XRenderWindow::updateProperty(Atom property, int state) {
    BaseCompWindow::updateProperty(property, state);

    if (property == m_opacityAtom) {
        updateMaskPicture();
    }
}


//--- PROTECTED WINDOW MANIPULATION --------------------------------------------

// Update the window's clip shape.
void XRenderWindow::updateShape() {
    BaseCompWindow::updateShape();
    updateMaskPicture();
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Update the window's mask picture.
void XRenderWindow::updateMaskPicture() throw() {
    if (!m_maskPicture || isResized()) {
        if (m_maskPixmap) {
            XFreePixmap(display(), m_maskPixmap);
            m_maskPixmap = None;
        }
        if (m_maskPicture) {
            XRenderFreePicture(display(), m_maskPicture);
            m_maskPicture = None;
        }

        m_maskPixmap = XCreatePixmap(display(), window(), realWidth(), realHeight(), 32);
        m_maskPicture = XRenderCreatePicture(display(), m_maskPixmap, XRenderFindStandardFormat(display(), PictStandardARGB32), 0, NULL);
    }

    XRenderColor color = { 0, 0, 0, 0 };
    XRenderFillRectangle(display(), PictOpSrc, m_maskPicture, &color, 0, 0, realWidth(), realHeight());

    color.alpha = (unsigned int)((alpha() * 0xffff) / 255.0);
    XRenderFillRectangles(display(), PictOpSrc, m_maskPicture, &color, clipShapeRects(), clipShapeRectCount());
}
