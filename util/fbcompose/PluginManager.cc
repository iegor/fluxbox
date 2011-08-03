/** PluginManager.cc file for the fluxbox compositor. */

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


#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif  // HAVE_CONFIG_H

#include "PluginManager.hh"

#include "BasePlugin.hh"
#include "BaseScreen.hh"
#include "Exceptions.hh"
#include "Logging.hh"

#include <algorithm>
#include <dlfcn.h>
#include <sstream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Costructor.
PluginManager::PluginManager(PluginType pluginType, const BaseScreen &screen) :
    m_screen(screen) {

    m_pluginType = pluginType;
}

// Destructor.
PluginManager::~PluginManager() {
    for (size_t i = 0; i < m_pluginObjects.size(); i++) {
        delete m_pluginObjects[i];
    }

    std::map<FbTk::FbString, PluginLibData>::iterator it = m_pluginLibs.begin();
    while (it != m_pluginLibs.end()) {
        unloadPlugin(it);
        ++it;
    }
}


//--- PLUGIN MANIPULATION ------------------------------------------------------

// Create a plugin object, load the appropriate library if needed.
void PluginManager::createPluginObject(FbTk::FbString name, std::vector<FbTk::FbString> args) {
    if (m_pluginLibs.find(name) == m_pluginLibs.end()) {
        loadPlugin(name);
    }

    CreatePluginFunction createFunction = m_pluginLibs.find(name)->second.createFunction;
    BasePlugin *newPluginObject = (*createFunction)(m_screen, args);
    m_pluginObjects.push_back(newPluginObject);
}


//--- INTERNAL PLUGIN MANIPULATION ---------------------------------------------

// Load a plugin.
void PluginManager::loadPlugin(FbTk::FbString name) {
    static union {
        void *voidPtr;
        PluginTypeFunction pluginTypeFunc;
        CreatePluginFunction createPluginFunc;
    } objUnion;

    std::vector<FbTk::FbString> paths = buildPluginPaths(name);

    // Get the handle to the plugin so object.
    void *handle = NULL;
    for (size_t i = 0; i < paths.size(); i++) {
        handle = dlopen(paths[i].c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (handle) {
            break;
        }
    }
    if (!handle) {
        std::stringstream ss;
        ss << "Could not find/load plugin \"" << name << "\".";
        throw PluginException(ss.str());
    }

    // Check for the correct plugin type.
    objUnion.voidPtr = getLibraryObject(handle, "pluginType", name.c_str(), "type function");
    PluginTypeFunction typeFunc = objUnion.pluginTypeFunc;

    if ((*(typeFunc))() != m_pluginType) {
        std::stringstream ss;
        ss << "Plugin \"" << name << "\" is of the wrong type.";
        throw PluginException(ss.str());
    }

    // Get the plugin creation function.
    objUnion.voidPtr = getLibraryObject(handle, "createPlugin", name.c_str(), "creation function");
    CreatePluginFunction createFunc = objUnion.createPluginFunc;

    // Track the loaded plugin.
    PluginLibData pluginData = { handle, createFunc };
    m_pluginLibs.insert(make_pair(name, pluginData));
}

// Unload a plugin.
void PluginManager::unloadPlugin(FbTk::FbString name) {
    std::map<FbTk::FbString, PluginLibData>::iterator it = m_pluginLibs.find(name);

    if (it == m_pluginLibs.end()) {
        std::stringstream ss;
        ss << "Plugin \"" << name << "\" is not loaded (unloadPlugin).";
        throw PluginException(ss.str());
    } else {
        unloadPlugin(it);
    }
}

// Unload a plugin (actual worker function).
void PluginManager::unloadPlugin(std::map<FbTk::FbString, PluginLibData>::iterator it) {
    dlclose(it->second.handle);

    it->second.handle = NULL;
    it->second.createFunction = NULL;
}


//--- CONVENIENCE FUNCTIONS ----------------------------------------------------

// Build a vector of search paths for a given plugin.
std::vector<FbTk::FbString> PluginManager::buildPluginPaths(const FbTk::FbString &name) {
    std::stringstream ss;
    std::vector<FbTk::FbString> paths;

    FbTk::FbString typeDir = "";
    if (m_pluginType == Plugin_OpenGL) {
        typeDir = "opengl/";
    } else if (m_pluginType == Plugin_XRender) {
        typeDir = "xrender/";
    }

    ss << "./plugins/" << typeDir << name << "/.libs/" << name << ".so";
    paths.push_back(ss.str());
    ss.str("");

    ss << "./plugins/" << typeDir << name << ".so";
    paths.push_back(ss.str());
    ss.str("");

    paths.push_back(name);

    return paths;
}

// Returns some object from the given library handle.
void *PluginManager::getLibraryObject(void *handle, const char *objectName, const char *pluginName,
                                      const char *verboseObjectName) {
    dlerror();
    void *rawObject = dlsym(handle, objectName);
    const char *error = dlerror();

    if (error) {
        dlclose(handle);
        std::stringstream ss;
        ss << "Error in loading " << verboseObjectName << " for \"" << pluginName << "\" plugin: " << error;
        throw PluginException(ss.str());
    } else {
        return rawObject;
    }
}
