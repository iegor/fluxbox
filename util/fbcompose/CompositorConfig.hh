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

#include "config.h"

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

        /** Constructor. */
        CompositorConfig(std::vector<FbTk::FbString> args) throw(InitException);

        /** Copy constructor. */
        CompositorConfig(const CompositorConfig &other) throw();

        /** Assignment operator. */
        CompositorConfig &operator=(const CompositorConfig &other) throw();

        /** Destructor. */
        ~CompositorConfig() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the display name. */
        const FbTk::FbString &displayName() const throw();

        /** \returns the refresh rate. */
        int framesPerSecond() const throw();

        /** \returns plugins and their arguments. */
        const std::vector< std::pair< FbTk::FbString, std::vector<FbTk::FbString> > > &plugins() const throw();

        /** \returns the selected rendering mode. */
        RenderingMode renderingMode() const throw();

        /** \returns whether the X errors should be printed. */
        bool showXErrors() const throw();

#ifdef USE_XRENDER_COMPOSITING
        /** \returns the XRender picture filter. */
        const char *xRenderPictFilter() const throw();
#endif  // USE_XRENDER_COMPOSITING


        //--- CONVENIENCE FUNCTIONS --------------------------------------------

        /** Output full help message. */
        static void printFullHelp(std::ostream &os) throw();

        /** Output short help message. */
        static void printShortHelp(std::ostream &os) throw();

        /** Output version information. */
        static void printVersion(std::ostream &os) throw();

    private:
        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Make the first scan of the arguments for special options. */
        void preScanArguments() throw(ConfigException);

        /** Properly scan the command line arguments. */
        void processArguments() throw(ConfigException);


        /** Fetch the value of the next command line argument, advance iterator. */
        FbTk::FbString getNextOption(std::vector<FbTk::FbString>::iterator &it, const char *errorMessage);


        //--- PRIVATE VARIABLES ------------------------------------------------

        /** The passed command line arguments. */
        std::vector<FbTk::FbString> m_args;


        /** Selected rendering mode. */
        RenderingMode m_renderingMode;

#ifdef USE_XRENDER_COMPOSITING
        /** XRender picture filter. */
        FbTk::FbString m_xRenderPictFilter;
#endif  // USE_XRENDER_COMPOSITING


        /** The name of the display we want to use. */
        FbTk::FbString m_displayName;

        /** The refresh rate. */
        int m_framesPerSecond;

        /** Plugins and their arguments. */
        std::vector< std::pair< FbTk::FbString, std::vector<FbTk::FbString> > > m_plugins;

        /** Whether the X errors should be printed. */
        bool m_showXErrors;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the display name.
    inline const FbTk::FbString &CompositorConfig::displayName() const throw() {
        return m_displayName;
    }

    // Returns the refresh rate.
    inline int CompositorConfig::framesPerSecond() const throw() {
        return m_framesPerSecond;
    }

    // Returns plugins and their arguments.
    inline const std::vector< std::pair< FbTk::FbString, std::vector<FbTk::FbString> > > &CompositorConfig::plugins() const throw() {
        return m_plugins;
    }

    // Returns the rendering mode.
    inline RenderingMode CompositorConfig::renderingMode() const throw() {
        return m_renderingMode;
    }

    // Returns whether the X errors should be printed.
    inline bool CompositorConfig::showXErrors() const throw() {
        return m_showXErrors;
    }


#ifdef USE_XRENDER_COMPOSITING
    // Returns the XRender picture filter.
    inline const char *CompositorConfig::xRenderPictFilter() const throw() {
        return m_xRenderPictFilter.c_str();
    }
#endif  // USE_XRENDER_COMPOSITING
}

#endif  // FBCOMPOSITOR_COMPOSITORCONFIG_HH
