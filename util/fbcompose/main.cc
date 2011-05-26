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

#include <X11/Xlib.h>
#include <X11/X.h>

#include <iostream>
#include <cstdlib>

using namespace FbCompositor;


/**
 * The entry point.
 *
 * \param argc The number of command line arguments.
 * \param argv An array of strings, containing the arguments themselves.
 * \returns Application's exit status.
 */
int main(int argc, char **argv) {
    try {
        CompositorConfig config(argc, argv);
        Compositor app(config);

        std::cout << app.screenCount() << " screen(s) available." << std::endl;
        for(int i = 0; i < app.screenCount(); i++) {
            std::cout << "Root window of screen " << i << ": " 
                      << app.getScreen(i).rootWindow() << std::endl;
            
            std::list<BaseCompWindow>::const_iterator it = app.getScreen(i).allWindows().begin();
            while(it != app.getScreen(i).allWindows().end()) {
                std::cout << "  " << *it << std::endl;
                it++;
            }
        }

        app.eventLoop();
    } catch(CompositorException e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
