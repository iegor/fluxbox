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


#ifndef FBCOMPOSITOR_PLUGIN_OPENGL_FADE_FADEPLUGIN_HH
#define FBCOMPOSITOR_PLUGIN_OPENGL_FADE_FADEPLUGIN_HH

#include "config.h"

#ifdef USE_OPENGL_COMPOSITING


#include "Constants.hh"
#include "Exceptions.hh"
#include "OpenGLPlugin.hh"
#include "TickTracker.hh"

#include "FbTk/FbString.hh"

#include <GL/glew.h>
#include <GL/gl.h>

#include <X11/Xlib.h>

#include <vector>
#include <map>


namespace FbCompositor {

    class BaseScreen;
    class FadePlugin;
    class InitException;
    class OpenGLPlugin;
    class OpenGLWindow;
    class TickTracker;


    /**
     * A simple plugin for testing purposes. Will be removed or replaced with a
     * better fitting example.
     */
    class FadePlugin : public OpenGLPlugin {
        struct FadeData;

    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw(InitException);

        /** Destructor. */
        ~FadePlugin() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the name of the plugin. */
        const char *pluginName() const throw();

        
        /** \returns the additional source code for the fragment shader. */
        const char *fragmentShader() const throw();

        /** \returns the additional source code for the vertex shader. */
        const char *vertexShader() const throw();


        //--- PLUGIN ACTIONS ---------------------------------------------------

        /** Pre background rendering actions. */
        void preBackgroundRenderActions();

        /** Pre window rendering actions. */
        void preReconfigureRectRenderActions(XRectangle reconfigureRect);

        /** Pre window rendering actions. */
        void preWindowRenderActions(const OpenGLWindow &window);


        //--- WINDOW EVENT CALLBACKS -------------------------------------------

        /** Called, whenever a window is mapped. */
        void windowMapped(const BaseCompWindow &window);

        /** Called, whenever a window is unmapped. */
        void windowUnmapped(const BaseCompWindow &window);

    private :
        //--- INTERNALS --------------------------------------------------------

        /** Holds the data about fades. */
        struct FadeData {
            int alpha;              ///< Window's relative alpha.
            TickTracker timer;      ///< Timer that tracks the current fade.
        };

        /** A list of appearing (positive) fades. */
        std::map<Window, FadeData> m_positiveFades;
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


#endif  // USE_OPENGL_COMPOSITING

#endif  // FBCOMPOSITOR_PLUGIN_OPENGL_FADE_FADEPLUGIN_HH
