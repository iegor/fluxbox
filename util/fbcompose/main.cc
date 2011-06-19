/** main.cc file for the fluxbox compositor. */

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


#include "Compositor.hh"
#include "Exceptions.hh"
#include "Logging.hh"

#include "FbTk/FbString.hh"

#include <X11/Xlib.h>

#include <vector>
#include <cstdlib>

using namespace FbCompositor;


/**
 * The entry point.
 *
 * \param argc The number of command line arguments.
 * \param argv An array of strings, containing the arguments.
 * \returns Application's exit status.
 */
int main(int argc, char **argv) {
    try {
        Logger::setLoggingLevel(LOG_LEVEL_WARN);

        std::vector<FbTk::FbString> args(argv + 1, argv + argc);
        CompositorConfig config(args);

        Compositor app(config);
        app.eventLoop();
    } catch (const ConfigException &e) {
        std::cerr << e.what() << std::endl;
        CompositorConfig::printShortHelp(std::cerr);
        return EXIT_FAILURE;
    } catch (const CompositorException &e) {
        fbLog_error << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
