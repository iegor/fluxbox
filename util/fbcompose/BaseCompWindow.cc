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

#include <ostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseCompWindow::BaseCompWindow(Window windowXID) :
    FbTk::FbWindow(windowXID) {

    XWindowAttributes xwa;
    XGetWindowAttributes(display(), window(), &xwa);

    m_class = xwa.c_class;
    m_isMapped = (xwa.map_state != IsUnmapped);

    if (m_class == InputOutput) {
        m_damage = XDamageCreate(display(), window(), XDamageReportNonEmpty);
    } else {
        m_damage = 0;
    }
    m_isDamaged = false;
}

// Destructor.
BaseCompWindow::~BaseCompWindow() {
    if (!m_damage) {
        XDamageDestroy(display(), m_damage);
    }
}


//--- WINDOW MANIPULATION ----------------------------------------------

// Marks the window as damaged.
// TODO: Do we need anything more sophisticated than this?
void BaseCompWindow::setDamaged() throw() {
    if (!m_damage) return;
    m_isDamaged = true;
}

// Marks the window as mapped.
void BaseCompWindow::setMapped() throw() {
    m_isMapped = true;
}

// Marks the window as unmapped.
void BaseCompWindow::setUnmapped() throw() {
    m_isMapped = false;
}


//--- OPERATORS ----------------------------------------------------------------

// << output stream operator for the window class.
std::ostream &FbCompositor::operator<<(std::ostream& out, const BaseCompWindow& w) {
    out << "Window " << w.window() << ": Geometry[" << w.x() << "," << w.y()
        << "," << w.width() << "," << w.height() << " " << w.borderWidth()
        << "] Depth[" << w.depth() << "] " << w.isMapped() << " " << w.isDamaged();
    return out;
}
