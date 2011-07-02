/** FadePlugin.cc file for the fluxbox compositor. */

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


#include "FadePlugin.hh"

#include "BaseScreen.hh"
#include "OpenGLScreen.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


namespace {
    //--- SHADER SOURCES -------------------------------------------------------

    /** Plugin's fragment shader source. */
    static const GLchar FRAGMENT_SHADER[] = "\
        uniform float fade_Alpha;                                            \n\
                                                                             \n\
        void fade() {                                                        \n\
            gl_FragColor *= vec4(1.0, 1.0, 1.0, fade_Alpha);                 \n\
        }                                                                    \n\
    ";

    /** Plugin's vertex shader source. */
    static const GLchar VERTEX_SHADER[] = "\
        void fade() { }                                                      \n\
    ";
}


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
FadePlugin::FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw(InitException) :
    OpenGLPlugin(screen, args) {
}

// Destructor.
FadePlugin::~FadePlugin() throw() { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns the additional source code for the fragment shader.
const char *FadePlugin::fragmentShader() const throw() {
    return FRAGMENT_SHADER;
}

// Returns the additional source code for the vertex shader.
const char *FadePlugin::vertexShader() const throw() {
    return VERTEX_SHADER;
}


//--- PLUGIN ACTIONS -----------------------------------------------------------

// Pre background rendering actions.
void FadePlugin::preBackgroundRenderActions() {
    static GLuint alphaPos = glGetUniformLocation(dynamic_cast<const OpenGLScreen&>(screen()).shaderProgram(), "fade_Alpha");
    glUniform1f(alphaPos, 1.0);
}

// Pre window rendering actions.
void FadePlugin::preReconfigureRectRenderActions(XRectangle /*reconfigureRect*/) {
    static GLuint alphaPos = glGetUniformLocation(dynamic_cast<const OpenGLScreen&>(screen()).shaderProgram(), "fade_Alpha");
    glUniform1f(alphaPos, 1.0);
}

// Pre window rendering actions.
void FadePlugin::preWindowRenderActions(const OpenGLWindow &window) {
    static GLuint alphaPos = glGetUniformLocation(dynamic_cast<const OpenGLScreen&>(screen()).shaderProgram(), "fade_Alpha");

    std::map<Window, FadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        it->second.alpha += it->second.timer.newElapsedTicks();
        
        if (it->second.alpha >= 255) {
            glUniform1f(alphaPos, 1.0);
            m_positiveFades.erase(it);
        } else {
            glUniform1f(alphaPos, (it->second.alpha / 255.0));
        }
    } else {
        glUniform1f(alphaPos, 1.0);
    }
}


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window is mapped.
void FadePlugin::windowMapped(const BaseCompWindow &window) {
    FadeData fade;
    fade.alpha = 0;
    fade.timer.setTickSize(500000 / 255);
    fade.timer.start();

    m_positiveFades.insert(std::make_pair(window.window(), fade));
}

// Called, whenever a window is unmapped.
void FadePlugin::windowUnmapped(const BaseCompWindow &window) {
    m_positiveFades.erase(window.window());
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new FadePlugin(screen, args);
}

// Returns plugin's type.
extern "C" PluginType pluginType() {
    return Plugin_OpenGL;
}
