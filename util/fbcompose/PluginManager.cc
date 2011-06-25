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

#include "PluginManager.hh"

#include <dlfcn.h>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Costructor.
PluginManager::PluginManager() throw(InitException) { }

// Destructor.
PluginManager::~PluginManager() {
    std::map<FbTk::FbString, PluginData>::iterator it = m_plugins.begin();
    while (it != m_plugins.end()) {
        unloadPluginProper(it);
        ++it;
    }
}


//--- PLUGIN MANIPULATION ------------------------------------------------------

// Load a plugin.
void PluginManager::loadPlugin(FbTk::FbString /*name*/, std::vector<FbTk::FbString> /*args*/) {
}

/** Set arguments for a particular plugin. */
void PluginManager::setPluginArguments(FbTk::FbString name, std::vector<FbTk::FbString> args) {
    m_plugins[name].args = args;
}

/** Unload a plugin. */
void PluginManager::unloadPlugin(FbTk::FbString name) {
    unloadPluginProper(m_plugins.find(name));
}


/** Return a vector with appropriately instantiated plugin objects. */
std::vector<BasePlugin*> PluginManager::instantiatePlugins(std::vector<FbTk::FbString> /*plugins*/) {
    return std::vector<BasePlugin*>();
}


//--- INTERNAL PLUGIN MANIPULATION ---------------------------------------------

// Unload a plugin.
void PluginManager::unloadPluginProper(std::map<FbTk::FbString, PluginData>::iterator /*it*/) {
}
