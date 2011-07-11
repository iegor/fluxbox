/** BasePlugin.cc file for the fluxbox compositor. */

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


#include "BasePlugin.hh"

#include "BaseCompWindow.hh"
#include "BaseScreen.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BasePlugin::BasePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &/*args*/) throw(InitException) :
    m_screen(screen) {

    // Here is the problem: all functions in Xlib that take a pointer to the
    // Display struct require a non-const version of it. Since screen can only
    // return a const version of it and the only other way to get it is via
    // FbTk::App (I'd rather not use that singleton), const_cast it is.
    m_display = const_cast<Display*>(screen.display());
}

// Destructor.
BasePlugin::~BasePlugin() throw() { }


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a new window is created.
void BasePlugin::windowCreated(const BaseCompWindow &/*window*/) throw(RuntimeException) { }

// Called, whenever a window is damaged.
void BasePlugin::windowDamaged(const BaseCompWindow &/*window*/) throw(RuntimeException) { }

// Called, whenever a window is destroyed.
void BasePlugin::windowDestroyed(const BaseCompWindow &/*window*/) throw(RuntimeException) { }

// Called, whenever a window is mapped.
void BasePlugin::windowMapped(const BaseCompWindow &/*window*/) throw(RuntimeException) { }

// Called, whenever window's property is changed.
void BasePlugin::windowPropertyChanged(const BaseCompWindow &/*window*/, Atom /*property*/, int /*state*/) throw(RuntimeException) { }

// Called, whenever a window is reconfigured.
void BasePlugin::windowReconfigured(const BaseCompWindow &/*window*/) throw(RuntimeException) { }

// Called, whenever window's shape changes.
void BasePlugin::windowShapeChanged(const BaseCompWindow &/*window*/) throw(RuntimeException) { }

// Called, whenever a window is unmapped.
void BasePlugin::windowUnmapped(const BaseCompWindow &/*window*/) throw(RuntimeException) { }


//--- SCREEN CHANGES -----------------------------------------------------------

// Notifies the screen of a background change.
void BasePlugin::setRootPixmapChanged() throw() { }

// Notifies the screen of a root window change.
void BasePlugin::setRootWindowSizeChanged() throw() { }
