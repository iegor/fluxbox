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
#include "OpenGLScreen.hh"

#include <GL/glx.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xutil.h>

#include <iostream>
#include <sstream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// The constructor.
Compositor::Compositor(const CompositorConfig &config) throw(ConfigException) :
    App(config.displayName().c_str()) {

    m_renderingMode = config.renderingMode();

    XSetErrorHandler(&handleXError);

    initXExtensions();
    if (m_renderingMode == RM_OpenGL) {
        initGLX();
    }

    // Setting up screens.
    int screenCount = XScreenCount(display());
    m_screens.reserve(screenCount);
    for (int i = 0; i < screenCount; i++) {
        switch (m_renderingMode) {
        case RM_OpenGL :
            m_screens.push_back(new OpenGLScreen(i));
            break;
        case RM_XRender :
            break;
        case RM_ServerAuto :
            XCompositeRedirectSubwindows(display(), XRootWindow(display(), i), CompositeRedirectAutomatic);
            break;
        default:
            // TODO: Throw something.
            break;
        }

        getCMSelectionOwnership(i);
    }

    for (int i = 0; i < screenCount; i++) {
        m_screens[i]->initWindows();
    }

    XFlush(display());
}

// Destructor.
Compositor::~Compositor() { }


//--- INITIALIZATION FUNCTIONS -----------------------------------------

// Acquires the ownership of compositing manager selections.
void Compositor::getCMSelectionOwnership(int screenNumber) throw(ConfigException) {
    std::stringstream ss;
    ss << "_NET_WM_CM_S" << screenNumber;
    Atom cmAtom = XInternAtom(display(), ss.str().c_str(), False);

    Window curOwner = XGetSelectionOwner(display(), cmAtom);
    if (curOwner != None) {
        // TODO: More detailed message - what is the other program?
        throw ConfigException("Another compositing manager is running.");
    }

    // TODO: Better way of obtaining program's name in SetWMProperties.
    curOwner = XCreateSimpleWindow(display(), XRootWindow(display(), screenNumber), 0, 0, 1, 1, 0, None, None);
    XmbSetWMProperties(display(), curOwner, "fbcompose", "fbcompose", NULL, 0, NULL, NULL, NULL);
    XSetSelectionOwner(display(), cmAtom, curOwner, CurrentTime);
}

// Initializes the GLX extension.
void Compositor::initGLX() throw(ConfigException) {
    // GLX extension.
    if (!glXQueryExtension(display(), &m_glxEventBase, &m_glxErrorBase)) {
        throw ConfigException("GLX extension not available.");
    }

    int glxMajor;
    int glxMinor;
    glXQueryVersion(display(), &glxMajor, &glxMinor);

    if ((glxMajor < MIN_XCOMPOSITE_MAJOR_VERSION)
            || ((glxMajor == MIN_XCOMPOSITE_MAJOR_VERSION) && (glxMinor < MIN_XCOMPOSITE_MINOR_VERSION))) {
        throw ConfigException("Unsupported GLX extension version.");
    }
}

// Initializes X's extensions.
void Compositor::initXExtensions() throw(ConfigException) {
    // XComposite extension.
    if (!XCompositeQueryExtension(display(), &m_compositeEventBase, &m_compositeErrorBase)) {
        throw ConfigException("XComposite extension not available.");
    }

    int compositeMajor;
    int compositeMinor;
    XCompositeQueryVersion(display(), &compositeMajor, &compositeMinor);

    if ((compositeMajor < MIN_XCOMPOSITE_MAJOR_VERSION)
            || ((compositeMajor == MIN_XCOMPOSITE_MAJOR_VERSION) && (compositeMinor < MIN_XCOMPOSITE_MINOR_VERSION))) {
        throw ConfigException("Unsupported XComposite extension version.");
    }

    // XDamage extension.
    if (!XDamageQueryExtension(display(), &m_damageEventBase, &m_damageErrorBase)) {
        throw ConfigException("XDamage extension not available.");
    }

    int damageMajor;
    int damageMinor;
    XDamageQueryVersion(display(), &damageMajor, &damageMinor);

    if ((damageMajor < MIN_XDAMAGE_MAJOR_VERSION)
            || ((damageMajor == MIN_XDAMAGE_MAJOR_VERSION) && (damageMinor < MIN_XDAMAGE_MINOR_VERSION))) {
        throw ConfigException("Unsupported XDamage extension version.");
    }

    // XFixes extension.
    if (!XFixesQueryExtension(display(), &m_fixesEventBase, &m_fixesErrorBase)) {
        throw ConfigException("XFixes extension not available.");
    }

    int fixesMajor;
    int fixesMinor;
    XFixesQueryVersion(display(), &fixesMajor, &fixesMinor);

    if ((fixesMajor < MIN_XDAMAGE_MAJOR_VERSION)
            || ((fixesMajor == MIN_XDAMAGE_MAJOR_VERSION) && (fixesMinor < MIN_XDAMAGE_MINOR_VERSION))) {
        throw ConfigException("Unsupported XFixes extension version.");
    }
}


//--- EVENT LOOP ---------------------------------------------------------------

// The event loop.
void Compositor::eventLoop() {
    XEvent event;
    bool changesOccured = false;

    while (!done()) {
        if (m_renderingMode == RM_ServerAuto) {
            continue;
        }

        while (XPending(display())) {
            XNextEvent(display(), &event);
            changesOccured = true;

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
                    std::cout << "  Expose on " << event.xexpose.window << " (" << event.xexpose.count << ")" << std::endl;
                }
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
                    XDamageNotifyEvent damageEvent = *((XDamageNotifyEvent*) &event);   // TODO: Better cast.
                    m_screens[eventScreen]->damageWindow(damageEvent.drawable);
                    std::cout << "  DamageNotify on " << damageEvent.drawable << std::endl;
                } else {
                    std::cout << "Event " << event.xany.type << " on screen " << eventScreen
                              << " and window " << event.xany.window << std::endl;
                    changesOccured = false;
                }
                break;
            }
        }

        if (changesOccured) {
            for (unsigned int i = 0; i < m_screens.size(); i++) {
                m_screens[i]->renderScreen();
            }

            std::cout << m_screens.size() << " screen(s) available." << std::endl;
            for (unsigned int i = 0; i < m_screens.size(); i++) {
                std::cout << *m_screens[i];
            }
            std::cout << "======================================" << std::endl;
            changesOccured = false;
        }
    }
}


//--- ERROR HANDLERS -----------------------------------------------------------

// Custom X error handler.
int FbCompositor::handleXError(Display *display, XErrorEvent *error) {
    static const int ERROR_TEXT_LENGTH = 128;

    char errorText[ERROR_TEXT_LENGTH];
    XGetErrorText(display, error->error_code, errorText, ERROR_TEXT_LENGTH);

    std::cerr << "X Error: " << errorText << " (errorCode=" << error->error_code
              << ", majorOpCode=" << error->request_code << ", minorOpCode="
              << error->minor_code << ", resourceId=" << std::hex << error->resourceid
              << std::dec << ")" << std::endl;
    return 0;
}
