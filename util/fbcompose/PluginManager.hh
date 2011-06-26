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

    class BasePlugin;
    class InitException;
    class PluginManager;
    class RuntimeException;


    //--- TYPEDEFS -------------------------------------------------------------

    /** A pointer to a function that creates a plugin class instance. */
    typedef BasePlugin* (*CreatePluginFunction)(const std::vector<FbTk::FbString>&);

    /** A pointer to a function that destroys a plugin class instance. */
    typedef void (*DestroyPluginFunction)(BasePlugin*);


    /**
     * Responsible for plugin loading, unloading and availibility.
     */
    class PluginManager {
        struct PluginLibData;

    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        PluginManager() throw(InitException);

        /** Destructor. */
        ~PluginManager();


        //--- PLUGIN MANIPULATION ----------------------------------------------

        /** Create a plugin object, load the appropriate library if needed. */
        void createPluginObject(FbTk::FbString name, std::vector<FbTk::FbString> args = std::vector<FbTk::FbString>()) throw(RuntimeException);

        /** \returns a reference to a vector with plugin objects. */
        std::vector<BasePlugin*> &plugins() throw();


    private :
        //--- INTERNAL PLUGIN MANIPULATION -------------------------------------

        /** Load a plugin. */
        void loadPlugin(FbTk::FbString name) throw(RuntimeException);

        /** Unload a plugin. */
        void unloadPlugin(FbTk::FbString name) throw(RuntimeException);

        /** Unload a plugin (actual worker function). */
        void unloadPlugin(std::map<FbTk::FbString, PluginLibData>::iterator it);


        //--- CONVENIENCE FUNCTIONS --------------------------------------------

        /** Build a vector of search paths for a given plugin. */
        std::vector<FbTk::FbString> buildPluginPaths(const FbTk::FbString &name);


        //--- PLUGINS AND METADATA ---------------------------------------------

        /** Specific plugin-related data. */
        struct PluginLibData {
            void *handle;                           ///< Handle to the loaded library.
            CreatePluginFunction createFunction;    ///< Plugin creation function.
            DestroyPluginFunction destroyFunction;  ///< Plugin destruction function.
        };

        /** A map, containing all loaded plugins. */
        std::map<FbTk::FbString, PluginLibData> m_pluginLibs;

        /** A vector with active plugin objects. */
        std::vector<BasePlugin*> m_pluginObjects;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns a reference to a vector with plugin objects.
    inline std::vector<BasePlugin*> &PluginManager::plugins() throw() {
        return m_pluginObjects;
    }
}

#endif  // FBCOMPOSITOR_PLUGINMANAGER_HH
