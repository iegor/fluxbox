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

#include "Atoms.hh"
#include "BaseScreen.hh"
#include "Logging.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>

#include <algorithm>
#include <ostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseCompWindow::BaseCompWindow(const BaseScreen &screen, Window windowXID) :
    FbTk::FbWindow(windowXID),
    m_screen(screen) {

    XWindowAttributes xwa;
    XGetWindowAttributes(display(), window(), &xwa);

    m_class = xwa.c_class;
    m_isMapped = (xwa.map_state != IsUnmapped);
    m_isRemapped = true;
    m_isResized = true;
    m_visual = xwa.visual;

    m_clipShapeChanged = true;
    m_clipShapeRects = 0;
    m_clipShapeRectCount = 0;
    m_clipShapeRectOrder = Unsorted;

    m_isIgnored = false;

    if (m_class == InputOutput) {
        m_damage = XDamageCreate(display(), window(), XDamageReportNonEmpty);
    } else {
        m_damage = 0;
    }

    m_contentPixmap = None;

    updateAlpha();
    updateWindowType();
}

// Destructor.
BaseCompWindow::~BaseCompWindow() {
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
void BaseCompWindow::addDamage() {
    m_isDamaged = true;
}

// Mark the window as mapped.
void BaseCompWindow::setMapped() {
    m_isMapped = true;
    m_isRemapped = true;
}

// Mark the window as unmapped.
void BaseCompWindow::setUnmapped() {
    m_isMapped = false;
}

// Update the window's contents.
// Note: this is an example implementation of this function. You should fully
// override it in derived classes.
void BaseCompWindow::updateContents() {
    updateContentPixmap();
    if (m_clipShapeChanged) {
        updateShape();
    }

    clearDamage();
}

// Update window's geometry.
void BaseCompWindow::updateGeometry() {
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

    // We have to adjust the size here to account for borders.
    for (int i = 0; i < m_clipShapeRectCount; i++) {
        m_clipShapeRects[i].height = std::min(m_clipShapeRects[i].height + 2 * borderWidth(), realHeight());
        m_clipShapeRects[i].width = std::min(m_clipShapeRects[i].width + 2 * borderWidth(), realWidth());
    }
}

// Update window's property.
void BaseCompWindow::updateProperty(Atom property, int /*state*/) {
    if (property == Atoms::opacityAtom()) {
        updateAlpha();
    } else if (property == Atoms::windowTypeAtom()) {
        updateWindowType();
    }
}


// Set the clip shape as changed.
void BaseCompWindow::setClipShapeChanged() {
    m_clipShapeChanged = true;
}


//--- PROTECTED WINDOW MANIPULATION --------------------------------------------

// Removes all damage from the window.
void BaseCompWindow::clearDamage() {
    m_clipShapeChanged = false;
    m_isDamaged = false;
    m_isRemapped = false;
    m_isResized = false;
}

// Updates the window's content pixmap.
void BaseCompWindow::updateContentPixmap() {
    // We must reset the damage here, otherwise we may miss damage events.
    XDamageSubtract(display(), m_damage, None, None);

    if (m_isResized || m_isRemapped) {
        XGrabServer(display());

        XWindowAttributes xwa;
        if (XGetWindowAttributes(display(), window(), &xwa)) {
            if (xwa.map_state == IsViewable) {
                if (m_contentPixmap) {
                    XFreePixmap(display(), m_contentPixmap);
                    m_contentPixmap = None;
                }
                m_contentPixmap = XCompositeNameWindowPixmap(display(), window());
            }
        }
        XUngrabServer(display());
    }
}


//--- PROPERTY UPDATE FUNCTIONS ----------------------------------------

// Updates window's alpha.
void BaseCompWindow::updateAlpha() {
    m_alpha = singlePropertyValue<long>(Atoms::opacityAtom(), 0xff) & 0xff;
}

// Updates the type of the window.
void BaseCompWindow::updateWindowType() {
    static std::vector< std::pair<Atom, WindowType> > typeList = Atoms::windowTypeAtomList();
    Atom rawType = singlePropertyValue<Atom>(Atoms::windowTypeAtom(), None);

    m_type = WinType_Normal;
    for (size_t i = 0; i < typeList.size(); i++) {
        if (rawType == typeList[i].first) {
            m_type = typeList[i].second;
            break;
        }
    }
}


//--- CONVENIENCE FUNCTIONS ----------------------------------------------------

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
    out << "Window " << std::hex << w.window() << ": Geometry[" << std::dec << w.x()
        << "," << w.y() << "," << w.width() << "," << w.height() << " " << w.borderWidth()
        << "] Depth=" << w.depth() << " Type=" << w.type() << " Map=" << w.isMapped()
        << " Dmg=" << w.isDamaged() << " Ignore=" << w.isIgnored();
    return out;
}
