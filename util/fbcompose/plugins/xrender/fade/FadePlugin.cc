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

#ifdef USE_XRENDER_COMPOSITING


#include "BaseScreen.hh"
#include "Utility.hh"
#include "XRenderScreen.hh"
#include "XRenderWindow.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
FadePlugin::FadePlugin(Display *display, const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw() :
    XRenderPlugin(screen, args),
    m_display(display),
    m_maskPictFormat(XRenderFindStandardFormat(m_display, PictStandardARGB32)) {
}

// Destructor.
FadePlugin::~FadePlugin() throw() { }


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window is mapped.
void FadePlugin::windowMapped(const BaseCompWindow &window) throw() {
    PosFadeData fade;
    fade.fadeAlpha = 0;
    fade.mask = None;
    fade.timer.setTickSize(250000 / 255);
    fade.timer.start();

    m_positiveFades.insert(std::make_pair(window.window(), fade));
}

// Called, whenever a window is unmapped.
void FadePlugin::windowUnmapped(const BaseCompWindow &window) throw() {
    const XRenderWindow &xrWindow = dynamic_cast<const XRenderWindow&>(window);
    NegFadeData fade;

    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        fade.fadeAlpha = it->second.fadeAlpha;
        m_positiveFades.erase(it);
    } else {
        fade.fadeAlpha = 255;
    }

    fade.origAlpha = xrWindow.alpha();
    fade.contentPictureHolder = xrWindow.contentPicture();
    fade.maskPictureHolder = xrWindow.maskPicture();
    fade.mask = None;
    fade.width = xrWindow.realWidth();
    fade.height = xrWindow.realHeight();
    fade.xCoord = xrWindow.x();
    fade.yCoord = xrWindow.y();

    fade.timer.setTickSize(250000 / 255);
    fade.timer.start();

    m_negativeFades.push_back(fade);
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Window rendering job initialization.
void FadePlugin::windowRenderingJobInit(const XRenderWindow &window, int &/*op_return*/,
                                        Picture &maskPic_return) throw(RuntimeException) {
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        it->second.fadeAlpha += it->second.timer.newElapsedTicks();
        if (it->second.fadeAlpha > 255) {
            it->second.fadeAlpha = 255;
        }

        Pixmap newMaskPixmap = createSolidPixmap(m_display, screen().rootWindow().window(),
                                                 window.realWidth(), window.realHeight(), it->second.fadeAlpha * 0x01010101);

        if (it->second.mask) {
            XRenderFreePicture(m_display, it->second.mask);
            it->second.mask = None;
        }
        it->second.mask = XRenderCreatePicture(m_display, newMaskPixmap, m_maskPictFormat, 0, NULL);

        XRenderComposite(m_display, PictOpDst, window.maskPicture()->picture(),
                         None, it->second.mask, 0, 0, 0, 0, 0, 0, window.realWidth(), window.realHeight());

        maskPic_return = it->second.mask;
    } 
}

// Window rendering job cleanup.
void FadePlugin::windowRenderingJobCleanup(const XRenderWindow &window) throw(RuntimeException) {
    std::map<Window, PosFadeData>::iterator it = m_positiveFades.find(window.window());
    if (it != m_positiveFades.end()) {
        if (it->second.fadeAlpha >= 255) {
            if (it->second.mask) {
                XRenderFreePicture(m_display, it->second.mask);
            }
            m_positiveFades.erase(it);
        }
    }
}


// \returns the number of extra rendering jobs the plugin will do.
int FadePlugin::extraRenderingJobCount() throw(RuntimeException) {
    return m_negativeFades.size();
}

// Initialize the specified extra rendering job.
void FadePlugin::extraRenderingJobInit(int job, int &op_return, Picture &srcPic_return,
        int &srcX_return, int &srcY_return, Picture &maskPic_return, int &maskX_return,
        int &maskY_return, int &destX_return, int &destY_return, int &width_return,
        int &height_return) throw(RuntimeException) {

    op_return = PictOpOver;
    srcPic_return = m_negativeFades[job].contentPictureHolder->picture();
    srcX_return = 0;
    srcY_return = 0;
    maskX_return = 0;
    maskY_return = 0;
    destX_return = m_negativeFades[job].xCoord;
    destY_return = m_negativeFades[job].yCoord;
    width_return = m_negativeFades[job].width;
    height_return = m_negativeFades[job].height;

    m_negativeFades[job].fadeAlpha -= m_negativeFades[job].timer.newElapsedTicks();
    if (m_negativeFades[job].fadeAlpha < 0) {
        m_negativeFades[job].fadeAlpha = 0;
    }

    Pixmap newMaskPixmap = createSolidPixmap(m_display, screen().rootWindow().window(), m_negativeFades[job].width,
                                             m_negativeFades[job].height, m_negativeFades[job].fadeAlpha * 0x01010101);

    if (m_negativeFades[job].mask) {
        XRenderFreePicture(m_display, m_negativeFades[job].mask);
        m_negativeFades[job].mask = None;
    }
    m_negativeFades[job].mask = XRenderCreatePicture(m_display, newMaskPixmap, m_maskPictFormat, 0, NULL);

    XRenderComposite(m_display, PictOpDst, m_negativeFades[job].maskPictureHolder->picture(),
                     None, m_negativeFades[job].mask, 0, 0, 0, 0, 0, 0, m_negativeFades[job].width, m_negativeFades[job].height);

    maskPic_return = m_negativeFades[job].mask;
}

// Called after the extra rendering jobs are executed.
void FadePlugin::postExtraRenderingActions() throw(RuntimeException) {
    std::vector<NegFadeData>::iterator it = m_negativeFades.begin();
    while (it != m_negativeFades.end()) {
        if (it->fadeAlpha <= 0) {
            if (it->mask) {
                XRenderFreePicture(m_display, it->mask);
            }
            it = m_negativeFades.erase(it);
        } else {
            ++it;
        }
    }
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    // The screen must remain const, but there is no way to return a non-const
    // Display* from it. Since the plugin needs that pointer, I have to
    // const_cast it.
    Display *display = const_cast<Display*>(screen.display());
    return new FadePlugin(display, screen, args);
}

// Returns plugin's type.
extern "C" PluginType pluginType() {
    return Plugin_XRender;
}

#endif  // USE_XRENDER_COMPOSITING
