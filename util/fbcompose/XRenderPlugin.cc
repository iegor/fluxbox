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


//--- CONSTANTS ----------------------------------------------------------------

namespace {
    /** Null rendering job. */
    static const XRenderRenderingJob NULL_JOB = {
            PictOpClear, XRenderPicturePtr(), XRenderPicturePtr(), 0, 0, 0, 0, 0, 0, 0, 0
    };
}


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


//--- PLUGIN ACTIONS -----------------------------------------------------------

// Extra background rendering job.
XRenderRenderingJob XRenderPlugin::extraBackgroundRenderingJob() {
    return NULL_JOB;
}

// Reconfigure rectangle rendering options.
void XRenderPlugin::reconfigureRectRenderActions(XRectangle &/*rect_return*/, GC /*gc*/) { }


// Extra pre window rendering job.
XRenderRenderingJob XRenderPlugin::extraPreWindowRenderingJob(const XRenderWindow &/*window*/) {
    return NULL_JOB;
}

// Window rendering options.
void XRenderPlugin::windowRenderingJobInit(const XRenderWindow &/*window*/, XRenderRenderingJob &/*job*/) { }

// Window rendering job cleanup.
void XRenderPlugin::windowRenderingJobCleanup(const XRenderWindow &/*window*/) { }

// Extra post window rendering job.
XRenderRenderingJob XRenderPlugin::extraPostWindowRenderingJob(const XRenderWindow &/*window*/) {
    return NULL_JOB;
}


// Called before the extra rendering jobs are executed.
void XRenderPlugin::preExtraRenderingActions() { }

// Returns the number of extra rendering jobs the plugin will do.
int XRenderPlugin::extraRenderingJobCount() {
    return 0;
}

// Initialize the specified extra rendering job.
XRenderRenderingJob XRenderPlugin::extraRenderingJobInit(int /*jobId*/) {
    return NULL_JOB;
}

// Clean up after an extra rendering job.
void XRenderPlugin::extraRenderingJobCleanup(int /*jobId*/) { }

// Called after the extra rendering jobs are executed.
void XRenderPlugin::postExtraRenderingActions() { }
