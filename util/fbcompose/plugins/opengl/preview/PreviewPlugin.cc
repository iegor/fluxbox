/** PreviewPlugin.cc file for the fluxbox compositor. */

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


#include "PreviewPlugin.hh"

#include "BaseScreen.hh"
#include "Exceptions.hh"
#include "OpenGLScreen.hh"
#include "OpenGLWindow.hh"
#include "Utility.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- SHADER SOURCES -----------------------------------------------------------

/** Plugin's fragment shader source. */
const GLchar FRAGMENT_SHADER[] = "\
    void preview() { }                                                       \n\
";

/** Plugin's vertex shader source. */
const GLchar VERTEX_SHADER[] = "\
    void preview() { }                                                       \n\
";


//--- CONSTANTS ----------------------------------------------------------------

/** Maximum height of the preview window. */
const int MAX_PREVIEW_HEIGHT = 150;

/** Maximum width of the preview window. */
const int MAX_PREVIEW_WIDTH = 150;

/** Transparency of the preview window. */
const unsigned int PREVIEW_ALPHA = 200;

/** Time in microseconds until the preview window is shown. */
const int SLEEP_TIME = 500000;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
PreviewPlugin::PreviewPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    OpenGLPlugin(screen, args) {

    m_tickTracker.setTickSize(SLEEP_TIME);
}

// Destructor.
PreviewPlugin::~PreviewPlugin() { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns the additional source code for the fragment shader.
const char *PreviewPlugin::fragmentShader() const {
    return FRAGMENT_SHADER;
}

// Returns the additional source code for the vertex shader.
const char *PreviewPlugin::vertexShader() const {
    return VERTEX_SHADER;
}


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a new window is created.
void PreviewPlugin::windowCreated(const BaseCompWindow &window) {
    const OpenGLWindow &glWindow = dynamic_cast<const OpenGLWindow&>(window);

    OpenGLRenderingJob job;
    job.prim_pos_buffer = new OpenGLBuffer(openGLScreen(), GL_ARRAY_BUFFER);
    job.main_tex_coord_buffer = openGLScreen().defaultTexCoordBuffer();
    job.shape_tex_coord_buffer = openGLScreen().defaultTexCoordBuffer();
    job.alpha = PREVIEW_ALPHA / 255.0f;
    job.shaderInit = new NullInitializer();
    job.shaderDeinit = new NullDeinitializer();

    PreviewWindowData winData = { glWindow, job };
    m_previewData.insert(std::make_pair(glWindow.window(), winData));
}

/** Called, whenever a window is destroyed. */
void PreviewPlugin::windowDestroyed(const BaseCompWindow &window) {
    std::map<Window, PreviewWindowData>::iterator it = m_previewData.find(window.window());
    if (it != m_previewData.end()) {
        m_previewData.erase(it);
    }
}


//--- RENDERING ACTIONS --------------------------------------------------------

/** Extra rendering actions and jobs. */
const std::vector<OpenGLRenderingJob> &PreviewPlugin::extraRenderingActions() {
    m_extraJobs.clear();

    std::map<Window, PreviewWindowData>::iterator it = m_previewData.find(screen().currentIconbarItem());
    if (it != m_previewData.end()) {
        PreviewWindowData &curPreview = it->second;

        if (!m_tickTracker.isRunning()) {
            m_tickTracker.start();
        }

        if (curPreview.window.partitionCount() > 0) {
            updatePreviewWindow(curPreview);
            if (m_tickTracker.totalElapsedTicks() > 0) {
                m_extraJobs.push_back(curPreview.job);
            }
        }
    } else {
        m_tickTracker.stop();
    }

    return m_extraJobs;
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Update the preview window.
// TODO: Place the preview window on the edge of the toolbar.
// TODO: Left/Right toolbar orientations.
// TODO: Use all window texture partitions.
void PreviewPlugin::updatePreviewWindow(PreviewWindowData &winPreview) {
    XRectangle thumbDim;

    // Find thumbnail's width and height.
    int fullThumbWidth = std::min(static_cast<int>(winPreview.window.realWidth()),
                                  openGLScreen().maxTextureSize());
    int fullThumbHeight = std::min(static_cast<int>(winPreview.window.realHeight()),
                                   openGLScreen().maxTextureSize());

    double scale_factor = 1.0;
    scale_factor = std::max(scale_factor, fullThumbWidth / double(MAX_PREVIEW_WIDTH));
    scale_factor = std::max(scale_factor, fullThumbHeight / double(MAX_PREVIEW_HEIGHT));

    thumbDim.width = static_cast<int>(fullThumbWidth / scale_factor);
    thumbDim.height = static_cast<int>(fullThumbHeight / scale_factor);

    // Find thumbnail's X and Y coordinates.
    int mousePosX, mousePosY;
    mousePointerLocation(screen(), mousePosX, mousePosY);

    if (screen().heads().size() > 0) {
        XRectangle curHead = screen().heads()[0];

        // First find which head the mouse pointer is on,
        for (size_t i = 1; i < screen().heads().size(); i++) {
            XRectangle head = screen().heads()[i];
            if ((mousePosX >= head.x) && (mousePosY >= head.y)
                    && (mousePosX < (head.x + head.width))
                    && (mousePosY < (head.y + head.height))) {
                curHead = head;
                break;
            }
        }

        // then position the thumbnail there.
        thumbDim.x = mousePosX - thumbDim.width / 2;

        int midHead = curHead.y + (curHead.height / 2);
        if (mousePosY < midHead) {
            thumbDim.y = mousePosY + 10;
        } else {
            thumbDim.y = mousePosY - thumbDim.height - 10;
        }
    } else {    // But WHAT IF we have no heads?
        thumbDim.x = mousePosX - (thumbDim.width / 2);
        thumbDim.y = mousePosY + 10;
    }

    // Update job struct variables.
    winPreview.job.prim_pos_buffer->bufferPosRectangle(screen().rootWindow().width(), screen().rootWindow().height(), thumbDim);

    winPreview.job.main_texture = winPreview.window.contentTexturePartition(0);
    winPreview.job.shape_texture = winPreview.window.shapeTexturePartition(0);
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new PreviewPlugin(screen, args);
}

// Returns the type of the plugin.
extern "C" FbCompositor::PluginType pluginType() {
    return Plugin_OpenGL;
}
