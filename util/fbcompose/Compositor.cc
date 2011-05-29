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
#include "XRenderAutoScreen.hh"

#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>

#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// The constructor.
Compositor::Compositor(const CompositorConfig &config) throw(ConfigException) :
    App(config.displayName().c_str()) {

    m_renderingMode = config.renderingMode();

    initXExtensions();

    // Setting up screens.
    int screenCount = XScreenCount(display());
    m_screens.reserve(screenCount);
    for (int i = 0; i < screenCount; i++) {
        switch (m_renderingMode) {
        case RM_OpenGL :
            break;
        case RM_XRenderManual :
            break;
        case RM_XRenderAuto :
            m_screens.push_back(new XRenderAutoScreen(i));
            break;
        default:
            // TODO: Throw something.
            break;
        }
    }
}

// Destructor.
Compositor::~Compositor() { }


//--- INITIALIZATION FUNCTIONS -----------------------------------------

// Initializes X's extensions.
void Compositor::initXExtensions() throw(ConfigException) {
    if (!XCompositeQueryExtension(display(), &m_compositeEventBase, &m_compositeErrorBase)) {
        XCloseDisplay(display());
        throw ConfigException("XComposite extension not available.");
    }

    int compositeMajor;
    int compositeMinor;
    XCompositeQueryVersion(display(), &compositeMajor, &compositeMinor);

    if ((compositeMajor < MIN_XCOMPOSITE_MAJOR_VERSION)
            || ((compositeMajor == MIN_XCOMPOSITE_MAJOR_VERSION) && (compositeMinor < MIN_XCOMPOSITE_MINOR_VERSION))) {
        XCloseDisplay(display());
        throw ConfigException("Unsupported XComposite extension version.");
    }

    if (!XDamageQueryExtension(display(), &m_damageEventBase, &m_damageErrorBase)) {
        XCloseDisplay(display());
        throw ConfigException("XDamage extension not available.");
    }

    int damageMajor;
    int damageMinor;
    XDamageQueryVersion(display(), &damageMajor, &damageMinor);

    if ((damageMajor < MIN_XDAMAGE_MAJOR_VERSION)
            || ((damageMajor == MIN_XDAMAGE_MAJOR_VERSION) && (damageMinor < MIN_XDAMAGE_MINOR_VERSION))) {
        XCloseDisplay(display());
        throw ConfigException("Unsupported XDamage extension version.");
    }
}


//--- EVENT LOOP ---------------------------------------------------------------

// The event loop.
void Compositor::eventLoop() {
    XEvent event;

    while (!done()) {
        while (XPending(display())) {
            XNextEvent(display(), &event);

            int eventScreen = -1;
            for (unsigned int i = 0; i < m_screens.size(); i++) {
                if (event.xany.window == m_screens[i]->rootWindow().window()) {
                    eventScreen = i;
                    break;
                }
            }
            if (eventScreen < 0) {
                XWindowAttributes xwa;
                XGetWindowAttributes(display(), event.xany.window, &xwa);
                eventScreen = XScreenNumberOfScreen(xwa.screen);
            }

            switch (event.type) {
            case ConfigureNotify :
                m_screens[eventScreen]->reconfigureWindow(event.xconfigure);
                std::cout << "  ConfigureNotify on " << event.xconfigure.window << std::endl;
                break;
            case CreateNotify :
                m_screens[eventScreen]->createWindow(event.xcreatewindow.window);
                std::cout << "  CreateNotify on " << event.xcreatewindow.window << std::endl;
                break;
            case DestroyNotify :
                m_screens[eventScreen]->destroyWindow(event.xdestroywindow.window);
                std::cout << "  DestroyNotify on " << event.xdestroywindow.window << std::endl;
                break;
            case Expose :
                if (event.xexpose.count == 0) {
                    m_screens[eventScreen]->damageWindow(event.xexpose.window);
                }
                std::cout << "  Expose on " << event.xexpose.window << " (" << event.xexpose.count << ")" << std::endl;
                break;
            case MapNotify :
                m_screens[eventScreen]->mapWindow(event.xmap.window);
                std::cout << "  MapNotify on " << event.xmap.window << std::endl;
                break;
            case PropertyNotify :
                m_screens[eventScreen]->updateWindowProperty(event.xproperty.window, event.xproperty.atom, event.xproperty.state);
                std::cout << "  PropertyNotify on " << event.xproperty.window << " ("
                          << XGetAtomName(display(), event.xproperty.atom) << ")" << std::endl;
                break;
            case UnmapNotify :
                m_screens[eventScreen]->unmapWindow(event.xunmap.window);
                std::cout << "  UnmapNotify on " << event.xunmap.window << std::endl;
                break;
            default :
                if (event.type == (m_damageEventBase + XDamageNotify)) {
                    XDamageNotifyEvent damageEvent = *((XDamageNotifyEvent*) &event);
                    m_screens[eventScreen]->damageWindow(damageEvent.drawable);
                    std::cout << "  DamageNotify on " << damageEvent.drawable << std::endl;
                } else {
                    std::cout << "Event " << event.xany.type << " on screen " << eventScreen
                              << " and window " << event.xany.window << std::endl;
                }

                break;
            }
        }

        // TODO: Draw the screen here.
    }
}
