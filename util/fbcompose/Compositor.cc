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


#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif  // HAVE_CONFIG_H

#include "Compositor.hh"

#include "Atoms.hh"
#include "BaseScreen.hh"
#include "CompositorConfig.hh"
#include "Constants.hh"
#include "Logging.hh"
#include "OpenGLScreen.hh"
#include "XRenderScreen.hh"

#ifdef USE_OPENGL_COMPOSITING
    #include <GL/glxew.h>
    #include <GL/glx.h>
#endif  // USE_OPENGL_COMPOSITING

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>
#ifdef XINERAMA
    #include <X11/extensions/Xinerama.h>
#endif  // XINERAMA
#ifdef USE_XRENDER_COMPOSITING
    #include <X11/extensions/Xrender.h>
#endif  // USE_XRENDER_COMPOSITING
#include <X11/Xutil.h>

#include <sstream>

#ifdef HAVE_CTIME
    #include <ctime>
#else
#ifdef HAVE_TIME_H
    #include <time.h>
#endif
#endif

#include <csignal>

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

/** Length of the error buffer in X error handler. */
const int ERROR_BUFFER_LENGTH = 128;

/** Name of the relevant error database with X error messages in X error handler. */
const char ERROR_DB_TEXT_NAME[] = "XRequest";

/** Default name for unknown requests in X error handler. */
const char REQUEST_NAME_UNKNOWN_MESSAGE[] = "<UNKNOWN>";

/** How many microseconds to sleep before restarting the event loop. */
const int SLEEP_TIME = 5000;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// The constructor.
Compositor::Compositor(const CompositorConfig &config) :
    App(config.displayName().c_str()) {

    if (config.synchronize()) {
        XSynchronize(display(), True);
    }

    if (config.renderingMode() == RM_ServerAuto) {
        throw InitException("Compositor class does not provide the serverauto renderer.");
    }
    m_renderingMode = config.renderingMode();

    if (config.showXErrors()) {
        XSetErrorHandler(&printXError);
    } else {
        XSetErrorHandler(&ignoreXError);
    }

    initAllExtensions();

    int screenCount = XScreenCount(display());
    m_screens.reserve(screenCount);

    for (int i = 0; i < screenCount; i++) {
        Window cmSelectionOwner = getCMSelectionOwnership(i);

        switch (m_renderingMode) {
#ifdef USE_OPENGL_COMPOSITING
        case RM_OpenGL :
            m_screens.push_back(new OpenGLScreen(i, config));
            break;
#endif  // USE_OPENGL_COMPOSITING
#ifdef USE_XRENDER_COMPOSITING
        case RM_XRender :
            m_screens.push_back(new XRenderScreen(i, config));
            break;
#endif  // USE_XRENDER_COMPOSITING
        default :
            throw InitException("Unknown rendering mode selected.");
            break;
        }

        m_screens[i]->ignoreWindow(cmSelectionOwner);
    }

    initHeads();
    for (size_t i = 0; i < m_screens.size(); i++) {
        m_screens[i]->initPlugins(config);
        m_screens[i]->initWindows();
    }

    m_timer.setTickSize(1000000 / config.framesPerSecond());
    m_timer.start();

    XFlush(display());

    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
}

// Destructor.
Compositor::~Compositor() {
    std::vector<BaseScreen*>::iterator it = m_screens.begin();
    while (it != m_screens.end()) {
        delete *it;
        ++it;
    }
}


//--- INITIALIZATION FUNCTIONS -----------------------------------------

// Acquires the ownership of compositing manager selections.
Window Compositor::getCMSelectionOwnership(int screenNumber) {
    Atom cmAtom = Atoms::compositingSelectionAtom(screenNumber);

    Window curOwner = XGetSelectionOwner(display(), cmAtom);
    if (curOwner != None) {
        // TODO: More detailed message - what is the other program?
        throw InitException("Another compositing manager is running.");
    }

    curOwner = XCreateSimpleWindow(display(), XRootWindow(display(), screenNumber), -10, -10, 1, 1, 0, None, None);
    XmbSetWMProperties(display(), curOwner, APP_NAME, APP_NAME, NULL, 0, NULL, NULL, NULL);
    XSetSelectionOwner(display(), cmAtom, curOwner, CurrentTime);

    return curOwner;
}

