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
        FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw();

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
        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** \returns the faded mask picture for the given window fade. */
        void createFadedMask(int alpha, XRenderPicturePtr mask, XRectangle dimensions,
                             Pixmap &fadePixmap_return, Picture &fadePicture_return) throw();


        //--- GENERAL RENDERING VARIABLES --------------------------------------

        /** PictFormat for mask pictures. */
        XRenderPictFormat *m_maskPictFormat;


        //--- FADE SPECIFIC ----------------------------------------------------

        /** Holds the data about positive fades. */
        struct PosFadeData {
            int fadeAlpha;          ///< Window's relative fade alpha.
            Picture fadePicture;    ///< Picture of the faded window mask.
            Pixmap fadePixmap;      ///< Pixmap of the faded window mask.
            TickTracker timer;      ///< Timer that tracks the current fade.
        };

        /** A list of appearing (positive) fades. */
        std::map<Window, PosFadeData> m_positiveFades;


        /** Holds the data about positive fades. */
        struct NegFadeData {
            Window windowId;                    ///< ID of the window that is being faded.
            int origAlpha;                      ///< Window's original opacity.
            XRenderPicturePtr contentPicture;   ///< Window's contents.
            XRenderPicturePtr maskPicture;      ///< Window's shape mask.
            XRectangle dimensions;              ///< Window's dimensions.

            int fadeAlpha;                      ///< Window's relative fade alpha.
            Picture fadePicture;                ///< Picture of the faded window mask.
            Pixmap fadePixmap;                  ///< Pixmap of the faded window mask.
            TickTracker timer;                  ///< Timer that tracks the current fade.
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


#endif  // FBCOMPOSITOR_PLUGIN_XRENDER_FADE_FADEPLUGIN_HH
