/** FadePlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_PLUGIN_XRENDER_FADE_FADEPLUGIN_HH
#define FBCOMPOSITOR_PLUGIN_XRENDER_FADE_FADEPLUGIN_HH

#include "config.h"

#ifdef USE_XRENDER_COMPOSITING


#include "Constants.hh"
#include "Exceptions.hh"
#include "TickTracker.hh"
#include "XRenderPlugin.hh"
#include "XRenderUtility.hh"

#include "FbTk/FbString.hh"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include <vector>
#include <map>


namespace FbCompositor {

    class BaseScreen;
    class FadePlugin;
    class InitException;
    class TickTracker;
    class XRenderPlugin;
    class XRenderScreen;
    class XRenderWindow;


    /**
     * A simple plugin that provides window fades for XRender renderer.
     */
    class FadePlugin : public XRenderPlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        FadePlugin(Display *display, const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw();

        /** Destructor. */
        ~FadePlugin() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the name of the plugin. */
        const char *pluginName() const throw();


        //--- WINDOW EVENT CALLBACKS -------------------------------------------

        /** Called, whenever a window is mapped. */
        void windowMapped(const BaseCompWindow &window) throw();

        /** Called, whenever a window is unmapped. */
        void windowUnmapped(const BaseCompWindow &window) throw();


        //--- RENDERING ACTIONS ------------------------------------------------

        /** Window rendering job initialization. */
        virtual void windowRenderingJobInit(const XRenderWindow &window, int &op_return,
                                            Picture &maskPic_return) throw(RuntimeException);

        /** Window rendering job cleanup. */
        virtual void windowRenderingJobCleanup(const XRenderWindow &window) throw(RuntimeException);


        /** \returns the number of extra rendering jobs the plugin will do. */
        virtual int extraRenderingJobCount() throw(RuntimeException);

        /** Initialize the specified extra rendering job. */
        virtual void extraRenderingJobInit(int job, int &op_return,
                                           Picture &srcPic_return, int &srcX_return, int &srcY_return,
                                           Picture &maskPic_return, int &maskX_return, int &maskY_return,
                                           int &destX_return, int &destY_return,
                                           int &width_return, int &height_return) throw(RuntimeException);

        /** Called after the extra rendering jobs are executed. */
        virtual void postExtraRenderingActions() throw(RuntimeException);


    private :
        //--- GENERAL RENDERING VARIABLES --------------------------------------

        /** Connection to the X server. */
        Display *m_display;

        /** PictFormat for mask pictures. */
        const XRenderPictFormat *m_maskPictFormat;


        //--- FADE SPECIFIC ----------------------------------------------------

        /** Holds the data about positive fades. */
        struct PosFadeData {
            int fadeAlpha;          ///< Window's relative fade alpha.
            Picture mask;           ///< Mask of the faded window.
            TickTracker timer;      ///< Timer that tracks the current fade.
        };

        /** A list of appearing (positive) fades. */
        std::map<Window, PosFadeData> m_positiveFades;


        /** Holds the data about positive fades. */
        struct NegFadeData {
            int origAlpha;                          ///< Window's original opacity.
            XRenderPicturePtr contentPictureHolder; ///< Window's contents.
            XRenderPicturePtr maskPictureHolder;    ///< Window's shape mask.
            int xCoord;                             ///< Window's X coordinate.
            int yCoord;                             ///< Window's Y coordinate.
            int width;                              ///< Window's width.
            int height;                             ///< Window's height.

            int fadeAlpha;                          ///< Window's fade relative alpha.
            Picture mask;                           ///< Mask of the faded window.
            TickTracker timer;                      ///< Timer that tracks the current fade.
        };

        /** A list of disappearing (negative) fades. */
        std::vector<NegFadeData> m_negativeFades;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the name of the plugin.
    inline const char *FadePlugin::pluginName() const throw() {
        return "fade";
    }
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

/** Creates a plugin object. */
extern "C" FbCompositor::BasePlugin *createPlugin(const FbCompositor::BaseScreen &screen, const std::vector<FbTk::FbString> &args);

/** \returns plugin's type. */
extern "C" FbCompositor::PluginType pluginType();


#endif  // USE_XRENDER_COMPOSITING

#endif  // FBCOMPOSITOR_PLUGIN_XRENDER_FADE_FADEPLUGIN_HH
