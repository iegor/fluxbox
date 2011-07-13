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

#include "OpenGLScreen.hh"
#include "OpenGLWindow.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Costructor.
OpenGLPlugin::OpenGLPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw(InitException) :
    BasePlugin(screen, args) {
}

// Destructor.
OpenGLPlugin::~OpenGLPlugin() throw() { }


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initialize OpenGL-specific code.
void OpenGLPlugin::initOpenGL(GLuint /*shaderProgram*/) throw(InitException) { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns a reference screen object, cast into the correct class.
const OpenGLScreen &OpenGLPlugin::openGLScreen() const throw() {
    static const OpenGLScreen &s = dynamic_cast<const OpenGLScreen&>(BasePlugin::screen());
    return s;
}


//--- PLUGIN ACTIONS -----------------------------------------------------------

// Pre background rendering actions.
void OpenGLPlugin::preBackgroundRenderActions() throw() { }

// Post background rendering actions.
void OpenGLPlugin::postBackgroundRenderActions() throw() { }

// Pre window rendering actions.
void OpenGLPlugin::preReconfigureRectRenderActions(XRectangle /*reconfigureRect*/) throw() { }

// Post window rendering actions.
void OpenGLPlugin::postReconfigureRectRenderActions(XRectangle /*reconfigureRect*/) throw() { }

// Pre window rendering actions.
void OpenGLPlugin::preWindowRenderActions(const OpenGLWindow &/*window*/) throw() { }

// Post window rendering actions.
void OpenGLPlugin::postWindowRenderActions(const OpenGLWindow &/*window*/) throw() { }


// Called before the extra rendering jobs are executed.
void OpenGLPlugin::preExtraRenderingActions() throw() { }

// Returns the number of extra rendering jobs the plugin will do.
int OpenGLPlugin::extraRenderingJobCount() throw() {
    return 0;
}

// Initialize the specified extra rendering job.
void OpenGLPlugin::extraRenderingJobInit(int /*job*/, GLuint &/*primPosBuffer_return*/,
        GLuint &/*mainTexCoordBuffer_return*/, GLuint &/*mainTexture_return*/, GLuint &/*shapeTexCoordBuffer_return*/,
        GLuint &/*shapeTexture_return*/, GLfloat &/*alpha_return*/) throw() { }

// Clean up after an extra rendering job.
void OpenGLPlugin::extraRenderingJobCleanup(int /*job*/) throw() { }

// Called after the extra rendering jobs are executed.
void OpenGLPlugin::postExtraRenderingActions() throw() { }
