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
FadePlugin::FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw() :
    XRenderPlugin(screen, args) {

    m_maskPictFormat = XRenderFindStandardFormat(display(), PictStandardARGB32);
}

// Destructor.
FadePlugin::~FadePlugin() throw() { }


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window is mapped.
void FadePlugin::windowMapped(const BaseCompWindow &window) throw() {
    PosFadeData fade;

    // Is the window being faded out?
    std::vector<NegFadeData>::iterator it = m_negativeFades.begin();
    while (true) {
        if (it == m_negativeFades.end()) {
            fade.fadeAlpha = 0;
            fade.fadePicture = None;
            fade.fadePixmap = None;
            break;
        } else if (it->windowId == window.window()) {
            fade.fadeAlpha = it->fadeAlpha;
            fade.fadePicture = it->fadePicture;
            fade.fadePixmap = it->fadePixmap;
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
void FadePlugin::windowUnmapped(const BaseCompWindow &window) throw() {
    const XRenderWindow &xrWindow = dynamic_cast<const XRenderWindow&>(window);
    NegFadeData fade;

    // Is the window being faded in?
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        fade.fadeAlpha = it->second.fadeAlpha;
        fade.fadePicture = it->second.fadePicture;
        fade.fadePixmap = it->second.fadePixmap;
        m_positiveFades.erase(it);
    } else {
        fade.fadeAlpha = 255;
        fade.fadePicture = None;
        fade.fadePixmap = None;
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
                                        Picture &maskPic_return) throw() {
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        PosFadeData &curFade = it->second;

        int newTicks;
        try {
            newTicks = curFade.timer.newElapsedTicks();
        } catch (const TimeException &e) {
            newTicks = 255;
        }

        if ((newTicks > 0) || (curFade.fadePicture == None)) {
            curFade.fadeAlpha += newTicks;
            if (curFade.fadeAlpha > 255) {
                curFade.fadeAlpha = 255;
            }

            createFadedMask(curFade.fadeAlpha, window.maskPicture(), window.dimensions(),
                            curFade.fadePixmap, curFade.fadePicture);
        }

        maskPic_return = curFade.fadePicture;
    }
}

// Window rendering job cleanup.
void FadePlugin::windowRenderingJobCleanup(const XRenderWindow &window) throw() {
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        if (it->second.fadeAlpha >= 255) {
            if (it->second.fadePicture) {
                XRenderFreePicture(display(), it->second.fadePicture);
            }
            if (it->second.fadePixmap) {
                XFreePixmap(display(), it->second.fadePixmap);
            }
            m_positiveFades.erase(it);
        }
    }
}


// Returns the number of extra rendering jobs the plugin will do.
int FadePlugin::extraRenderingJobCount() throw() {
    return m_negativeFades.size();
}

// Initialize the specified extra rendering job.
void FadePlugin::extraRenderingJobInit(int job, int &op_return, Picture &srcPic_return,
        int &srcX_return, int &srcY_return, Picture &maskPic_return, int &maskX_return,
        int &maskY_return, int &destX_return, int &destY_return, int &width_return,
        int &height_return) throw() {

    NegFadeData &curFade = m_negativeFades[job];

    // Set up the fade mask.
    int newTicks;
    try {
        newTicks = curFade.timer.newElapsedTicks();
    } catch (const TimeException &e) {
        newTicks = 255;
    }

    if ((newTicks > 0) || (curFade.fadePicture == None)) {
        curFade.fadeAlpha -= newTicks;
        if (curFade.fadeAlpha < 0) {
            curFade.fadeAlpha = 0;
        }

        createFadedMask(curFade.fadeAlpha, curFade.maskPicture, curFade.dimensions,
                        curFade.fadePixmap, curFade.fadePicture);
    }

    maskPic_return = curFade.fadePicture;

    // Initialize the other rendering variables.
    op_return = PictOpOver;
    srcPic_return = curFade.contentPicture->unwrap();
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
void FadePlugin::postExtraRenderingActions() throw() {
    std::vector<NegFadeData>::iterator it = m_negativeFades.begin();
    while (it != m_negativeFades.end()) {
        if (it->fadeAlpha <= 0) {
            if (it->fadePicture) {
                XRenderFreePicture(display(), it->fadePicture);
            }
            if (it->fadePixmap) {
                XFreePixmap(display(), it->fadePixmap);
            }
            it = m_negativeFades.erase(it);
        } else {
            ++it;
        }
    }
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Returns the faded mask picture for the given window fade.
void FadePlugin::createFadedMask(int alpha, XRenderPictureWrapperPtr mask, XRectangle dimensions,
                                 Pixmap &fadePixmap_return, Picture &fadePicture_return) throw() {
    if (fadePixmap_return) {
        XFreePixmap(display(), fadePixmap_return);
        fadePixmap_return = None;
    }
    fadePixmap_return = createSolidPixmap(display(), screen().rootWindow().window(),
                                          dimensions.width, dimensions.height, alpha * 0x01010101);

    if (fadePicture_return) {
        XRenderFreePicture(display(), fadePicture_return);
        fadePicture_return = None;
    }
    fadePicture_return = XRenderCreatePicture(display(), fadePixmap_return, m_maskPictFormat, 0, NULL);

    XRenderComposite(display(), PictOpIn, mask->unwrap(), None, fadePicture_return,
                     0, 0, 0, 0, 0, 0, dimensions.width, dimensions.height);
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new FadePlugin(screen, args);
}
