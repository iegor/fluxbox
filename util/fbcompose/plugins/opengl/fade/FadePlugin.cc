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

#include <iostream>

using namespace FbCompositor;


namespace {
    //--- SHADER SOURCES -------------------------------------------------------

    /** Plugin's fragment shader source. */
    static const GLchar FRAGMENT_SHADER[] = "\
        uniform float fade_Alpha;                                            \n\
                                                                             \n\
        void fade() {                                                        \n\
            gl_FragColor *= vec4(0.9, 0.7, 0.5, 1.0);                        \n\
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

// Pre-rendering actions (uniform setup etc).
void FadePlugin::preRenderActions(GLuint shaderProgram) throw() {
    static GLuint alphaPos = glGetUniformLocation(shaderProgram, "fade_Alpha");
    glUniform1f(alphaPos, 0.1);
}

// Post-rendering actions (plugin-specific cleanup etc).
void FadePlugin::postRenderActions(GLuint /*shaderProgram*/) throw() { }


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window is mapped.
void FadePlugin::windowMapped(const BaseCompWindow &window) {
    std::cout << window.window() << " mapped." << std::endl;
}

// Called, whenever a window is unmapped.
void FadePlugin::windowUnmapped(const BaseCompWindow &window) {
    std::cout << window.window() << " unmapped." << std::endl;
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
