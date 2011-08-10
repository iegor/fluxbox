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
#include "Utility.hh"
#include "XRenderScreen.hh"
#include "XRenderWindow.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
FadePlugin::FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    XRenderPlugin(screen, args) {

    m_fadePictFormat = XRenderFindStandardFormat(display(), PictStandardARGB32);
}

// Destructor.
FadePlugin::~FadePlugin() { }


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window becomes ignored.
void FadePlugin::windowBecameIgnored(const BaseCompWindow &window) {
    std::map<Window, PosFadeData>::iterator posIt = m_positiveFades.find(window.window());
    if (posIt != m_positiveFades.end()) {
        m_positiveFades.erase(posIt);
    } 

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

    std::vector<NegFadeData>::iterator it = m_negativeFades.begin();
    while (true) {
        if (it == m_negativeFades.end()) {
            fade.fadeAlpha = 0;
            fade.fadePicture = new XRenderPicture(xrenderScreen(), m_fadePictFormat, xrenderScreen().pictFilter());
            break;
        } else if (it->windowId == window.window()) {
            fade.fadeAlpha = it->fadeAlpha;
            fade.fadePicture = it->fadePicture;
            m_negativeFades.erase(it);
            break;
        } else {
            ++it;
        }
    }

    fade.dimensions = window.dimensions();
    fade.timer.setTickSize(250000 / 255);
    fade.timer.start();

    m_positiveFades.insert(std::make_pair(window.window(), fade));
}

// Called, whenever a window is unmapped.
void FadePlugin::windowUnmapped(const BaseCompWindow &window) {
    const XRenderWindow &xrWindow = dynamic_cast<const XRenderWindow&>(window);
    NegFadeData fade;

    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        fade.fadeAlpha = it->second.fadeAlpha;
        fade.fadePicture = it->second.fadePicture;
        m_positiveFades.erase(it);
    } else {
        fade.fadeAlpha = 255;
        fade.fadePicture = new XRenderPicture(xrenderScreen(), m_fadePictFormat, xrenderScreen().pictFilter());
    }

    if (xrWindow.contentPicture()->pictureHandle() != None) {
        fade.dimensions = xrWindow.dimensions();
        fade.maskPicture = xrWindow.maskPicture();
        fade.windowId = xrWindow.window();

        fade.job.operation = PictOpOver;
        fade.job.sourcePicture = xrWindow.contentPicture();
        fade.job.sourceX = 0;
        fade.job.sourceY = 0;
        fade.job.maskX = 0;
        fade.job.maskY = 0;
        fade.job.destinationX = xrWindow.x();
        fade.job.destinationY = xrWindow.y();
        fade.job.width = xrWindow.realWidth();
        fade.job.height = xrWindow.realHeight();

        fade.timer.setTickSize(250000 / 255);
        fade.timer.start();

        m_negativeFades.push_back(fade);
    }
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Rectangles that the plugin wishes to damage.
const std::vector<XRectangle> &FadePlugin::damagedAreas() {
    m_damagedAreas.clear();     // TODO: Stop recreating vector's contents.

    std::map<Window, PosFadeData>::iterator posIt = m_positiveFades.begin();
    while (posIt != m_positiveFades.end()) {
        m_damagedAreas.push_back(posIt->second.dimensions);
        ++posIt;
    }

    std::vector<NegFadeData>::iterator negIt = m_negativeFades.begin();
    while (negIt != m_negativeFades.end()) {
        m_damagedAreas.push_back(negIt->dimensions);
        ++negIt;
    }

    return m_damagedAreas;
}


// Window rendering job initialization.
void FadePlugin::windowRenderingJobInit(const XRenderWindow &window, XRenderRenderingJob &job) {
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        PosFadeData &curFade = it->second;

        int newTicks;
        try {
            newTicks = curFade.timer.newElapsedTicks();
        } catch (const TimeException &e) {
            newTicks = 255;
        }

        if ((newTicks > 0) || (curFade.fadePicture->pictureHandle() == None)) {
            curFade.fadeAlpha += newTicks;
            if (curFade.fadeAlpha > 255) {
                curFade.fadeAlpha = 255;
            }

            createFadedMask(curFade.fadeAlpha, window.maskPicture(), window.dimensions(), curFade.fadePicture);
        }

        if (curFade.fadePicture->pictureHandle() != None) {
            job.maskPicture = curFade.fadePicture;
        }
    }
}

// Extra rendering actions and jobs.
const std::vector<XRenderRenderingJob> &FadePlugin::extraRenderingActions() {
    m_extraJobs.clear();    // TODO: Stop recreating vector's contents.

    for (size_t i = 0; i < m_negativeFades.size(); i++) {
        int newTicks;
        try {
            newTicks = m_negativeFades[i].timer.newElapsedTicks();
        } catch (const TimeException &e) {
            newTicks = 255;
        }

        if ((newTicks > 0) || (m_negativeFades[i].fadePicture->pictureHandle() == None)) {
            m_negativeFades[i].fadeAlpha -= newTicks;
            if (m_negativeFades[i].fadeAlpha < 0) {
                m_negativeFades[i].fadeAlpha = 0;
            }

            createFadedMask(m_negativeFades[i].fadeAlpha, m_negativeFades[i].maskPicture,
                            m_negativeFades[i].dimensions, m_negativeFades[i].fadePicture);
        }

        if (m_negativeFades[i].fadePicture->pictureHandle() != None) {
            m_negativeFades[i].job.maskPicture = m_negativeFades[i].fadePicture;
            m_extraJobs.push_back(m_negativeFades[i].job);
        }
    }

    return m_extraJobs;
}

// Called after the extra rendering jobs are executed.
void FadePlugin::postExtraRenderingActions() {
    std::map<Window, PosFadeData>::iterator posIt = m_positiveFades.begin();
    while (posIt != m_positiveFades.end()) {
        if (posIt->second.fadeAlpha >= 255) {
            m_positiveFades.erase(posIt++);
        } else {
            ++posIt;
        }
    }

    std::vector<NegFadeData>::iterator negIt = m_negativeFades.begin();
    while (negIt != m_negativeFades.end()) {
        if (negIt->fadeAlpha <= 0) {
            negIt = m_negativeFades.erase(negIt);
        } else {
            ++negIt;
        }
    }
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Returns the faded mask picture for the given window fade.
void FadePlugin::createFadedMask(int alpha, XRenderPicturePtr mask, XRectangle dimensions,
                                 XRenderPicturePtr fadePicture_return) {
    if (mask->pictureHandle() == None) {
        return;
    }

    Pixmap fadePixmap = createSolidPixmap(screen(), dimensions.width, dimensions.height, alpha * 0x01010101);
    fadePicture_return->setPixmap(fadePixmap, true);

    XRenderComposite(display(), PictOpIn, mask->pictureHandle(), None, fadePicture_return->pictureHandle(),
                     0, 0, 0, 0, 0, 0, dimensions.width, dimensions.height);
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new FadePlugin(screen, args);
}

// Returns the type of the plugin.
extern "C" FbCompositor::PluginType pluginType() {
    return Plugin_XRender;
}
