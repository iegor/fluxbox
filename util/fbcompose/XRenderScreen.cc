/** XRenderScreen.cc file for the fluxbox compositor. */

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

#include "XRenderScreen.hh"

#ifdef USE_XRENDER_COMPOSITING


#include "CompositorConfig.hh"
#include "Logging.hh"
#include "XRenderWindow.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xutil.h>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
XRenderScreen::XRenderScreen(int screenNumber, const CompositorConfig &config) throw(InitException):
    BaseScreen(screenNumber, Plugin_XRender, config),
    m_pictFilter(config.xRenderPictFilter()) {

    initRenderingSurface();
    initBackgroundPicture();
}

// Destructor.
XRenderScreen::~XRenderScreen() throw() {
    if (m_backBufferGC) {
        XFreeGC(display(), m_backBufferGC);
    }
    if (m_backBufferPicture) {
        XRenderFreePicture(display(), m_backBufferPicture);
    }
    if (m_backBufferPixmap) {
        XFreePixmap(display(), m_backBufferPixmap);
    }
    if (m_renderingPicture) {
        XRenderFreePicture(display(), m_renderingPicture);
    }
    if (m_rootPicture) {
        XRenderFreePicture(display(), m_rootPicture);
    }

    XUnmapWindow(display(), m_renderingWindow);
    XDestroyWindow(display(), m_renderingWindow);
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Initializes the rendering surface.
void XRenderScreen::initRenderingSurface() throw(InitException) {
    // Get all the elements, needed for the creation of the rendering surface.
    Window compOverlay = XCompositeGetOverlayWindow(display(), rootWindow().window());

    XVisualInfo visualInfo;
    if (!XMatchVisualInfo(display(), screenNumber(), 32, TrueColor, &visualInfo)) {
        throw InitException("Cannot find the required visual.");
    }

    XSetWindowAttributes wa;
    wa.border_pixel = XBlackPixel(display(), screenNumber());   // Without this XCreateWindow gives BadMatch error.
    wa.colormap = XCreateColormap(display(), rootWindow().window(), visualInfo.visual, AllocNone);
    long waMask = CWBorderPixel | CWColormap;

    // Create the rendering surface.
    m_renderingWindow = XCreateWindow(display(), compOverlay, 0, 0, rootWindow().width(), rootWindow().height(), 0,
                                      visualInfo.depth, InputOutput, visualInfo.visual, waMask, &wa);
    XmbSetWMProperties(display(), m_renderingWindow, "fbcompose", "fbcompose", NULL, 0, NULL, NULL, NULL);
    XMapWindow(display(), m_renderingWindow);

    // Make sure the overlays do not consume any input events.
    XserverRegion emptyRegion = XFixesCreateRegion(display(), NULL, 0);
    XFixesSetWindowShapeRegion(display(), compOverlay, ShapeInput, 0, 0, emptyRegion);
    XFixesSetWindowShapeRegion(display(), m_renderingWindow, ShapeInput, 0, 0, emptyRegion);
    XFixesDestroyRegion(display(), emptyRegion);

    addWindowToIgnoreList(compOverlay);
    addWindowToIgnoreList(m_renderingWindow);

    // Create an XRender picture for the rendering window.
    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long paMask = CPSubwindowMode;

    m_renderingPictFormat = XRenderFindVisualFormat(display(), visualInfo.visual);  // Do not XFree.
    if (!m_renderingPictFormat) {
        throw InitException("Cannot find the required picture format.");
    }
    m_renderingPicture = XRenderCreatePicture(display(), m_renderingWindow, m_renderingPictFormat, paMask, &pa);

    // Create the back buffer.
    m_backBufferPictFormat = XRenderFindStandardFormat(display(), PictStandardARGB32);     // Do not XFree.
    m_backBufferPixmap = XCreatePixmap(display(), rootWindow().window(), rootWindow().width(), rootWindow().height(), 32);
    m_backBufferPicture = XRenderCreatePicture(display(), m_backBufferPixmap, m_backBufferPictFormat, paMask, &pa);
    m_backBufferGC = XCreateGC(display(), m_backBufferPixmap, 0, NULL);

    // Set picture filters.
    XRenderSetPictureFilter(display(), m_renderingPicture, m_pictFilter, NULL, 0);
    XRenderSetPictureFilter(display(), m_backBufferPicture, m_pictFilter, NULL, 0);
}

// Initializes background picture.
void XRenderScreen::initBackgroundPicture() throw() {
    m_rootChanged = false;
    m_rootPictFormat = XRenderFindVisualFormat(display(), rootWindow().visual());
    m_rootPicture = None;

    updateBackgroundPicture();
}


//--- SCREEN MANIPULATION ------------------------------------------------------

// Notifies the screen of a background change.
void XRenderScreen::setRootPixmapChanged() throw() {
    BaseScreen::setRootPixmapChanged();
    m_rootChanged = true;
}

// Notifies the screen of a root window change.
void XRenderScreen::setRootWindowSizeChanged() throw() {
    BaseScreen::setRootWindowSizeChanged();
    m_rootChanged = true;

    if (m_backBufferGC) {
        XFreeGC(display(), m_backBufferGC);
        m_backBufferGC = None;
    }
    if (m_backBufferPicture) {
        XRenderFreePicture(display(), m_backBufferPicture);
        m_backBufferPicture = None;
    }
    if (m_backBufferPixmap) {
        XFreePixmap(display(), m_backBufferPixmap);
        m_backBufferPixmap = None;
    }
    if (m_renderingPicture) {
        XRenderFreePicture(display(), m_renderingPicture);
        m_renderingPicture = None;
    }

    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long paMask = CPSubwindowMode;

    XResizeWindow(display(), m_renderingWindow, rootWindow().width(), rootWindow().height());
    m_renderingPicture = XRenderCreatePicture(display(), m_renderingWindow, m_renderingPictFormat, paMask, &pa);

    m_backBufferPixmap = XCreatePixmap(display(), rootWindow().window(), rootWindow().width(), rootWindow().height(), 32);
    m_backBufferPicture = XRenderCreatePicture(display(), m_backBufferPixmap, m_backBufferPictFormat, paMask, &pa);
    m_backBufferGC = XCreateGC(display(), m_backBufferPixmap, 0, NULL);

    XRenderSetPictureFilter(display(), m_renderingPicture, m_pictFilter, NULL, 0);
    XRenderSetPictureFilter(display(), m_backBufferPicture, m_pictFilter, NULL, 0);
}


// Update the background picture.
void XRenderScreen::updateBackgroundPicture() throw() {
    Pixmap bgPixmap = rootWindowPixmap();

    if (bgPixmap) {
        if (m_rootPicture) {
            XRenderFreePicture(display(), m_rootPicture);
            m_rootPicture = None;
        }

        XRenderPictureAttributes pa;
        pa.subwindow_mode = IncludeInferiors;
        long paMask = CPSubwindowMode;

        m_rootPicture = XRenderCreatePicture(display(), bgPixmap, m_rootPictFormat, paMask, &pa);
        m_rootChanged = false;
    } else {
        fbLog_warn << "Cannot create background texture (cannot find background pixmap)." << std::endl;
    }
}


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void XRenderScreen::renderScreen() throw() {
    renderBackground();

    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        if ((*it)->isMapped()) {
            renderWindow(*(dynamic_cast<XRenderWindow*>(*it)));
        }
        ++it;
    }

    if ((reconfigureRectangle().width != 0) && (reconfigureRectangle().height != 0)) {
        renderReconfigureRect();
    }

    swapBuffers();
}

