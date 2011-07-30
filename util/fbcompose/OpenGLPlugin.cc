/** OpenGLPlugin.cc file for the fluxbox compositor. */

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


#include "OpenGLPlugin.hh"

#include "Exceptions.hh"
#include "OpenGLScreen.hh"
#include "OpenGLWindow.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Costructor.
OpenGLPlugin::OpenGLPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    BasePlugin(screen, args) {
}

// Destructor.
OpenGLPlugin::~OpenGLPlugin() { }


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initialize OpenGL-specific code.
void OpenGLPlugin::initOpenGL(OpenGLShaderProgramPtr /*shaderProgram*/) { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns a reference screen object, cast into the correct class.
const OpenGLScreen &OpenGLPlugin::openGLScreen() const {
    static const OpenGLScreen &s = dynamic_cast<const OpenGLScreen&>(BasePlugin::screen());
    return s;
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Background rendering initialization.
void OpenGLPlugin::backgroundRenderInit(int /*partId*/) { }

// Background rendering cleanup.
void OpenGLPlugin::backgroundRenderCleanup(int /*partId*/) { }

// Post background rendering actions.
std::vector<OpenGLRenderingJob> OpenGLPlugin::postBackgroundRenderActions() {
    return std::vector<OpenGLRenderingJob>();
}


// Pre window rendering actions and jobs.
std::vector<OpenGLRenderingJob> OpenGLPlugin::preWindowRenderActions(const OpenGLWindow &/*window*/) {
    return std::vector<OpenGLRenderingJob>();
}

// Window rendering initialization.
void OpenGLPlugin::windowRenderInit(const OpenGLWindow &/*window*/, int /*partId*/) { }

// Window rendering cleanup.
void OpenGLPlugin::windowRenderCleanup(const OpenGLWindow &/*window*/, int /*partId*/) { }

// Post window rendering actions and jobs.
std::vector<OpenGLRenderingJob> OpenGLPlugin::postWindowRenderActions(const OpenGLWindow &/*window*/) {
    return std::vector<OpenGLRenderingJob>();
}


// Reconfigure rectangle rendering initialization.
void OpenGLPlugin::recRectRenderInit(XRectangle /*recRect*/) { }

// Reconfigure rectangle rendering cleanup.
void OpenGLPlugin::recRectRenderCleanup(XRectangle /*recRect*/) { }


// Extra rendering actions and jobs.
std::vector<OpenGLRenderingJob> OpenGLPlugin::extraRenderingActions() {
    return std::vector<OpenGLRenderingJob>();
}

// Post extra rendering actions.
void OpenGLPlugin::postExtraRenderingActions() { }


// Null rendering job initialization.
void OpenGLPlugin::nullRenderInit() { }
