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
#include <X11/Xatom.h>

#include <algorithm>
#include <ostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseCompWindow::BaseCompWindow(Window windowXID) throw() :
    FbTk::FbWindow(windowXID) {

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
        m_damage = XDamageCreate(display(), window(), XDamageReportDeltaRectangles);
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
}


//--- PROPERTY ACCESS ----------------------------------------------------------

// Returns the specified cardinal property.
// This function was overriden, because it would return only one value of the
// property, regardless of how many values there are in it.
std::vector<long> BaseCompWindow::cardinalProperty(Atom propertyAtom) {
    unsigned long nItems;
    long *data;

    if (rawPropertyData(propertyAtom, XA_CARDINAL, &nItems, reinterpret_cast<unsigned char**>(&data))) {
        std::vector<long> actualData(data, data + nItems);
        XFree(data);
        return actualData;
    }

    return std::vector<long>();
}

// Returns the specified pixmap property.
std::vector<Pixmap> BaseCompWindow::pixmapProperty(Atom propertyAtom) {
    unsigned long nItems;
    Pixmap *data;

    if (rawPropertyData(propertyAtom, XA_PIXMAP, &nItems, reinterpret_cast<unsigned char**>(&data))) {
        std::vector<Pixmap> actualData(data, data + nItems);
        XFree(data);
        return actualData;
    }

    return std::vector<Pixmap>();
}

// Returns the specified window property.
std::vector<Window> BaseCompWindow::windowProperty(Atom propertyAtom) {
    unsigned long nItems;
    Window *data;

    if (rawPropertyData(propertyAtom, XA_WINDOW, &nItems, reinterpret_cast<unsigned char**>(&data))) {
        std::vector<Window> actualData(data, data + nItems);
        XFree(data);
        return actualData;
    }

    return std::vector<Window>();
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Add damage to a window.
void BaseCompWindow::addDamage(XRectangle area) throw() {
    area.height = std::min(area.height + 1, (int)realHeight());
    area.width = std::min(area.width + 1, (int)realWidth());
    m_damagedArea.push_back(area);
}

// Reconfigure a window.
void BaseCompWindow::reconfigure(const XConfigureEvent &event) throw() {
    unsigned int oldBorderWidth = borderWidth();
    unsigned int oldHeight = height();
    unsigned int oldWidth = width();
    updateGeometry();

    if ((borderWidth() != oldBorderWidth) || (height() != oldHeight) || (width() != oldWidth)) {
        setClipShapeChanged();
        m_isResized = true;
    }
}

// Set the clip shape as changed.
void BaseCompWindow::setClipShapeChanged() throw() {
    m_clipShapeChanged = true;
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
    updateContentPixmap();
    if (m_clipShapeChanged) {
        updateShape();
    }

    clearDamage();
}

// Update the window's clip shape.
void BaseCompWindow::updateShape() throw() {
    if (m_clipShapeRects) {
        XFree(m_clipShapeRects);
    }

    m_clipShapeRects = XShapeGetRectangles(display(), window(), ShapeClip, &m_clipShapeRectCount, &m_clipShapeRectOrder);
    for (int i = 0; i < m_clipShapeRectCount; i++) {
        m_clipShapeRects[i].height = std::min(m_clipShapeRects[i].height + 2 * borderWidth(), realHeight());
        m_clipShapeRects[i].width = std::min(m_clipShapeRects[i].width + 2 * borderWidth(), realWidth());
    }
}

// Update window's property.
void BaseCompWindow::updateProperty(Atom property, int state) { }


//--- PROTECTED WINDOW MANIPULATION --------------------------------------------

// Removes all damage from the window.
void BaseCompWindow::clearDamage() throw() {
    m_clipShapeChanged = false;
    m_damagedArea.clear();
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
