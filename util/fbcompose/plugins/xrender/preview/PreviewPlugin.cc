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

    XRenderPictFormat *pictFormat = XRenderFindStandardFormat(display(), PictStandardARGB32);
    XRenderPicturePtr thumbnail(new XRenderPicture(xrenderScreen(), pictFormat, FilterBest));

    Pixmap thumbPixmap = createSolidPixmap(display(), window.window(), MAX_PREVIEW_WIDTH, MAX_PREVIEW_HEIGHT);
    thumbnail->setPixmap(thumbPixmap, true);

    XRenderRenderingJob job = { PictOpOver, thumbnail, m_maskPicture, 0, 0, 0, 0, 0, 0, 0, 0 };

    PreviewWindowData winData = { xrWindow, job };
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

        if ((m_previousWindow != curWindow)
                && (curPreview.window.contentPicture()->pictureHandle())
                && (curPreview.window.maskPicture()->pictureHandle())) {
            m_previousWindow = curWindow;
            updatePreviewData(curPreview);
        }

        int mousePosX, mousePosY;
        mousePointerLocation(screen(), mousePosX, mousePosY);
        curPreview.job.destinationX = mousePosX;
        curPreview.job.destinationY = mousePosY;

        XRectangle curDamage = { curPreview.job.destinationX, curPreview.job.destinationY,
                                 curPreview.job.width, curPreview.job.height };
        m_damagedAreas.push_back(curDamage);
        m_previousDamage = curDamage;

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

        if ((curPreview.job.sourcePicture->pictureHandle()) && (m_tickTracker.totalElapsedTicks() > 0)) {
            m_extraJobs.push_back(curPreview.job);
        }
    }

    return m_extraJobs;
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Update the preview window data.
void PreviewPlugin::updatePreviewData(PreviewWindowData &data) {
    double scaleFactor = 1.0;
    scaleFactor = std::max(scaleFactor, data.window.realWidth() / double(MAX_PREVIEW_WIDTH));
    scaleFactor = std::max(scaleFactor, data.window.realHeight() / double(MAX_PREVIEW_HEIGHT));

    int thumbWidth = static_cast<int>(data.window.realWidth() * scaleFactor);
    int thumbHeight = static_cast<int>(data.window.realHeight() * scaleFactor);

    data.window.contentPicture()->scalePicture(scaleFactor, scaleFactor);
    data.window.maskPicture()->scalePicture(scaleFactor, scaleFactor);

    XRenderComposite(display(), PictOpSrc,
                     data.window.contentPicture()->pictureHandle(), 
                     data.window.maskPicture()->pictureHandle(),
                     data.job.sourcePicture->pictureHandle(),
                     0, 0, 0, 0, 0, 0, thumbWidth, thumbHeight);

    data.window.contentPicture()->resetPictureTransform();
    data.window.maskPicture()->resetPictureTransform();

    data.job.width = thumbWidth;
    data.job.height = thumbHeight;
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
