/** TestPlugin.hh file for the fluxbox compositor. */

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

#ifndef FBCOMPOSITOR_TESTPLUGIN_HH
#define FBCOMPOSITOR_TESTPLUGIN_HH

#include "BasePlugin.hh"
#include "Constants.hh"
#include "Exceptions.hh"

#include "FbTk/FbString.hh"

#include <vector>


namespace FbCompositor {

    class BasePlugin;
    class InitException;
    class TestPlugin;

    /**
     * A simple plugin for testing purposes. Will be removed or replaced with a
     * better fitting example.
     */
    class TestPlugin : public BasePlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -----------------------------------------

        /** Constructor. */
        TestPlugin(const std::vector<FbTk::FbString> &args) throw(InitException);

        /** Destructor. */
        virtual ~TestPlugin();


        //--- ACCESSORS ------------------------------------------------------------

        /** \returns the name of the plugin. */
        const char *pluginName() const throw();

        /** \returns which rendering mode the plugin is written for. */
        RenderingMode pluginType() const throw();
    };


    //--- INLINE FUNCTIONS ---------------------------------------------------------

    // Returns the name of the plugin.
    inline const char *TestPlugin::pluginName() const throw() {
        return "Test";
    }

    // Returns which rendering mode the plugin is written for.
    inline RenderingMode TestPlugin::pluginType() const throw() {
        return RM_OpenGL;
    }
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

/** Creates a plugin object. */
extern "C" FbCompositor::BasePlugin *createPlugin(const std::vector<FbTk::FbString> &args);

/** Destroys a plugin object. */
extern "C" void destroyPlugin(FbCompositor::BasePlugin *plugin);


#endif  // FBCOMPOSITOR_TESTPLUGIN_HH
