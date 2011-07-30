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
#include "Exceptions.hh"
#include "OpenGLScreen.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- SHADER SOURCES -----------------------------------------------------------

/** Plugin's fragment shader source. */
const GLchar FRAGMENT_SHADER[] = "\
    uniform float fade_Alpha;                                                \n\
                                                                             \n\
    void fade() {                                                            \n\
        gl_FragColor *= vec4(1.0, 1.0, 1.0, fade_Alpha);                     \n\
    }                                                                        \n\
";

/** Plugin's vertex shader source. */
const GLchar VERTEX_SHADER[] = "\
    void fade() { }                                                          \n\
";


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
FadePlugin::FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    OpenGLPlugin(screen, args) {
}

// Destructor.
FadePlugin::~FadePlugin() { }


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initialize OpenGL-specific code.
void FadePlugin::initOpenGL(OpenGLShaderProgramPtr shaderProgram) {
    m_alphaUniformPos = shaderProgram->getUniformLocation("fade_Alpha");
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

// Called, whenever a window becomes ignored.
void FadePlugin::windowBecameIgnored(const BaseCompWindow &window) {
    // Remove the window's positive fade, if any.
    std::map<Window, PosFadeData>::iterator posIt = m_positiveFades.find(window.window());
    if (posIt != m_positiveFades.end()) {
        m_positiveFades.erase(posIt);
    } 

    // Remove the window's negative fade, if any.
    std::vector<NegFadeData>::iterator negIt = m_negativeFades.begin();
    while (negIt != m_negativeFades.end()) {
        if (negIt->windowId == window.window()) {
            m_negativeFades.erase(negIt);
            break;
        } 
        ++negIt;
    }
}

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
    float fadeAlpha = 255;
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        fadeAlpha = it->second.fadeAlpha;
        m_positiveFades.erase(it);
    }

    // Create a fade for each window partition.
    for (int i = 0; i < glWindow.partitionCount(); i++) {
        NegFadeData fade;

        fade.contentTexture = glWindow.contentTexturePartition(i);
        fade.fadeAlpha = fadeAlpha;
        fade.origAlpha = glWindow.alpha();
        fade.shapeTexture = glWindow.shapeTexturePartition(i);
        fade.windowId = glWindow.window();
        fade.windowPosBuffer = glWindow.partitionPosBuffer(i);

        fade.timer.setTickSize(250000 / 255);
        fade.timer.start();

        m_negativeFades.push_back(fade);
    }
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Background rendering initialization.
void FadePlugin::backgroundRenderInit(int /*partId*/) {
    glUniform1f(m_alphaUniformPos, 1.0);
}

// Window rendering initialization.
void FadePlugin::windowRenderInit(const OpenGLWindow &window, int /*partId*/) {
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        try {
            it->second.fadeAlpha += it->second.timer.newElapsedTicks();
        } catch (const TimeException &e) {
            it->second.fadeAlpha = 255;
        }
        
        if (it->second.fadeAlpha >= 255) {
            glUniform1f(m_alphaUniformPos, 1.0);
            m_positiveFades.erase(it);
        } else {
            glUniform1f(m_alphaUniformPos, (it->second.fadeAlpha / 255.0));
        }
    } else {
        glUniform1f(m_alphaUniformPos, 1.0);
    }
}

// Reconfigure rectangle rendering initialization.
void FadePlugin::recRectRenderInit(XRectangle /*recRect*/) {
    glUniform1f(m_alphaUniformPos, 1.0);
}


// Extra rendering actions and jobs.
std::vector<OpenGLRenderingJob> FadePlugin::extraRenderingActions() {
    std::vector<OpenGLRenderingJob> jobs;

    for (size_t i = 0; i < m_negativeFades.size(); i++) {
        try {
            m_negativeFades[i].fadeAlpha -= m_negativeFades[i].timer.newElapsedTicks();
        } catch (const TimeException &e) {
            m_negativeFades[i].fadeAlpha = 0;
        }

        if (m_negativeFades[i].fadeAlpha <= 0) {
            m_negativeFades[i].fadeAlpha = 0;
        }

        OpenGLRenderingJob job;
        job.primPosBuffer = m_negativeFades[i].windowPosBuffer;
        job.mainTexCoordBuffer = openGLScreen().defaultTexCoordBuffer();
        job.mainTexture = m_negativeFades[i].contentTexture;
        job.shapeTexCoordBuffer = openGLScreen().defaultTexCoordBuffer();
        job.shapeTexture = m_negativeFades[i].shapeTexture;
        job.alpha = m_negativeFades[i].origAlpha / 255.0;
        job.initAction = new FadeInitAction(m_alphaUniformPos, m_negativeFades[i].fadeAlpha / 255.0);
        job.cleanupAction = new NullCleanupAction();
        jobs.push_back(job);
    }

    return jobs;
}

// Post extra rendering actions.
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


// Null rendering job initialization.
void FadePlugin::nullRenderInit() {
    glUniform1f(m_alphaUniformPos, 1.0);
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new FadePlugin(screen, args);
}

// Returns the type of the plugin.
extern "C" FbCompositor::PluginType pluginType() {
    return Plugin_OpenGL;
}
