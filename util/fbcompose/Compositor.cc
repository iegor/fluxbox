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
#include "Logging.hh"
#include "OpenGLScreen.hh"
#include "XRenderScreen.hh"

#include "FbTk/RefCount.hh"

#include <GL/glx.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xutil.h>

#include <iostream>
#include <sstream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// The constructor.
Compositor::Compositor(const CompositorConfig &config) throw(InitException) :
    App(config.displayName().c_str()),
    m_redrawTimer() {

    XSynchronize(display(), True);

    if (config.renderingMode() == RM_ServerAuto) {
        throw InitException("Compositor class does not provide the serverauto renderer.");
    }
    m_renderingMode = config.renderingMode();

    XSetErrorHandler(&handleXError);
    initAllExtensions();

    int screenCount = XScreenCount(display());
    m_screens.reserve(screenCount);
    for (int i = 0; i < screenCount; i++) {
        switch (m_renderingMode) {
        case RM_OpenGL :
            m_screens.push_back(new OpenGLScreen(i));
            break;
        case RM_XRender :
            m_screens.push_back(new XRenderScreen(i));
            break;
        default :
            throw InitException("Unknown rendering mode selected.");
            break;
        }

        getCMSelectionOwnership(i);
    }

    initXinerama();
    for (size_t i = 0; i < m_screens.size(); i++) {
        m_screens[i]->initWindows();
    }

    FbTk::RefCount<FbTk::Command<void> > command(new RenderScreensCommand(this));
    m_redrawTimer.setCommand(command);
    m_redrawTimer.setTimeout(0, 1000000.0 / config.framesPerSecond());
    m_redrawTimer.start();

    XFlush(display());
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
void Compositor::getCMSelectionOwnership(int screenNumber) throw(InitException) {
    Atom cmAtom = Atoms::compositingSelectionAtom(screenNumber);

    Window curOwner = XGetSelectionOwner(display(), cmAtom);
    if (curOwner != None) {
        // TODO: More detailed message - what is the other program?
        throw InitException("Another compositing manager is running.");
    }

    // TODO: Better way of obtaining program's name in SetWMProperties.
    curOwner = XCreateSimpleWindow(display(), XRootWindow(display(), screenNumber), -10, -10, 1, 1, 0, None, None);
    XmbSetWMProperties(display(), curOwner, "fbcompose", "fbcompose", NULL, 0, NULL, NULL, NULL);
    XSetSelectionOwner(display(), cmAtom, curOwner, CurrentTime);

    m_screens[screenNumber]->addWindowToIgnoreList(curOwner);
}

// Initializes X's extensions.
void Compositor::initAllExtensions() throw(InitException) {
    if (m_renderingMode == RM_OpenGL) {
        initExtension("GLX", &glXQueryExtension, &glXQueryVersion, 1, 4, &m_glxEventBase, &m_glxErrorBase);
        initExtension("XComposite", &XCompositeQueryExtension, &XCompositeQueryVersion, 0, 4, &m_compositeEventBase, &m_compositeErrorBase);
        initExtension("XDamage", &XDamageQueryExtension, &XDamageQueryVersion, 1, 0, &m_damageEventBase, &m_damageErrorBase);
        initExtension("XFixes", &XFixesQueryExtension, &XFixesQueryVersion, 2, 0, &m_fixesEventBase, &m_fixesErrorBase);
        initExtension("XShape", &XShapeQueryExtension, &XShapeQueryVersion, 1, 1, &m_shapeEventBase, &m_shapeErrorBase);
    } else if (m_renderingMode == RM_XRender) {
        initExtension("XComposite", &XCompositeQueryExtension, &XCompositeQueryVersion, 0, 4, &m_compositeEventBase, &m_compositeErrorBase);
        initExtension("XDamage", &XDamageQueryExtension, &XDamageQueryVersion, 1, 0, &m_damageEventBase, &m_damageErrorBase);
        initExtension("XFixes", &XFixesQueryExtension, &XFixesQueryVersion, 2, 0, &m_fixesEventBase, &m_fixesErrorBase);
        initExtension("XRender", &XRenderQueryExtension, &XRenderQueryVersion, 0, 1, &m_renderEventBase, &m_renderErrorBase);
        initExtension("XShape", &XShapeQueryExtension, &XShapeQueryVersion, 1, 1, &m_shapeEventBase, &m_shapeErrorBase);
    }
}

// Initializes a particular X server extension.
void Compositor::initExtension(const char *extensionName, QueryExtensionFunction extensionFunc,
                               QueryVersionFunction versionFunc, const int minMajorVer, const int minMinorVer,
                               int *eventBase, int *errorBase) throw(InitException) {
    int majorVer;
    int minorVer;

    if (!(*extensionFunc)(display(), eventBase, errorBase)) {
        *eventBase = -1;
        *errorBase = -1;

        std::stringstream ss;
        ss << extensionName << " extension not found.";
        throw InitException(ss.str());
    }

    if (!(*versionFunc)(display(), &majorVer, &minorVer)) {
        *eventBase = -1;
        *errorBase = -1;

        std::stringstream ss;
        ss << "Could not query the version of " << extensionName << " extension.";
        throw InitException(ss.str());
    }

    if ((majorVer < minMajorVer) || ((majorVer == minMajorVer) && (minorVer < minMinorVer))) {
        *eventBase = -1;
        *errorBase = -1;

        std::stringstream ss;
        ss << "Unsupported " << extensionName << " extension version (required >=" << minMajorVer
           << "." << minMinorVer << ", got " << majorVer << "." << minorVer << ").";
        throw InitException(ss.str());
    }
}

// Initializes Xinerama.
void Compositor::initXinerama() throw() {
    HeadMode headMode = Heads_One;
    try {
        initExtension("Xinerama", &XineramaQueryExtension, &XCompositeQueryVersion, 0, 0, &m_xineramaEventBase, &m_xineramaErrorBase);
        if (XineramaIsActive(display())) {
            headMode = Heads_Xinerama;
        }
    } catch (const InitException &e) { }

    for (size_t i = 0; i < m_screens.size(); i++) {
        m_screens[i]->initHeads(headMode);
    }
}


//--- EVENT LOOP ---------------------------------------------------------------

// The event loop.
void Compositor::eventLoop() {
    XEvent event;

    while (!done()) {
        while (XPending(display())) {
            XNextEvent(display(), &event);

            int eventScreen = screenOfEvent(event);
            if (eventScreen < 0) {
                fbLog_info << "Event " << std::dec << event.xany.serial << " (window " << std::hex << event.xany.window
                           << ", type " << std::dec << event.xany.type << ") does not affect any managed windows, skipping."
                           << std::endl;
                continue;
            }

            switch (event.type) {
            case ConfigureNotify :
                m_screens[eventScreen]->reconfigureWindow(event.xconfigure);
                fbLog_info << "ConfigureNotify on " << std::hex << event.xconfigure.window << std::endl;
                break;
            case CreateNotify :
                m_screens[eventScreen]->createWindow(event.xcreatewindow.window);
                fbLog_info << "CreateNotify on " << std::hex << event.xcreatewindow.window << std::endl;
                break;
            case DestroyNotify :
                m_screens[eventScreen]->destroyWindow(event.xdestroywindow.window);
                fbLog_info << "DestroyNotify on " << std::hex << event.xdestroywindow.window << std::endl;
                break;
            case MapNotify :
                m_screens[eventScreen]->mapWindow(event.xmap.window);
                fbLog_info << "MapNotify on " << std::hex << event.xmap.window << std::endl;
                break;
            case PropertyNotify :
                m_screens[eventScreen]->updateWindowProperty(event.xproperty.window, event.xproperty.atom, event.xproperty.state);
                fbLog_info << "PropertyNotify on " << std::hex << event.xproperty.window << " ("
                           << XGetAtomName(display(), event.xproperty.atom) << ")" << std::endl;
                break;
            case ReparentNotify :
                m_screens[eventScreen]->reparentWindow(event.xreparent.window, event.xreparent.parent);
                fbLog_info << "ReparentNotify on " << std::hex << event.xreparent.window << " (parent "
                           << event.xreparent.parent << ")" << std::endl;
                break;
            case UnmapNotify :
                m_screens[eventScreen]->unmapWindow(event.xunmap.window);
                fbLog_info << "UnmapNotify on " << std::hex << event.xunmap.window << std::endl;
                break;
            default :
                if (event.type == (m_damageEventBase + XDamageNotify)) {
                    XDamageNotifyEvent damageEvent = *((XDamageNotifyEvent*) &event);
                    m_screens[eventScreen]->damageWindow(damageEvent.drawable);
                    fbLog_info << "DamageNotify on " << std::hex << damageEvent.drawable << std::endl;
                } else if (event.type == (m_shapeEventBase + ShapeNotify)) {
                    XShapeEvent shapeEvent = *((XShapeEvent*) &event);
                    m_screens[eventScreen]->updateShape(shapeEvent.window);
                    fbLog_info << "ShapeNotify on " << std::hex << shapeEvent.window << std::endl;
                } else {
                    fbLog_info << "Event " << std::dec << event.xany.type << " on screen " << eventScreen
                               << " and window " << std::hex << event.xany.window << std::endl;
                }
                break;
            }
        }

        // This calls the m_redrawTimer, which actually draws the screens.
        FbTk::Timer::updateTimers(XConnectionNumber(display()));

        fbLog_debug << m_screens.size() << " screen(s) available." << std::endl;
        for (size_t i = 0; i < m_screens.size(); i++) {
            fbLog_debug << *m_screens[i];
        }
        fbLog_debug << "======================================" << std::endl;
    }
}


//--- INTERNAL FUNCTIONS -----------------------------------------------

// Render the screens.
void Compositor::renderScreens() {
    for (size_t i = 0; i < m_screens.size(); i++) {
        m_screens[i]->renderScreen();
    }
}

// Locates the screen an event affects. Returns -1 on failure.
int Compositor::screenOfEvent(const XEvent &event) {
    for (size_t i = 0; i < m_screens.size(); i++) {
        if (event.xany.window == m_screens[i]->rootWindow().window()) {
            return i;
        }
    }

    for (size_t i = 0; i < m_screens.size(); i++) {
        if (m_screens[i]->isWindowManaged(event.xany.window)) {
            return i;
        }
    }

    return -1;
}


//--- VARIOUS HANDLERS ---------------------------------------------------------

// Custom X error handler.
int FbCompositor::handleXError(Display *display, XErrorEvent *error) {
    static const int ERROR_TEXT_LENGTH = 128;

    char errorText[ERROR_TEXT_LENGTH];
    XGetErrorText(display, error->error_code, errorText, ERROR_TEXT_LENGTH);

    fbLog_warn << "X Error: " << errorText << " (errorCode=" << std::dec << int(error->error_code)
               << ", majorOpCode=" << int(error->request_code) << ", minorOpCode="
               << int(error->minor_code) << ", resourceId=" << std::hex << error->resourceid
               << std::dec << ")" << std::endl;
    return 0;
}
