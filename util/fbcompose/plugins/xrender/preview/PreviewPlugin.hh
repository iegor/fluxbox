/** PreviewPlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_PLUGIN_XRENDER_PREVIEW_PREVIEWPLUGIN_HH
#define FBCOMPOSITOR_PLUGIN_XRENDER_PREVIEW_PREVIEWPLUGIN_HH


#include "Enumerations.hh"
#include "TickTracker.hh"
#include "XRenderPlugin.hh"
#include "XRenderResources.hh"

#include "FbTk/FbString.hh"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include <map>
#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class XRenderScreen;
    class XRenderWindow;


    /**
     * Provides window preview feature for the iconbar.
     */
    class PreviewPlugin : public XRenderPlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        PreviewPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args);

        /** Destructor. */
        ~PreviewPlugin();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the name of the plugin. */
        const char *pluginName() const;


        //--- WINDOW EVENT CALLBACKS -------------------------------------------

        /** Called, whenever a new window is created. */
        void windowCreated(const BaseCompWindow &window);

        /** Called, whenever a window is destroyed. */
        void windowDestroyed(const BaseCompWindow &window);


        //--- RENDERING ACTIONS ------------------------------------------------

        /** Rectangles that the plugin wishes to damage. */
        const std::vector<XRectangle> &damagedAreas();

        /** Extra rendering actions and jobs. */
        const std::vector<XRenderRenderingJob> &extraRenderingActions();


    private :
        //--- GENERAL RENDERING VARIABLES --------------------------------------

        /** Vector, containing the areas that the plugin wishes to paint. */
        std::vector<XRectangle> m_damagedAreas;

        /** Vector, containing the plugin's extra rendering jobs. */
        std::vector<XRenderRenderingJob> m_extraJobs;


        /** Preview window's mask. */
        XRenderPicturePtr m_maskPicture;

        /** Previously modified rectangle. */
        XRectangle m_previousDamage;

        /** Timer that signals when the preview window should appear. */
        TickTracker m_tickTracker;


        //--- PREVIEW WINDOW DATA ----------------------------------------------

        /** Holds data about the preview window. */
        struct PreviewWindowData {
            const XRenderWindow &window;
            XRenderPicturePtr previewPicture;
            XRectangle dimensions;
        };

        /** A list of potential preview windows. */
        std::map<Window, PreviewWindowData> m_previewData;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the name of the plugin.
    inline const char *PreviewPlugin::pluginName() const {
        return "preview";
    }
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

/** Creates a plugin object. */
extern "C" FbCompositor::BasePlugin *createPlugin(const FbCompositor::BaseScreen &screen, const std::vector<FbTk::FbString> &args);

/** \returns the type of the plugin. */
extern "C" FbCompositor::PluginType pluginType();


#endif  // FBCOMPOSITOR_PLUGIN_XRENDER_PREVIEW_PREVIEWPLUGIN_HH