// Render the desktop wallpaper.
void XRenderScreen::renderBackground() throw() {
    // TODO: Simply make the window transparent.
    if (m_rootChanged) {
        updateBackgroundPicture();
    }

    XRenderComposite(display(), PictOpSrc, m_rootPicture, None, m_backBufferPicture,
                     0, 0, 0, 0, 0, 0, rootWindow().width(), rootWindow().height());
}

// Render the reconfigure rectangle.
void XRenderScreen::renderReconfigureRect() throw() {
    XSetForeground(display(), m_backBufferGC, XWhitePixel(display(), screenNumber()));
    XSetFunction(display(), m_backBufferGC, GXxor);
    XSetLineAttributes(display(), m_backBufferGC, 1, LineSolid, CapNotLast, JoinMiter);

    XRectangle rect = reconfigureRectangle();
    XDrawRectangles(display(), m_backBufferPixmap, m_backBufferGC, &rect, 1);
}

// Render a particular window onto the screen.
void XRenderScreen::renderWindow(XRenderWindow &window) throw() {
    if (window.isDamaged()) {
        window.updateContents();
    }

    XRenderComposite(display(), PictOpOver, window.contentPicture(), window.maskPicture(), m_backBufferPicture,
                     0, 0, 0, 0, window.x(), window.y(), window.realWidth(), window.realHeight());
}

// Swap back and front buffers.
void XRenderScreen::swapBuffers() throw() {
    XRenderComposite(display(), PictOpSrc, m_backBufferPicture, None, m_renderingPicture,
                     0, 0, 0, 0, 0, 0, rootWindow().width(), rootWindow().height());
}


//--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS --------------------------------

// Creates a window object from its XID.
BaseCompWindow *XRenderScreen::createWindowObject(Window window) throw(InitException) {
    XRenderWindow *newWindow = new XRenderWindow(*this, window, m_pictFilter);
    return newWindow;
}

#endif // USE_XRENDER_COMPOSITING
