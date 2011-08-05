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
#include "Utility.hh"
#include "XRenderScreen.hh"
#include "XRenderWindow.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


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
    XRenderPlugin(screen, args) {

    unsigned long maskColor = 0x01010101 * PREVIEW_ALPHA;
    Pixmap maskPixmap = createSolidPixmap(display(), screen.rootWindow().window(), MAX_PREVIEW_WIDTH, MAX_PREVIEW_HEIGHT, maskColor);
    XRenderPictFormat *pictFormat = XRenderFindStandardFormat(display(), PictStandardARGB32);
    m_maskPicture = new XRenderPicture(xrenderScreen(), pictFormat, FilterFast);
    m_maskPicture->setPixmap(maskPixmap, true);

    m_previousDamage.width = 0;
    m_previousDamage.height = 0;
    m_previousWindow = None;

    m_tickTracker.setTickSize(SLEEP_TIME);
}

// Destructor.
PreviewPlugin::~PreviewPlugin() { }



//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a new window is created.
void PreviewPlugin::windowCreated(const BaseCompWindow &window) {
    const XRenderWindow &xrWindow = dynamic_cast<const XRenderWindow&>(window);

    XRenderPictFormat *pictFormat = XRenderFindVisualFormat(display(), xrWindow.visual());
    PreviewWindowData winData = { xrWindow,
                                  XRenderPicturePtr(new XRenderPicture(xrenderScreen(), pictFormat, FilterBest)),
                                  XRectangle() };
    m_previewData.insert(std::make_pair(xrWindow.window(), winData));
}

/** Called, whenever a window is destroyed. */
void PreviewPlugin::windowDestroyed(const BaseCompWindow &window) {
    std::map<Window, PreviewWindowData>::iterator it = m_previewData.find(window.window());
    if (it != m_previewData.end()) {
        m_previewData.erase(it);
    }
}


//--- RENDERING ACTIONS --------------------------------------------------------

/** Rectangles that the plugin wishes to damage. */
const std::vector<XRectangle> &PreviewPlugin::damagedAreas() {
    m_damagedAreas.clear();

    if ((m_previousDamage.width > 0) && (m_previousDamage.height > 0)) {
        m_damagedAreas.push_back(m_previousDamage);
    }

    std::map<Window, PreviewWindowData>::iterator it = m_previewData.find(screen().currentIconbarItem());
    if (it != m_previewData.end()) {
        Window curWindow = it->first;
        PreviewWindowData &curPreview = it->second;

        if ((m_previousWindow != curWindow) && (curPreview.window.contentPicture()->pictureHandle())) {
            m_previousWindow = curWindow;

            curPreview.previewPicture->setPixmap(curPreview.window.contentPixmap(), false);

            double scaleFactor = 1.0;
            scaleFactor = std::max(scaleFactor, curPreview.window.realWidth() / double(MAX_PREVIEW_WIDTH));
            scaleFactor = std::max(scaleFactor, curPreview.window.realHeight() / double(MAX_PREVIEW_HEIGHT));
            curPreview.previewPicture->scalePicture(scaleFactor, scaleFactor);

            curPreview.dimensions.width = static_cast<int>(curPreview.window.realWidth() * scaleFactor);
            curPreview.dimensions.height = static_cast<int>(curPreview.window.realHeight() * scaleFactor);
        }

        int mousePosX, mousePosY;
        mousePointerLocation(screen(), mousePosX, mousePosY);
        curPreview.dimensions.x = mousePosX;
        curPreview.dimensions.y = mousePosY;

        m_damagedAreas.push_back(curPreview.dimensions);
        m_previousDamage = curPreview.dimensions;
        if (!m_tickTracker.isRunning()) {
            m_tickTracker.start();
        }
    } else {
        m_previousDamage.width = 0;
        m_previousDamage.height = 0;
        m_previousWindow = None;
        m_tickTracker.stop();
    }

    return m_damagedAreas;
}

/** Extra rendering actions and jobs. */
const std::vector<XRenderRenderingJob> &PreviewPlugin::extraRenderingActions() {
    m_extraJobs.clear();

    std::map<Window, PreviewWindowData>::iterator it = m_previewData.find(xrenderScreen().currentIconbarItem());
    if (it != m_previewData.end()) {
        PreviewWindowData &curPreview = it->second;

        if ((curPreview.previewPicture->pictureHandle()) && (m_tickTracker.totalElapsedTicks() > 0)) {
            XRenderRenderingJob job;
            job.operation = PictOpOver;
            job.sourcePicture = curPreview.previewPicture;
            job.maskPicture = m_maskPicture;
            job.sourceX = 0;
            job.sourceY = 0;
            job.maskX = 0;
            job.maskY = 0;
            job.destinationX = curPreview.dimensions.x;
            job.destinationY = curPreview.dimensions.y;
            job.width = curPreview.dimensions.width;
            job.height = curPreview.dimensions.height;
            m_extraJobs.push_back(job);
        }
    }

    return m_extraJobs;
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new PreviewPlugin(screen, args);
}

// Returns the type of the plugin.
extern "C" FbCompositor::PluginType pluginType() {
    return Plugin_XRender;
}
