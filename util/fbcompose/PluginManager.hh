/** PluginManager.hh file for the fluxbox compositor. */

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

#ifndef FBCOMPOSITOR_PLUGINMANAGER_HH
#define FBCOMPOSITOR_PLUGINMANAGER_HH

#include "config.h"

#include "BasePlugin.hh"
#include "Constants.hh"
#include "Exceptions.hh"

#include "FbTk/FbString.hh"

#include <map>
#include <vector>


namespace FbCompositor {

    class InitException;


    //--- TYPEDEFS -------------------------------------------------------------

    /** A pointer to a function that creates a plugin class instance. */
    typedef BasePlugin* (*CreatePluginFunction)(const std::vector<FbTk::FbString>&);


    /**
     * Responsible for plugin loading, unloading and availibility.
     */
    class PluginManager {
        struct PluginData;

    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        PluginManager() throw(InitException);

        /** Destructor. */
        ~PluginManager();


        //--- PLUGIN MANIPULATION ----------------------------------------------

        /** Load a plugin. */
        void loadPlugin(FbTk::FbString name, std::vector<FbTk::FbString> args = std::vector<FbTk::FbString>());

        /** Set arguments for a particular plugin. */
        void setPluginArguments(FbTk::FbString name, std::vector<FbTk::FbString> args);
        
        /** Unload a plugin. */
        void unloadPlugin(FbTk::FbString name);


        /** Return a vector with appropriately instantiated plugin objects. */
        std::vector<BasePlugin*> instantiatePlugins(std::vector<FbTk::FbString> plugins = std::vector<FbTk::FbString>());


    private :
        //--- INTERNAL PLUGIN MANIPULATION -------------------------------------

        /** Unload a plugin. */
        void unloadPluginProper(std::map<FbTk::FbString, PluginData>::iterator it);


        //--- PLUGINS AND METADATA ---------------------------------------------

        /** Specific plugin-related data. */
        struct PluginData {
            FbTk::FbString name;                    ///< Name of the plugin.
            void *handle;                           ///< Handle to the loaded library.
            CreatePluginFunction factoryFunction;   ///< Plugin creation function.
            std::vector<FbTk::FbString> args;       ///< Plugin arguments.
        };

        /** A map, containing all loaded plugins. */
        std::map<FbTk::FbString, PluginData> m_plugins;
    };

}

#undef EMTPY_VECTOR

#endif  // FBCOMPOSITOR_PLUGINMANAGER_HH
