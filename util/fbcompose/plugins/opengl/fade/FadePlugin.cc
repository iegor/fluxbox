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
FadePlugin::FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    OpenGLPlugin(screen, args) {
}

// Destructor.
FadePlugin::~FadePlugin() { }


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initialize OpenGL-specific code.
void FadePlugin::initOpenGL(GLuint shaderProgram) {
    alphaUniformPos = glGetUniformLocation(shaderProgram, "fade_Alpha");
}


//--- ACCESSORS ----------------------------------------------------------------

// Returns the additional source code for the fragment shader.
const char *FadePlugin::fragmentShader() const {
    return FRAGMENT_SHADER;
}

// Returns the additional source code for the vertex shader.
const char *FadePlugin::vertexShader() const {
    return VERTEX_SHADER;
}


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window is mapped.
void FadePlugin::windowMapped(const BaseCompWindow &window) {
    PosFadeData fade;

    // Is the window being faded out?
    std::vector<NegFadeData>::iterator it = m_negativeFades.begin();
    while (true) {
        if (it == m_negativeFades.end()) {
            fade.fadeAlpha = 0;
            break;
        } else if (it->windowId == window.window()) {
            fade.fadeAlpha = it->fadeAlpha;
            m_negativeFades.erase(it);
            break;
        } else {
            ++it;
        }
    }

    // Initialize the remaining fields.
    fade.timer.setTickSize(250000 / 255);
    fade.timer.start();

    // Track the fade.
    m_positiveFades.insert(std::make_pair(window.window(), fade));
}

// Called, whenever a window is unmapped.
void FadePlugin::windowUnmapped(const BaseCompWindow &window) {
    const OpenGLWindow &glWindow = dynamic_cast<const OpenGLWindow&>(window);
    NegFadeData fade;

    // Is the window being faded in?
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        fade.fadeAlpha = it->second.fadeAlpha;
        m_positiveFades.erase(it);
    } else {
        fade.fadeAlpha = 255;
    }

    // Initialize the remaining fields.
    fade.contentTexture = glWindow.contentTexture();
    fade.origAlpha = glWindow.alpha();
    fade.shapeTexture = glWindow.shapeTexture();
    fade.windowId = glWindow.window();
    fade.windowPosBuffer = glWindow.windowPosBuffer();

    fade.timer.setTickSize(250000 / 255);
    fade.timer.start();

    // Track the fade.
    m_negativeFades.push_back(fade);
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Pre background rendering actions.
void FadePlugin::preBackgroundRenderActions() {
    glUniform1f(alphaUniformPos, 1.0);
}

// Pre window rendering actions.
void FadePlugin::preReconfigureRectRenderActions(XRectangle /*reconfigureRect*/) {
    glUniform1f(alphaUniformPos, 1.0);
}

// Pre window rendering actions.
void FadePlugin::preWindowRenderActions(const OpenGLWindow &window) {
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        try {
            it->second.fadeAlpha += it->second.timer.newElapsedTicks();
        } catch (const TimeException &e) {
            it->second.fadeAlpha = 255;
        }
        
        if (it->second.fadeAlpha >= 255) {
            glUniform1f(alphaUniformPos, 1.0);
            m_positiveFades.erase(it);
        } else {
            glUniform1f(alphaUniformPos, (it->second.fadeAlpha / 255.0));
        }
    } else {
        glUniform1f(alphaUniformPos, 1.0);
    }
}


// Returns the number of extra rendering jobs the plugin will do.
int FadePlugin::extraRenderingJobCount() {
    return m_negativeFades.size();
}

// Initialize the specified extra rendering job.
void FadePlugin::extraRenderingJobInit(int job, GLuint &primPosBuffer_return, GLuint &mainTexCoordBuffer_return,
        GLuint &mainTexture_return, GLuint &shapeTexCoordBuffer_return, GLuint &shapeTexture_return,
        GLfloat &alpha_return) {

    NegFadeData &curFade = m_negativeFades[job];

    primPosBuffer_return = curFade.windowPosBuffer->unwrap();
    mainTexCoordBuffer_return = openGLScreen().defaultTexCoordBuffer();
    mainTexture_return = curFade.contentTexture->unwrap();
    shapeTexCoordBuffer_return = openGLScreen().defaultTexCoordBuffer();
    shapeTexture_return = curFade.shapeTexture->unwrap();
    alpha_return = curFade.origAlpha / 255.0;

    try {
        curFade.fadeAlpha -= curFade.timer.newElapsedTicks();
    } catch (const TimeException &e) {
        curFade.fadeAlpha = 0;
    }

    if (curFade.fadeAlpha <= 0) {
        glUniform1f(alphaUniformPos, 0.0);
    } else {
        glUniform1f(alphaUniformPos, (curFade.fadeAlpha / 255.0));
    }
}

// Called after the extra rendering jobs are executed.
void FadePlugin::postExtraRenderingActions() {
    std::vector<NegFadeData>::iterator it = m_negativeFades.begin();
    while (it != m_negativeFades.end()) {
        if (it->fadeAlpha <= 0) {
            it = m_negativeFades.erase(it);
        } else {
            ++it;
        }
    }
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new FadePlugin(screen, args);
}