// Initializes X's extensions.
void Compositor::initAllExtensions() {
#ifdef USE_OPENGL_COMPOSITING
    if (m_renderingMode == RM_OpenGL) {
        initExtension("GLX", &glXQueryExtension, &glXQueryVersion, 1, 3, &m_glxEventBase, &m_glxErrorBase);
        initExtension("XComposite", &XCompositeQueryExtension, &XCompositeQueryVersion, 0, 4, &m_compositeEventBase, &m_compositeErrorBase);
        initExtension("XDamage", &XDamageQueryExtension, &XDamageQueryVersion, 1, 0, &m_damageEventBase, &m_damageErrorBase);
        initExtension("XFixes", &XFixesQueryExtension, &XFixesQueryVersion, 2, 0, &m_fixesEventBase, &m_fixesErrorBase);
        initExtension("XShape", &XShapeQueryExtension, &XShapeQueryVersion, 1, 1, &m_shapeEventBase, &m_shapeErrorBase);
    } else
#endif  // USE_OPENGL_COMPOSITING

#ifdef USE_XRENDER_COMPOSITING
    if (m_renderingMode == RM_XRender) {
        initExtension("XComposite", &XCompositeQueryExtension, &XCompositeQueryVersion, 0, 4, &m_compositeEventBase, &m_compositeErrorBase);
        initExtension("XDamage", &XDamageQueryExtension, &XDamageQueryVersion, 1, 0, &m_damageEventBase, &m_damageErrorBase);
        initExtension("XFixes", &XFixesQueryExtension, &XFixesQueryVersion, 2, 0, &m_fixesEventBase, &m_fixesErrorBase);
        initExtension("XRender", &XRenderQueryExtension, &XRenderQueryVersion, 0, 1, &m_renderEventBase, &m_renderErrorBase);
        initExtension("XShape", &XShapeQueryExtension, &XShapeQueryVersion, 1, 1, &m_shapeEventBase, &m_shapeErrorBase);
    } else
#endif  // USE_XRENDER_COMPOSITING

    { }
}

// Initializes a particular X server extension.
void Compositor::initExtension(const char *extensionName, QueryExtensionFunction extensionFunc,
                               QueryVersionFunction versionFunc, const int minMajorVer, const int minMinorVer,
                               int *eventBase, int *errorBase) {
    int majorVer;
    int minorVer;

    // Check that the extension exists.
    if (!(*extensionFunc)(display(), eventBase, errorBase)) {
        *eventBase = -1;
        *errorBase = -1;

        std::stringstream ss;
        ss << extensionName << " extension not found.";
        throw InitException(ss.str());
    }

    // Get extension version.
    if (!(*versionFunc)(display(), &majorVer, &minorVer)) {
        *eventBase = -1;
        *errorBase = -1;

        std::stringstream ss;
        ss << "Could not query the version of " << extensionName << " extension.";
        throw InitException(ss.str());
    }

    // Make sure the extension version is at least what we require.
    if ((majorVer < minMajorVer) || ((majorVer == minMajorVer) && (minorVer < minMinorVer))) {
        *eventBase = -1;
        *errorBase = -1;

        std::stringstream ss;
        ss << "Unsupported " << extensionName << " extension version found (required >=" << minMajorVer
           << "." << minMinorVer << ", got " << majorVer << "." << minorVer << ").";
        throw InitException(ss.str());
    }
}

// Initializes monitor heads on every screen.
void Compositor::initHeads() {
    HeadMode headMode = Heads_One;

#ifdef XINERAMA
    try {
        initExtension("Xinerama", &XineramaQueryExtension, &XCompositeQueryVersion, 0, 0, &m_xineramaEventBase, &m_xineramaErrorBase);
        if (XineramaIsActive(display())) {
            headMode = Heads_Xinerama;
        }
    } catch (const InitException &e) { }

    if (headMode != Heads_Xinerama) {
        fbLog_warn << "Could not initialize Xinerama." << std::endl;
    }
#endif  // XINERAMA

    for (size_t i = 0; i < m_screens.size(); i++) {
        m_screens[i]->updateHeads(headMode);
    }
}


//--- EVENT LOOP ---------------------------------------------------------------

