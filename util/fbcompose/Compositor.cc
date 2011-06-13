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

#include "FbTk/RefCount.hh"

#include <GL/glx.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xutil.h>

#include <iostream>
#include <sstream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// The constructor.
Compositor::Compositor(const CompositorConfig &config) throw(InitException) :
    App(config.displayName().c_str()),
    m_redrawTimer() {

    m_renderingMode = config.renderingMode();

    XSynchronize(display(), True);
    XSetErrorHandler(&handleXError);

    initAllExtensions();

    // Set up screens.
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

    // Set up windows.
    if (m_renderingMode != RM_ServerAuto) {
        for (size_t i = 0; i < m_screens.size(); i++) {
            m_screens[i]->initWindows();
        }
    }

    XFlush(display());

    FbTk::RefCount<FbTk::Command<void> > command(new RenderScreensCommand(this));
    m_redrawTimer.setCommand(command);
    m_redrawTimer.setTimeout(0, 1000000 / 60);
    m_redrawTimer.start();
}

// Destructor.
Compositor::~Compositor() { }


//--- INITIALIZATION FUNCTIONS -----------------------------------------

// Acquires the ownership of compositing manager selections.
void Compositor::getCMSelectionOwnership(int screenNumber) throw(InitException) {
    std::stringstream ss;
    ss << "_NET_WM_CM_S" << screenNumber;
    Atom cmAtom = XInternAtom(display(), ss.str().c_str(), False);

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
    } else if (m_renderingMode == RM_ServerAuto) {
        initExtension("XComposite", &XCompositeQueryExtension, &XCompositeQueryVersion, 0, 1, &m_compositeEventBase, &m_compositeErrorBase);
    }
}

// Initializes a particular X server extension.
void Compositor::initExtension(const char *extensionName, QueryExtensionFunction extensionFunc,
                               QueryVersionFunction versionFunc, const int minMajorVer, const int minMinorVer,
                               int *eventBase, int *errorBase) throw(InitException) {
    int majorVer;
    int minorVer;

    if (!(*extensionFunc)(display(), eventBase, errorBase)) {
        std::stringstream ss;
        ss << extensionName << " extension is not present.";
        throw InitException(ss.str());
    }

    if (!(*versionFunc)(display(), &majorVer, &minorVer)) {
        std::stringstream ss;
        ss << "Could not query the version of " << extensionName << " extension.";
        throw InitException(ss.str());
    }

    if ((majorVer < minMajorVer) || ((majorVer == minMajorVer) && (minorVer < minMinorVer))) {
        std::stringstream ss;
        ss << "Unsupported " << extensionName << " extension version (required >=" << minMajorVer
           << "." << minMinorVer << ", got " << majorVer << "." << minorVer << ").";
        throw InitException(ss.str());
    }
}

//--- EVENT LOOP ---------------------------------------------------------------

// The event loop.
void Compositor::eventLoop() {
    XEvent event;

    while (!done()) {
        if (m_renderingMode == RM_ServerAuto) {
            continue;
        }

        XFlush(display());
        while (XPending(display())) {
            XNextEvent(display(), &event);

            int eventScreen = screenOfEvent(event);
            if (eventScreen < 0) {
                fbLog_info << "Event " << event.xany.serial << " does not affect any managed windows, skipping." << std::endl;
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
                if (event.xreparent.parent == m_screens[eventScreen]->rootWindow().window()) {
                    m_screens[eventScreen]->createWindow(event.xreparent.window);
                } else {
                    m_screens[eventScreen]->destroyWindow(event.xreparent.window);
                }
                fbLog_info << "ReparentNotify on " << std::hex << event.xreparent.window << " (parent "
                           << event.xreparent.parent << ")" << std::endl;
                break;
            case UnmapNotify :
                m_screens[eventScreen]->unmapWindow(event.xunmap.window);
                fbLog_info << "UnmapNotify on " << std::hex << event.xunmap.window << std::endl;
                break;
            default :
                if (event.type == (m_damageEventBase + XDamageNotify)) {
                    XDamageNotifyEvent damageEvent = *((XDamageNotifyEvent*) &event);   // TODO: Better cast.
                    m_screens[eventScreen]->damageWindow(damageEvent.drawable, damageEvent.area);
                    fbLog_info << "DamageNotify on " << std::hex << damageEvent.drawable << std::endl;
                } else if (event.type == (m_shapeEventBase + ShapeNotify)) {
                    XShapeEvent shapeEvent = *((XShapeEvent*) &event);      // TODO: Better cast.
                    m_screens[eventScreen]->updateShape(shapeEvent.window);
                    fbLog_info << "ShapeNotify on " << std::hex << shapeEvent.window << std::endl;
                } else {
                    fbLog_info << "Event " << event.xany.type << " on screen " << eventScreen
                               << " and window " << event.xany.window << std::endl;
                }
                break;
            }
        }

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


//--- ERROR HANDLERS -----------------------------------------------------------

// Custom X error handler.
int FbCompositor::handleXError(Display *display, XErrorEvent *error) {
    static const int ERROR_TEXT_LENGTH = 128;

    char errorText[ERROR_TEXT_LENGTH];
    XGetErrorText(display, error->error_code, errorText, ERROR_TEXT_LENGTH);

    fbLog_warn << "X Error: " << errorText << " (errorCode=" << int(error->error_code)
               << ", majorOpCode=" << int(error->request_code) << ", minorOpCode="
               << int(error->minor_code) << ", resourceId=" << std::hex << error->resourceid
               << std::dec << ")" << std::endl;
    return 0;
}
