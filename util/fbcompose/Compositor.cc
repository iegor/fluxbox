/** Compositor.cc file for the fluxbox compositor. */

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

#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>

#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// The constructor.
Compositor::Compositor(const CompositorConfig &config) :
    App(config.displayName().c_str()) {

    m_renderingMode = config.renderingMode();

    // Checking for XComposite extension.
    if(!XCompositeQueryExtension(display(), &m_compositeEventBase, &m_compositeErrorBase)) {
        XCloseDisplay(display());
        throw ConfigException("XComposite extension not available.");
    }

    int compositeMajor;
    int compositeMinor;
    XCompositeQueryVersion(display(), &compositeMajor, &compositeMinor);

    if((compositeMajor < MIN_XCOMPOSITE_MAJOR_VERSION) 
            || ((compositeMajor == MIN_XCOMPOSITE_MAJOR_VERSION) && (compositeMinor < MIN_XCOMPOSITE_MINOR_VERSION))) {
        XCloseDisplay(display());
        throw ConfigException("Unsupported XComposite extension version.");
    }

    // Checking for XDamage extension.
    if(!XDamageQueryExtension(display(), &m_damageEventBase, &m_damageErrorBase)) {
        XCloseDisplay(display());
        throw ConfigException("XDamage extension not available.");
    }

    int damageMajor;
    int damageMinor;
    XDamageQueryVersion(display(), &damageMajor, &damageMinor);

    if((damageMajor < MIN_XDAMAGE_MAJOR_VERSION) 
            || ((damageMajor == MIN_XDAMAGE_MAJOR_VERSION) && (damageMinor < MIN_XDAMAGE_MINOR_VERSION))) {
        XCloseDisplay(display());
        throw ConfigException("Unsupported XDamage extension version.");
    }

    // Setting up screens.
    int screenCount = XScreenCount(display());
    m_screens.reserve(screenCount);
    for(int i = 0; i < screenCount; i++) {
        m_screens.push_back(BaseScreen(display(), i));
    }
}

// Destructor.
Compositor::~Compositor() { }


//--- EVENT LOOP ---------------------------------------------------------------

// The event loop.
void Compositor::eventLoop() {
    XEvent event;

    while(!done()) {
        while(XPending(display())) {
            XNextEvent(display(), &event);

            // TODO: Use size_t?
            int eventScreen = -1;
            for(int i = 0; i < int(m_screens.size()); i++) {
                if(event.xany.window == m_screens[i].rootWindow().window()) {
                    eventScreen = i;
                    break;
                }
            }
            if(eventScreen < 0) {
                // TODO: Do something here.
            }

            std::cout << "Event " << event.xany.type << " on screen " << eventScreen << std::endl;

            switch(event.type) {
            case ConfigureNotify :
                std::cout << "  ConfigureNotify on " << event.xconfigure.window << std::endl;
                break;
            case CreateNotify :
                m_screens[eventScreen].createWindow(event.xcreatewindow);
                std::cout << "  CreateNotify on " << event.xcreatewindow.window << std::endl;
                break;
            case DestroyNotify :
                m_screens[eventScreen].destroyWindow(event.xdestroywindow);
                std::cout << "  DestroyNotify on " << event.xdestroywindow.window << std::endl;
                break;
            case Expose :
                std::cout << "  Expose on " << event.xexpose.window << std::endl;
                break;
            case MapNotify :
                m_screens[eventScreen].mapWindow(event.xmap);
                std::cout << "  MapNotify on " << event.xmap.window << std::endl;
                break;
            case PropertyNotify :
                std::cout << "  PropertyNotify on " << event.xproperty.window << std::endl;
                break;
            case UnmapNotify :
                m_screens[eventScreen].unmapWindow(event.xunmap);
                std::cout << "  UnmapNotify on " << event.xunmap.window << std::endl;
                break;
            default:
                break;
            }
        }

        // TODO: Draw the screen here.
    }
}
