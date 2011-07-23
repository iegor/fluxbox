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

    // Initialize the remaining fields.
    fade.timer.setTickSize(250000 / 255);
    fade.timer.start();

    // Track the fade.
    m_positiveFades.insert(std::make_pair(window.window(), fade));
}

// Called, whenever a window is unmapped.
void FadePlugin::windowUnmapped(const BaseCompWindow &window) {
    const XRenderWindow &xrWindow = dynamic_cast<const XRenderWindow&>(window);
    NegFadeData fade;

    // Is the window being faded in?
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        fade.fadeAlpha = it->second.fadeAlpha;
        fade.fadePicture = it->second.fadePicture;
        m_positiveFades.erase(it);
    } else {
        fade.fadeAlpha = 255;
        fade.fadePicture = new XRenderPicture(xrenderScreen(), m_fadePictFormat, xrenderScreen().pictFilter());
    }

    // Initialize the remaining fields.
    fade.contentPicture = xrWindow.contentPicture();
    fade.dimensions = xrWindow.dimensions();
    fade.maskPicture = xrWindow.maskPicture();
    fade.origAlpha = xrWindow.alpha();
    fade.windowId = xrWindow.window();

    fade.timer.setTickSize(250000 / 255);
    fade.timer.start();

    // Track the fade.
    m_negativeFades.push_back(fade);
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Window rendering job initialization.
void FadePlugin::windowRenderingJobInit(const XRenderWindow &window, int &/*op_return*/,
                                        Picture &maskPic_return) {
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

        maskPic_return = curFade.fadePicture->pictureHandle();
    }
}

// Window rendering job cleanup.
void FadePlugin::windowRenderingJobCleanup(const XRenderWindow &window) {
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        if (it->second.fadeAlpha >= 255) {
            m_positiveFades.erase(it);
        }
    }
}


// Returns the number of extra rendering jobs the plugin will do.
int FadePlugin::extraRenderingJobCount() {
    return m_negativeFades.size();
}

// Initialize the specified extra rendering job.
void FadePlugin::extraRenderingJobInit(int job, int &op_return, Picture &srcPic_return,
        int &srcX_return, int &srcY_return, Picture &maskPic_return, int &maskX_return,
        int &maskY_return, int &destX_return, int &destY_return, int &width_return,
        int &height_return) {

    NegFadeData &curFade = m_negativeFades[job];

    // Set up the fade mask.
    int newTicks;
    try {
        newTicks = curFade.timer.newElapsedTicks();
    } catch (const TimeException &e) {
        newTicks = 255;
    }

    if ((newTicks > 0) || (curFade.fadePicture->pictureHandle() == None)) {
        curFade.fadeAlpha -= newTicks;
        if (curFade.fadeAlpha < 0) {
            curFade.fadeAlpha = 0;
        }

        createFadedMask(curFade.fadeAlpha, curFade.maskPicture, curFade.dimensions, curFade.fadePicture);
    }

    maskPic_return = curFade.fadePicture->pictureHandle();

    // Initialize the other rendering variables.
    op_return = PictOpOver;
    srcPic_return = curFade.contentPicture->pictureHandle();
    srcX_return = 0;
    srcY_return = 0;
    maskX_return = 0;
    maskY_return = 0;
    destX_return = curFade.dimensions.x;
    destY_return = curFade.dimensions.y;
    width_return = curFade.dimensions.width;
    height_return = curFade.dimensions.height;
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


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Returns the faded mask picture for the given window fade.
void FadePlugin::createFadedMask(int alpha, XRenderPicturePtr mask, XRectangle dimensions,
                                 XRenderPicturePtr &fadePicture_return) {
    Pixmap fadePixmap = createSolidPixmap(display(), screen().rootWindow().window(),
                                          dimensions.width, dimensions.height, alpha * 0x01010101);
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
