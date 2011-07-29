/** XRenderPlugin.cc file for the fluxbox compositor. */

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


#include "XRenderPlugin.hh"

#include "XRenderScreen.hh"
#include "XRenderWindow.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Costructor.
XRenderPlugin::XRenderPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    BasePlugin(screen, args) {
}

// Destructor.
XRenderPlugin::~XRenderPlugin() { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns a reference screen object, cast into the correct class.
const XRenderScreen &XRenderPlugin::xrenderScreen() const {
    static const XRenderScreen &s = dynamic_cast<const XRenderScreen&>(BasePlugin::screen());
    return s;
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Rectangles that the plugin wishes to damage.
std::vector<XRectangle> XRenderPlugin::damagedAreas() {
    return std::vector<XRectangle>();
}


// Post background rendering actions and jobs.
std::vector<XRenderRenderingJob> XRenderPlugin::postBackgroundRenderingActions() {
    return std::vector<XRenderRenderingJob>();
}


// Pre window rendering actions and jobs.
std::vector<XRenderRenderingJob> XRenderPlugin::preWindowRenderingActions(const XRenderWindow &/*window*/) {
    return std::vector<XRenderRenderingJob>();
}

// Window rendering job initialization.
void XRenderPlugin::windowRenderingJobInit(const XRenderWindow &/*window*/, XRenderRenderingJob &/*job*/) { }

// Post window rendering actions and jobs.
std::vector<XRenderRenderingJob> XRenderPlugin::postWindowRenderingActions(const XRenderWindow &/*window*/) {
    return std::vector<XRenderRenderingJob>();
}


// Reconfigure rectangle rendering job initialization.
void XRenderPlugin::recRectRenderingJobInit(XRectangle &/*rect_return*/, GC /*gc*/) { }


// Extra rendering actions and jobs.
std::vector<XRenderRenderingJob> XRenderPlugin::extraRenderingActions() {
    return std::vector<XRenderRenderingJob>();
}

// Post extra rendering actions.
void XRenderPlugin::postExtraRenderingActions() { }
