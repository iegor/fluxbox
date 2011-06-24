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


#include "CompositorConfig.hh"
#include "Logging.hh"

#ifdef USE_XRENDER_COMPOSITING
#include <X11/extensions/Xrender.h>
#endif  // USE_XRENDER_COMPOSITING

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
CompositorConfig::CompositorConfig(std::vector<FbTk::FbString> args) throw(ConfigException) :
    m_args(args),

#ifdef USE_OPENGL_COMPOSITING
    m_renderingMode(RM_OpenGL),
#else
#ifdef USE_XRENDER_COMPOSITING
    m_renderingMode(RM_XRender),
#else
    m_renderingMode(RM_ServerAuto),
#endif
#endif

#ifdef USE_XRENDER_COMPOSITING
    m_xRenderPictFilter(FilterFast),
#endif

    m_displayName(""),
    m_framesPerSecond(60) {

    preScanArguments();
    processArguments();
}

// Destructor.
CompositorConfig::~CompositorConfig() throw() {}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Make the first scan of the arguments for special options.
void CompositorConfig::preScanArguments() {
    std::vector<FbTk::FbString>::iterator it = m_args.begin();

    while (it != m_args.end()) {
        if ((*it == "-h") || (*it == "--help")) {
            printFullHelp(std::cout);
            exit(EXIT_SUCCESS);
        } else if ((*it == "-V") || (*it == "--version")) {
            printVersion(std::cout);
            exit(EXIT_SUCCESS);
        }
        ++it;
    }
}

// Properly scan the command line arguments.
void CompositorConfig::processArguments() throw(ConfigException) {
    std::vector<FbTk::FbString>::iterator it = m_args.begin();
    std::stringstream ss;

    while (it != m_args.end()) {
        if ((*it == "-d") || (*it == "--display")) {
            m_displayName = getNextOption(it, "No display string specified.");
        } else if ((*it == "-m") || (*it == "--mode")) {
            FbTk::FbString mode = getNextOption(it, "No rendering mode specified.");

#ifdef USE_OPENGL_COMPOSITING
            if (mode == "opengl") {
                m_renderingMode = RM_OpenGL;
            } else
#endif  // USE_OPENGL_COMPOSITING
                
#ifdef USE_XRENDER_COMPOSITING
            if (mode == "xrender") {
                m_renderingMode = RM_XRender;
            } else
#endif  // USE_XRENDER_COMPOSITING

            if (mode == "serverauto") {
                m_renderingMode = RM_ServerAuto;
            } else {
                ss.str("");
                ss << "Unknown rendering mode \"" << mode << "\".";
                throw ConfigException(ss.str());
            }
        } else if ((*it == "-q") || (*it == "--quiet")) {
            Logger::setLoggingLevel(LOG_LEVEL_NONE);
        } else if ((*it == "-r") || (*it == "--refresh-rate")) {
            ss.str(getNextOption(it, "No refresh rate specified."));
            ss >> m_framesPerSecond;

            if (m_framesPerSecond <= 0) {
                ss.str("");
                ss << "Invalid refresh rate given.";
                throw ConfigException(ss.str());
            }
        } else if ((*it == "-v") || (*it == "--verbose")) {
            Logger::setLoggingLevel(LOG_LEVEL_INFO);
        } else {
            ss.str("");
            ss << "Unknown option \"" << *it << "\".";
            throw ConfigException(ss.str());
        }
        ++it;
    }
}


// Fetch the value of the next command line argument, advance iterator.
FbTk::FbString CompositorConfig::getNextOption(std::vector<FbTk::FbString>::iterator &it, const char *errorMessage) {
    ++it;
    if (it == m_args.end()) {
        throw ConfigException(errorMessage);
    }
    return *it;
}


//--- CONVENIENCE FUNCTIONS ----------------------------------------------------

// Output full help message.
void CompositorConfig::printFullHelp(std::ostream &os) throw() {
    static const char modes[] = 
#ifdef USE_OPENGL_COMPOSITING
        "opengl, "
#endif  // USE_OPENGL_COMPOSITING
#ifdef USE_XRENDER_COMPOSITING
        "xrender, "
#endif  // USE_XRENDER_COMPOSITING
        "serverauto";

    os << "Usage: fbcompose [OPTION]..." << std::endl
       << "Options and arguments:" << std::endl
       << "  -d DISPLAY, --display DISPLAY" << std::endl
       << "                         Use the specified display connection." << std::endl
       << "  -h, --help             Print this text and exit." << std::endl
       << "  -m MODE, --mode MODE   Select the rendering mode." << std::endl
       << "                         MODE can be " << modes << "." << std::endl
       << "  -q, --quiet            Do not print anything." << std::endl
       << "  -r RATE, --refresh-rate RATE" << std::endl
       << "                         Specify the compositor's refresh rate in Hz" << std::endl
       << "                         (aka frames per second)." << std::endl
       << "  -v, --verbose          Print more information." << std::endl
       << "  -V, --version          Print version and exit." << std::endl;
}

// Output short help message.
void CompositorConfig::printShortHelp(std::ostream &os) throw() {
    os << "Usage: fbcompose [OPTION]..." << std::endl
       << "Try `fbcompose --help` for more information." << std::endl;
}

// Output version information.
void CompositorConfig::printVersion(std::ostream &os) throw() {
    os << "Fluxbox compositor %VERSION%" << std::endl
       << "Copyright (c) 2011 Gediminas Liktaras" << std::endl;
}
