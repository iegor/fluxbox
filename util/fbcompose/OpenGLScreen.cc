/** OpenGLScreen.cc file for the fluxbox compositor. */

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


#include "OpenGLScreen.hh"
#include "OpenGLWindow.hh"

#include <X11/extensions/Xcomposite.h>

#include <list>
#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLScreen::OpenGLScreen(int screenNumber) :
    BaseScreen(screenNumber) {

    initRenderingSurface();
    getTopLevelWindows();
}

// Destructor.
OpenGLScreen::~OpenGLScreen() { }


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Read and stores all top level windows.
void OpenGLScreen::getTopLevelWindows() {
    Window root;
    Window parent;
    Window* children;
    unsigned int childCount;

    XQueryTree(display(), rootWindow().window(), &root, &parent, &children, &childCount);

    for (unsigned int i = 0; i < childCount; i++) {
        createWindow(children[i]);
    }
    if (children) {
        XFree(children);
    }
}

// Initializes the rendering surface.
void OpenGLScreen::initRenderingSurface() {
    // TODO: Better context creation (GL 3.0 etc).
    // TODO: Better failure handling with FBConfigs.

    // Creating arrays of FBConfig attributes.
    static const int PREFERRED_FBCONFIG_ATTRIBUTES[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER, true,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        None
    };

    // Selecting the FBConfig.
    int nConfigs;
    GLXFBConfig *fbConfigs = glXChooseFBConfig(display(), screenNumber(),
                                               PREFERRED_FBCONFIG_ATTRIBUTES, &nConfigs);
    if (!fbConfigs) {
        throw ConfigException("Screen does not support the required GLXFBConfig.");
    }
    m_fbConfig = fbConfigs[0];

    // Creating an X window for rendering.
    Window compOverlay = XCompositeGetOverlayWindow(display(), rootWindow().window());

    XVisualInfo *mainVisual = glXGetVisualFromFBConfig(display(), m_fbConfig);
    Colormap mainColormap = XCreateColormap(display(), rootWindow().window(), mainVisual->visual, AllocNone);

    XSetWindowAttributes wa;
    wa.colormap = mainColormap;
    long waMask = CWColormap;

    m_renderingWindow = XCreateWindow(display(), compOverlay, 0, 0, rootWindow().width(), rootWindow().height(), 0,
                                      mainVisual->depth, InputOutput, mainVisual->visual, waMask, &wa);
    XmbSetWMProperties(display(), m_renderingWindow, "fbcompose", "fbcompose", NULL, 0, NULL, NULL, NULL);
    XMapWindow(display(), m_renderingWindow);

    // Creating a GLX handle for the above window.
    m_glxRenderingWindow = glXCreateWindow(display(), m_fbConfig, m_renderingWindow, NULL);
    if (!m_glxRenderingWindow) {
        throw ConfigException("Cannot create the rendering surface.");
    }

    // Creating the GLX rendering context.
    m_glxContext = glXCreateNewContext(display(), m_fbConfig, GLX_RGBA_TYPE, NULL, false);
    if (!m_glxContext) {
        throw ConfigException("Cannot create the rendering context.");
    }
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Creates a window object from its XID.
BaseCompWindow *OpenGLScreen::createWindowObject(Window window) {
    OpenGLWindow *newWindow = new OpenGLWindow(window);
    return newWindow;
}

// Cleans up a window object before it is deleted.
void OpenGLScreen::cleanupWindowObject(BaseCompWindow &window) { }

// Damages a window object.
void OpenGLScreen::damageWindowObject(BaseCompWindow &window) {
    window.setDamaged();
}

// Maps a window object.
void OpenGLScreen::mapWindowObject(BaseCompWindow &window) {
    window.setMapped();
}

// Updates window's configuration.
void OpenGLScreen::reconfigureWindowObject(BaseCompWindow &window) {
    window.updateGeometry();
}

// Unmaps a window object.
void OpenGLScreen::unmapWindowObject(BaseCompWindow &window) {
    window.setUnmapped();
}

// Updates the value of some window's property.
void OpenGLScreen::updateWindowObjectProperty(BaseCompWindow &window, Atom property, int state) { }


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void OpenGLScreen::renderScreen() {
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);

    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while(it != allWindows().end()) {
        it++;
    }

    glXSwapBuffers (display(), m_glxRenderingWindow);
}
