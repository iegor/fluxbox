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
XRenderPlugin::XRenderPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw(InitException) :
    BasePlugin(screen, args) {
}

// Destructor.
XRenderPlugin::~XRenderPlugin() throw() { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns a reference screen object, cast into the correct class.
const XRenderScreen &XRenderPlugin::xrenderScreen() const throw() {
    static const XRenderScreen &s = dynamic_cast<const XRenderScreen&>(BasePlugin::screen());
    return s;
}


//--- PLUGIN ACTIONS -----------------------------------------------------------

// Extra background rendering job.
void XRenderPlugin::extraBackgroundRenderingJob(int &op_return, Picture &/*srcPic_return*/,
        int &/*srcX_return*/, int &/*srcY_return*/, Picture &/*maskPic_return*/, int &/*maskX_return*/,
        int &/*maskY_return*/, int &/*destX_return*/, int &/*destY_return*/, int &/*width_return*/,
        int &/*height_return*/) throw() {
    op_return = PictOpClear;
}

// Reconfigure rectangle rendering options.
void XRenderPlugin::reconfigureRectRenderActions(XRectangle &/*rect_return*/, GC /*gc*/) throw() { }


// Extra pre window rendering job.
void XRenderPlugin::extraPreWindowRenderingJob(const XRenderWindow &/*window*/, int &op_return,
        Picture &/*srcPic_return*/, int &/*srcX_return*/, int &/*srcY_return*/, Picture &/*maskPic_return*/,
        int &/*maskX_return*/, int &/*maskY_return*/, int &/*destX_return*/, int &/*destY_return*/,
        int &/*width_return*/, int &/*height_return*/) throw() {
    op_return = PictOpClear;
}

// Window rendering options.
void XRenderPlugin::windowRenderingJobInit(const XRenderWindow &/*window*/, int &/*op_return*/,
                                           Picture &/*maskPic_return*/) throw() { }

// Window rendering job cleanup.
void XRenderPlugin::windowRenderingJobCleanup(const XRenderWindow &/*window*/) throw() { }

// Extra post window rendering job.
void XRenderPlugin::extraPostWindowRenderingJob(const XRenderWindow &/*window*/, int &op_return,
        Picture &/*srcPic_return*/, int &/*srcX_return*/, int &/*srcY_return*/, Picture &/*maskPic_return*/,
        int &/*maskX_return*/, int &/*maskY_return*/, int &/*destX_return*/, int &/*destY_return*/,
        int &/*width_return*/, int &/*height_return*/) throw() {
    op_return = PictOpClear;
}


// Called before the extra rendering jobs are executed.
void XRenderPlugin::preExtraRenderingActions() throw() { }

// Returns the number of extra rendering jobs the plugin will do.
int XRenderPlugin::extraRenderingJobCount() throw() {
    return 0;
}

// Initialize the specified extra rendering job.
void XRenderPlugin::extraRenderingJobInit(int /*job*/, int &op_return, Picture &/*srcPic_return*/,
        int &/*srcX_return*/, int &/*srcY_return*/, Picture &/*maskPic_return*/, int &/*maskX_return*/,
        int &/*maskY_return*/, int &/*destX_return*/, int &/*destY_return*/, int &/*width_return*/,
        int &/*height_return*/) throw() {
    op_return = PictOpClear;
}

// Clean up after an extra rendering job.
void XRenderPlugin::extraRenderingJobCleanup(int /*job*/) throw() { }

// Called after the extra rendering jobs are executed.
void XRenderPlugin::postExtraRenderingActions() throw() { }
