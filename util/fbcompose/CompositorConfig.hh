/** CompositorConfig.cc file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_COMPOSITORCONFIG_HH
#define FBCOMPOSITOR_COMPOSITORCONFIG_HH

#include "Constants.hh"
#include "Exceptions.hh"

#include <X11/Xlib.h>


namespace FbCompositor {

    class CompositorConfig;
    class ConfigException;

    /**
     * Handles the compositor's configuration.
     *
     * This class is responsible for obtaining the compositor's configuration,
     * ensuring that all of it is correct and presenting it to the Compositor
     * class.
     */
    class CompositorConfig {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** The constructor. */
        CompositorConfig(int argc, char **argv) throw(ConfigException);

        /** Destructor. */
        ~CompositorConfig();


        //--- ACCESSORS --------------------------------------------------------
        
        /** \returns the display name. */
        const std::string &displayName() const throw();

        /** \returns the selected rendering mode. */
        RenderingMode renderingMode() const throw();


    private:
        //--- PRIVATE VARIABLES ------------------------------------------------
        
        /** The name of the display we want to use. */
        std::string m_displayName;

        /** Selected rendering mode. */
        RenderingMode m_renderingMode;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // \returns the display name.
    inline const std::string &CompositorConfig::displayName() const throw() {
        return m_displayName;
    }

    // Returns the rendering mode.
    inline RenderingMode CompositorConfig::renderingMode() const throw() {
        return m_renderingMode;
    }

}

#endif  // FBCOMPOSITOR_COMPOSITORCONFIG_HH
