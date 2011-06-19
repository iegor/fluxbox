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

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
CompositorConfig::CompositorConfig(std::vector<FbTk::FbString> args) throw(ConfigException) :
    m_displayName(""),
    m_renderingMode(RM_XRender) {
    // TODO: Proper command line argument parsing (getopt or something else).

    std::vector<FbTk::FbString>::iterator it;
    std::stringstream ss;

    it = args.begin();
    while (it != args.end()) {
        if ((*it == "-h") || (*it == "--help")) {
            printFullHelp(std::cout);
            exit(EXIT_SUCCESS);
        } else if ((*it == "-V") || (*it == "--version")) {
            printVersion(std::cout);
            exit(EXIT_SUCCESS);
        }
        ++it;
    }

    it = args.begin();
    while (it != args.end()) {
        if ((*it == "-m") || (*it == "--mode")) {
            ++it;
            if (it == args.end()) {
                throw ConfigException("No rendering mode specified.");
            }

            if (*it == "opengl") {
                m_renderingMode = RM_OpenGL;
            } else if (*it == "xrender") {
                m_renderingMode = RM_XRender;
            } else if (*it == "serverauto") {
                m_renderingMode = RM_ServerAuto;
            } else {
                ss.str("");
                ss << "Unknown rendering mode \"" << *it << "\".";
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

// Destructor.
CompositorConfig::~CompositorConfig() throw() {}


//--- CONVENIENCE FUNCTIONS ----------------------------------------------------

// Output full help message.
void CompositorConfig::printFullHelp(std::ostream &os) throw() {
    os << "Usage: fbcompose [OPTION]..." << std::endl
       << std::endl
       << "Options and arguments:" << std::endl
       << "  -h, --help           : Print this text and exit." << std::endl
       << "  -m MODE, --mode MODE : Select the rendering mode." << std::endl
       << "                         MODE can be \"opengl\", \"xrender\" or \"serverauto\"." << std::endl
       << "  -v, --verbose        : Print more information." << std::endl
       << "  -V, --version        : Print version and exit." << std::endl;
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
