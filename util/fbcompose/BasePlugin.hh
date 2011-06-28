/** BasePlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_BASEPLUGIN_HH
#define FBCOMPOSITOR_BASEPLUGIN_HH

#include "config.h"

#include "Constants.hh"
#include "Exceptions.hh"

#include "FbTk/FbString.hh"

#include <vector>


namespace FbCompositor {

    class BasePlugin;
    class InitException;


    /**
     * Base class for compositor plugins.
     */
    class BasePlugin {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        BasePlugin(const std::vector<FbTk::FbString> &args) throw(InitException);

        /** Destructor. */
        virtual ~BasePlugin();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the name of the plugin. */
        virtual const char *pluginName() const throw() = 0;
    };
}


#endif  // FBCOMPOSITOR_BASEPLUGIN_HH