// The event loop.
void Compositor::eventLoop() {
    union {
        XEvent eventX;
        XDamageNotifyEvent eventXDamageNotify;
        XShapeEvent eventXShape;
    } eventUnion;
    XEvent &event = eventUnion.eventX;

    int eventScreen;
    timespec sleepTimespec = { 0, SLEEP_TIME * 1000 };

    while (!done()) {
        while (XPending(display())) {
            XNextEvent(display(), &event);

            eventScreen = screenOfEvent(event);
            if (eventScreen < 0) {
                fbLog_info << "Event " << std::dec << event.xany.serial << " (window " << std::hex << event.xany.window
                           << ", type " << std::dec << event.xany.type << ") does not affect any managed windows, skipping."
                           << std::endl;
                continue;
            }

            switch (event.type) {
            case CirculateNotify :
                m_screens[eventScreen]->circulateWindow(event.xcirculate.window, event.xcirculate.place);
                fbLog_debug << "CirculateNotify on " << std::hex << event.xcirculate.window << std::endl;
                break;

            case ConfigureNotify :
                m_screens[eventScreen]->reconfigureWindow(event.xconfigure);
                fbLog_debug << "ConfigureNotify on " << std::hex << event.xconfigure.window << std::endl;
                break;

            case CreateNotify :
                m_screens[eventScreen]->createWindow(event.xcreatewindow.window);
                fbLog_debug << "CreateNotify on " << std::hex << event.xcreatewindow.window << std::endl;
                break;

            case DestroyNotify :
                m_screens[eventScreen]->destroyWindow(event.xdestroywindow.window);
                fbLog_debug << "DestroyNotify on " << std::hex << event.xdestroywindow.window << std::endl;
                break;

            case Expose :
                m_screens[eventScreen]->damageWindow(event.xexpose.window, getExposedRect(event.xexpose));
                fbLog_debug << "Expose on " << std::hex << event.xexpose.window << std::endl;
                break;

            case GravityNotify :
                fbLog_debug << "GravityNotify on " << std::hex << event.xgravity.window << std::endl;
                break;

            case MapNotify :
                m_screens[eventScreen]->mapWindow(event.xmap.window);
                fbLog_debug << "MapNotify on " << std::hex << event.xmap.window << std::endl;
                break;

            case PropertyNotify :
                m_screens[eventScreen]->updateWindowProperty(event.xproperty.window, event.xproperty.atom, event.xproperty.state);
                fbLog_debug << "PropertyNotify on " << std::hex << event.xproperty.window << " ("
                           << XGetAtomName(display(), event.xproperty.atom) << ")" << std::endl;
                break;

            case ReparentNotify :
                m_screens[eventScreen]->reparentWindow(event.xreparent.window, event.xreparent.parent);
                fbLog_debug << "ReparentNotify on " << std::hex << event.xreparent.window << " (parent "
                           << event.xreparent.parent << ")" << std::endl;
                break;

            case UnmapNotify :
                m_screens[eventScreen]->unmapWindow(event.xunmap.window);
                fbLog_debug << "UnmapNotify on " << std::hex << event.xunmap.window << std::endl;
                break;

            default :
                if (event.type == (m_damageEventBase + XDamageNotify)) {
                    XDamageNotifyEvent damageEvent = eventUnion.eventXDamageNotify;
                    m_screens[eventScreen]->damageWindow(damageEvent.drawable, damageEvent.area);
                    fbLog_debug << "DamageNotify on " << std::hex << damageEvent.drawable << std::endl;

                } else if (event.type == (m_shapeEventBase + ShapeNotify)) {
                    XShapeEvent shapeEvent = eventUnion.eventXShape;
                    m_screens[eventScreen]->updateShape(shapeEvent.window);
                    fbLog_debug << "ShapeNotify on " << std::hex << shapeEvent.window << std::endl;

                } else {
                    fbLog_info << "Other event " << std::dec << event.xany.type << " received on screen "
                               << eventScreen << " and window " << std::hex << event.xany.window << std::endl;
                }
                break;
            }
        }

        if (m_timer.newElapsedTicks()) {
            for (size_t i = 0; i < m_screens.size(); i++) {
                m_screens[i]->renderScreen();
                m_screens[i]->clearScreenDamage();
            }
            XSync(display(), False);

            fbLog_debugDump << m_screens.size() << " screen(s) available." << std::endl;
            for (size_t i = 0; i < m_screens.size(); i++) {
                fbLog_debugDump << *m_screens[i];
            }
            fbLog_debugDump << "======================================" << std::endl;
        } else {
            nanosleep(&sleepTimespec, NULL);
        }
    }
}


//--- INTERNAL FUNCTIONS -----------------------------------------------

// Returns the exposed area in a XExposeEvent as an XRectangle.
XRectangle Compositor::getExposedRect(const XExposeEvent &event) {
    XRectangle rect = { event.x, event.y, event.width, event.height };
    return rect;
}

// Locates the screen an event affects. Returns -1 on failure.
int Compositor::screenOfEvent(const XEvent &event) {
    if (m_screens.size() == 1) {
        return 0;
    } else {
        for (size_t i = 0; i < m_screens.size(); i++) {
            if ((event.xany.window == m_screens[i]->rootWindow().window())
                    || (m_screens[i]->isWindowManaged(event.xany.window))) {
                return i;
            }
        }
    }

    return -1;
}


//--- VARIOUS HANDLERS ---------------------------------------------------------

// Custom signal handler.
void FbCompositor::handleSignal(int sig) {
    if ((sig == SIGINT) || (sig == SIGTERM)) {
        FbTk::App::instance()->end();
    }
}


// Custom X error handler (ignore).
int FbCompositor::ignoreXError(Display * /*display*/, XErrorEvent * /*error*/) {
    return 0;
}

// Custom X error handler (print, continue).
int FbCompositor::printXError(Display *display, XErrorEvent *error) {
    static std::stringstream ss;
    ss.str("");

    char errorText[ERROR_BUFFER_LENGTH];
    XGetErrorText(display, error->error_code, errorText, ERROR_BUFFER_LENGTH);

    ss << int(error->request_code);

    char requestName[ERROR_BUFFER_LENGTH];
    XGetErrorDatabaseText(display, (char*)(ERROR_DB_TEXT_NAME), (char*)(ss.str().c_str()),
                          (char*)(REQUEST_NAME_UNKNOWN_MESSAGE), requestName, ERROR_BUFFER_LENGTH);

    fbLog_warn << "X Error: " << errorText << " in " << requestName << " request, errorCode="
               << std::dec << int(error->error_code) << ", majorOpCode=" << int(error->request_code)
               << ", minorOpCode=" << int(error->minor_code) << ", resourceId=" << std::hex
               << error->resourceid << "." << std::endl;

    return 0;
}
