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

#include "FbTk/FbString.hh"

#include <X11/Xlib.h>

#include <vector>
#include <iosfwd>


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
        CompositorConfig(std::vector<FbTk::FbString> args) throw(ConfigException);

        /** Destructor. */
        ~CompositorConfig() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the display name. */
        const FbTk::FbString &displayName() const throw();

        /** \returns the selected rendering mode. */
        RenderingMode renderingMode() const throw();


        //--- CONVENIENCE FUNCTIONS --------------------------------------------

        /** Output full help message. */
        static void printFullHelp(std::ostream &os) throw();

        /** Output short help message. */
        static void printShortHelp(std::ostream &os) throw();

        /** Output version information. */
        static void printVersion(std::ostream &os) throw();

    private:
        //--- PRIVATE VARIABLES ------------------------------------------------

        /** The name of the display we want to use. */
        FbTk::FbString m_displayName;

        /** Selected rendering mode. */
        RenderingMode m_renderingMode;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the display name.
    inline const FbTk::FbString &CompositorConfig::displayName() const throw() {
        return m_displayName;
    }

    // Returns the rendering mode.
    inline RenderingMode CompositorConfig::renderingMode() const throw() {
        return m_renderingMode;
    }

}

#endif  // FBCOMPOSITOR_COMPOSITORCONFIG_HH
