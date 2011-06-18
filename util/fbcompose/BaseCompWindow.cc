/** BaseCompWindow.cc file for the fluxbox compositor. */

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


#include "BaseCompWindow.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>

#include <algorithm>
#include <ostream>

using namespace FbCompositor;


//--- STATIC VARIABLES -------------------------------------------------

// Opacity atom.
Atom BaseCompWindow::m_opacityAtom = 0;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseCompWindow::BaseCompWindow(Window windowXID) throw(InitException) :
    FbTk::FbWindow(windowXID) {

    // Set up atoms and properties.
    static bool atomsInitialized = false;
    if (!atomsInitialized) {
        m_opacityAtom = XInternAtom(display(), "_NET_WM_WINDOW_OPACITY", False);
        atomsInitialized = true;
    }
    m_alpha = singlePropertyValue<long>(m_opacityAtom, 0xff) & 0xff;

    // Set up other window attributes.
    XWindowAttributes xwa;
    XGetWindowAttributes(display(), window(), &xwa);

    m_class = xwa.c_class;
    m_isMapped = (xwa.map_state != IsUnmapped);
    m_isResized = true;

    m_clipShapeChanged = true;
    m_clipShapeRects = 0;
    m_clipShapeRectCount = 0;
    m_clipShapeRectOrder = Unsorted;

    if (m_class == InputOutput) {
        m_damage = XDamageCreate(display(), window(), XDamageReportNonEmpty);
    } else {
        m_damage = 0;
    }
    m_contentPixmap = None;
}

// Destructor.
BaseCompWindow::~BaseCompWindow() throw() {
    if (m_clipShapeRects) {
        XFree(m_clipShapeRects);
    }
    if (m_contentPixmap) {
        XFreePixmap(display(), m_contentPixmap);
    }
    // m_damage is apparently destroyed server-side.
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Add damage to a window.
void BaseCompWindow::addDamage() throw() {
    m_isDamaged = true;
}

// Mark the window as mapped.
void BaseCompWindow::setMapped() throw() {
    m_isMapped = true;
}

// Mark the window as unmapped.
void BaseCompWindow::setUnmapped() throw() {
    m_isMapped = false;
}

// Update the window's contents.
void BaseCompWindow::updateContents() {
    if (isWindowBad()) {
        return;
    }

    updateContentPixmap();
    if (m_clipShapeChanged) {
        updateShape();
    }

    clearDamage();
}

// Update window's geometry.
void BaseCompWindow::updateGeometry(const XConfigureEvent &/*event*/) throw() {
    unsigned int oldBorderWidth = borderWidth();
    unsigned int oldHeight = height();
    unsigned int oldWidth = width();
    FbTk::FbWindow::updateGeometry();

    if ((borderWidth() != oldBorderWidth) || (height() != oldHeight) || (width() != oldWidth)) {
        setClipShapeChanged();
        m_isResized = true;
    }
}

// Update the window's clip shape.
void BaseCompWindow::updateShape() {
    if (m_clipShapeRects) {
        XFree(m_clipShapeRects);
        m_clipShapeRects = NULL;
    }
    m_clipShapeRects = XShapeGetRectangles(display(), window(), ShapeClip, &m_clipShapeRectCount, &m_clipShapeRectOrder);

    for (int i = 0; i < m_clipShapeRectCount; i++) {
        m_clipShapeRects[i].height = std::min(m_clipShapeRects[i].height + 2 * borderWidth(), realHeight());
        m_clipShapeRects[i].width = std::min(m_clipShapeRects[i].width + 2 * borderWidth(), realWidth());
    }
}

// Update window's property.
void BaseCompWindow::updateProperty(Atom property, int /*state*/) {
    if (property == m_opacityAtom) {
        m_alpha = singlePropertyValue<long>(m_opacityAtom, 0xff) & 0xff;
    }
}


// Set the clip shape as changed.
void BaseCompWindow::setClipShapeChanged() throw() {
    m_clipShapeChanged = true;
}


//--- PROTECTED WINDOW MANIPULATION --------------------------------------------

// Removes all damage from the window.
void BaseCompWindow::clearDamage() throw() {
    m_clipShapeChanged = false;
    m_isDamaged = false;
    m_isResized = false;
}

// Updates the window's content pixmap.
void BaseCompWindow::updateContentPixmap() throw() {
    // We must reset the damage here, otherwise we may miss damage events.
    XDamageSubtract(display(), m_damage, None, None);

    if (m_contentPixmap) {
        XFreePixmap(display(), m_contentPixmap);
        m_contentPixmap = None;
    }
    m_contentPixmap = XCompositeNameWindowPixmap(display(), window());
}


//--- OTHER FUNCTIONS --------------------------------------------------

// Checks whether the current window is bad.
bool BaseCompWindow::isWindowBad() {
    static XWindowAttributes xwa;
    return (!XGetWindowAttributes(display(), window(), &xwa));
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Reads and returns raw property contents.
bool BaseCompWindow::rawPropertyData(Atom propertyAtom, Atom propertyType,
                                     unsigned long *itemCount_return, unsigned char **data_return) {
    Atom actualType;
    int actualFormat;
    unsigned long bytesLeft;

    if (property(propertyAtom, 0, 0x7fffffff, False, propertyType,
                 &actualType, &actualFormat, itemCount_return, &bytesLeft, data_return)) {
        if (*itemCount_return > 0) {
            return true;
        }
    }

    return false;
}


//--- OPERATORS ----------------------------------------------------------------

// << output stream operator for the window class.
std::ostream &FbCompositor::operator<<(std::ostream& out, const BaseCompWindow& w) {
    out << "Window " << w.window() << ": Geometry[" << w.x() << "," << w.y()
        << "," << w.width() << "," << w.height() << " " << w.borderWidth()
        << "] Depth[" << w.depth() << "] " << w.isMapped() << " " << w.isDamaged();
    return out;
}
