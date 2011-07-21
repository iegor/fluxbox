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

    m_alpha = singlePropertyValue<long>(Atoms::opacityAtom(), 0xff) & 0xff;
    m_isRenderable = true;

    XWindowAttributes xwa;
    XGetWindowAttributes(display(), window(), &xwa);

    m_class = xwa.c_class;
    m_isMapped = (xwa.map_state != IsUnmapped);
    m_isResized = true;
    m_visual = xwa.visual;

    m_clipShapeChanged = true;
    m_clipShapeRects = 0;
    m_clipShapeRectCount = 0;
    m_clipShapeRectOrder = Unsorted;

#ifdef HAVE_XDAMAGE
    if (m_class == InputOutput) {
        m_damage = XDamageCreate(display(), window(), XDamageReportNonEmpty);
    } else {
        m_damage = 0;
    }
#endif  // HAVE_XDAMAGE

    m_contentPixmap = None;

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
}

// Mark the window as unmapped.
void BaseCompWindow::setUnmapped() {
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
void BaseCompWindow::updateGeometry(const XConfigureEvent &/*event*/) {
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
    if (property == Atoms::opacityAtom()) {
        m_alpha = singlePropertyValue<long>(Atoms::opacityAtom(), 0xff) & 0xff;
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
    m_isResized = false;
}

// Updates the window's content pixmap.
void BaseCompWindow::updateContentPixmap() {
#ifdef HAVE_XDAMAGE
    // We must reset the damage here, otherwise we may miss damage events.
    XDamageSubtract(display(), m_damage, None, None);
#endif  // HAVE_XDAMAGE

    // You may sometimes get a cluster of X errors, which begin with a BadMatch
    // in XCompositeNameWindowPixmap in this function. That error is raised
    // whenever XCompositeNameWindowPixmap tries to access the contents of an
    // unmapped window.
    //
    // Why does the compositor try to access contents of an unmapped window?
    // The reason is the way the X server sends events to the client
    // applications. If a window is unmapped, the client will get a damage
    // event, followed by an unmap event. Sometimes there is a gap between the
    // two and sometimes the compositor will attempt to redraw the screen in
    // that gap. Since at that point the window is unmapped, but the compositor
    // is not aware of it, the errors are thrown.
    //
    // Why not just check for window's map state using XGetWindowAttributes?
    // Well, I tried it and this error condition *still* occurs, although not
    // as often. On the plus side, this only happens when several windows are
    // being mapped/unmapped in quick succession (going through the menu bar,
    // for example), so it shouldn't be overly noticeable. It can probably be
    // fixed properly, but for the time being it is too much trouble for its
    // worth. You are welcome to give this issue a shot, if you want to.
    //
    // TODO: Fix this.

    m_contentPixmap = None;
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


//--- OPERATORS ----------------------------------------------------------------

// << output stream operator for the window class.
std::ostream &FbCompositor::operator<<(std::ostream& out, const BaseCompWindow& w) {
    out << "Window " << std::hex << w.window() << ": Geometry[" << std::dec << w.x()
        << "," << w.y() << "," << w.width() << "," << w.height() << " " << w.borderWidth()
        << "] Depth=" << w.depth() << " Type=" << w.type() << " Map=" << w.isMapped()
        << " Dmg=" << w.isDamaged() << " Show=" << w.isRenderable();
    return out;
}
