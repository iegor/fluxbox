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

#include "Atoms.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
XRenderWindow::XRenderWindow(const BaseScreen &screen, Window windowXID, const char *pictFilter) :
    BaseCompWindow(screen, windowXID),
    m_pictFilter(pictFilter) {

    m_maskPixmap = None;

    XRenderPictFormat *contentPictFormat = XRenderFindVisualFormat(display(), visual());
    m_contentPicture = new XRenderPictureWrapper(display(), contentPictFormat, m_pictFilter);

    XRenderPictFormat *maskPictFormat = XRenderFindStandardFormat(display(), PictStandardARGB32);
    m_maskPicture = new XRenderPictureWrapper(display(), maskPictFormat, m_pictFilter);
}

// Destructor.
XRenderWindow::~XRenderWindow() {
    if (m_maskPixmap) {
        XFreePixmap(display(), m_maskPixmap);
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

    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long paMask = CPSubwindowMode;

    m_contentPicture->setPixmap(contentPixmap(), pa, paMask);

    clearDamage();
}

// Update window's property.
void XRenderWindow::updateProperty(Atom property, int state) {
    BaseCompWindow::updateProperty(property, state);

    if (property == Atoms::opacityAtom()) {
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
void XRenderWindow::updateMaskPicture() {
    if (!m_maskPicture || isResized()) {
        if (m_maskPixmap) {
            XFreePixmap(display(), m_maskPixmap);
            m_maskPixmap = None;
        }
        m_maskPixmap = XCreatePixmap(display(), window(), realWidth(), realHeight(), 32);

        m_maskPicture->setPixmap(m_maskPixmap);
    }

    XRenderColor color = { 0, 0, 0, 0 };
    XRenderFillRectangle(display(), PictOpSrc, direct_maskPicture(), &color, 0, 0, realWidth(), realHeight());

    color.alpha = (unsigned long)((alpha() * 0xffff) / 255.0);
    XRenderFillRectangles(display(), PictOpSrc, direct_maskPicture(), &color, clipShapeRects(), clipShapeRectCount());
}
