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

#include "CompositorConfig.hh"
#include "Logging.hh"
#include "Utility.hh"
#include "XRenderPlugin.hh"
#include "XRenderWindow.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xutil.h>

using namespace FbCompositor;


//--- MACROS -------------------------------------------------------------------

// Macro for plugin iteration.
#define forEachPlugin(i, plugin)                                                        \
    (plugin) = ((pluginManager().plugins().size() > 0)                                  \
                   ? (dynamic_cast<XRenderPlugin*>(pluginManager().plugins()[0]))       \
                   : NULL);                                                             \
    for(size_t (i) = 0;                                                                 \
        ((i) < pluginManager().plugins().size());                                       \
        (i)++,                                                                          \
        (plugin) = (((i) < pluginManager().plugins().size())                            \
                       ? (dynamic_cast<XRenderPlugin*>(pluginManager().plugins()[(i)])) \
                       : NULL))


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
XRenderScreen::XRenderScreen(int screenNumber, const CompositorConfig &config):
    BaseScreen(screenNumber, Plugin_XRender, config),
    m_pictFilter(config.xRenderPictFilter()) {

    initRenderingSurface();
    updateBackgroundPicture();
}

// Destructor.
XRenderScreen::~XRenderScreen() {
    XUnmapWindow(display(), m_renderingWindow);
    XDestroyWindow(display(), m_renderingWindow);
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Initializes the rendering surface.
void XRenderScreen::initRenderingSurface() {
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

    XRenderPictFormat *renderingPictFormat = XRenderFindVisualFormat(display(), visualInfo.visual);
    if (!renderingPictFormat) {
        throw InitException("Cannot find the required picture format.");
    }

    m_renderingPicture = new XRenderPicture(*this, renderingPictFormat, m_pictFilter);
    m_renderingPicture->setWindow(m_renderingWindow, pa, paMask);

    // Create the back buffer.
    XRenderPictFormat *backBufferPictFormat = XRenderFindStandardFormat(display(), PictStandardARGB32);
    Pixmap backBufferPixmap = XCreatePixmap(display(), rootWindow().window(), rootWindow().width(), rootWindow().height(), 32);

    m_backBufferPicture = new XRenderPicture(*this, backBufferPictFormat, m_pictFilter);
    m_backBufferPicture->setPixmap(backBufferPixmap, true, pa, paMask);
}


//--- SCREEN MANIPULATION ------------------------------------------------------

// Notifies the screen of a background change.
void XRenderScreen::setRootPixmapChanged() {
    BaseScreen::setRootPixmapChanged();
    m_rootChanged = true;
}

// Notifies the screen of a root window change.
void XRenderScreen::setRootWindowSizeChanged() {
    BaseScreen::setRootWindowSizeChanged();
    m_rootChanged = true;

    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long paMask = CPSubwindowMode;

    XResizeWindow(display(), m_renderingWindow, rootWindow().width(), rootWindow().height());
    m_renderingPicture->setWindow(m_renderingWindow, pa, paMask);   // We need to recreate the picture.

    Pixmap backBufferPixmap = XCreatePixmap(display(), rootWindow().window(), rootWindow().width(), rootWindow().height(), 32);
    m_backBufferPicture->setPixmap(backBufferPixmap, true, pa, paMask);
}


// Update the background picture.
void XRenderScreen::updateBackgroundPicture() {
    XRenderPictFormat *pictFormat;
    if (wmSetRootWindowPixmap()) {
        pictFormat = XRenderFindVisualFormat(display(), rootWindow().visual());
    } else {
        pictFormat = XRenderFindStandardFormat(display(), PictStandardARGB32);
    }

    if (!pictFormat) {
        throw RuntimeException("Cannot find the required picture format.");
    }

    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long paMask = CPSubwindowMode;

    if (!m_rootPicture) {
        m_rootPicture = new XRenderPicture(*this, pictFormat, m_pictFilter);
    } else {
        m_rootPicture->setPictFormat(pictFormat);
    }
    m_rootPicture->setPixmap(rootWindowPixmap(), false, pa, paMask);
    m_rootChanged = false;
}


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void XRenderScreen::renderScreen() {
    // Render desktop background.
    renderBackground();

    // Render windows.
    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        if ((*it)->isRenderable() && (*it)->isMapped()) {
            renderWindow(*(dynamic_cast<XRenderWindow*>(*it)));
        }
        ++it;
    }

    // Render reconfigure rectangle.
    if ((reconfigureRectangle().width != 0) && (reconfigureRectangle().height != 0)) {
        renderReconfigureRect();
    }
    
    // Perform any extra rendering jobs from plugins.
    renderExtraJobs();

    // Swap buffers.
    swapBuffers();
}

// Render the desktop wallpaper.
void XRenderScreen::renderBackground() {
    // TODO: Simply make the window transparent.
    XRenderPlugin *plugin = NULL;

    // React to desktop background change.
    if (m_rootChanged) {
        updateBackgroundPicture();
    }

    // Draw the desktop.
    XRenderComposite(display(), PictOpSrc, m_rootPicture->pictureHandle(), None, m_backBufferPicture->pictureHandle(),
                     0, 0, 0, 0, 0, 0, rootWindow().width(), rootWindow().height());

    // Additional rendering of desktop.
    Picture srcPic, maskPic;
    int op, srcX, srcY, maskX, maskY, destX, destY, width, height;

    forEachPlugin(i, plugin) {
        plugin->extraBackgroundRenderingJob(op, srcPic, srcX, srcY, maskPic, maskX, maskY, destX, destY, width, height);
        if (op != PictOpClear) {
            XRenderComposite(display(), op, srcPic, maskPic, m_backBufferPicture->pictureHandle(),
                             srcX, srcY, maskX, maskY, destX, destY, width, height);
        }
    }
}

// Perform extra rendering jobs from plugins.
void XRenderScreen::renderExtraJobs() {
    XRenderPlugin *plugin = NULL;

    Picture srcPic, maskPic;
    int op, srcX, srcY, maskX, maskY, destX, destY, width, height;

    forEachPlugin(i, plugin) {
        plugin->preExtraRenderingActions();

        for (int j = 0; j < plugin->extraRenderingJobCount(); j++) {
            plugin->extraRenderingJobInit(j, op, srcPic, srcX, srcY, maskPic, maskX, maskY, destX, destY, width, height);
            if (op != PictOpClear) {
                XRenderComposite(display(), op, srcPic, maskPic, m_backBufferPicture->pictureHandle(), srcX, srcY,
                                 maskX, maskY, destX, destY, width, height);
            }
            plugin->extraRenderingJobCleanup(j);
        }

        plugin->postExtraRenderingActions();
    }
}

// Render the reconfigure rectangle.
void XRenderScreen::renderReconfigureRect() {
    XRenderPlugin *plugin = NULL;

    XRectangle rect = reconfigureRectangle();

    XSetForeground(display(), m_backBufferPicture->gcHandle(), XWhitePixel(display(), screenNumber()));
    XSetFunction(display(), m_backBufferPicture->gcHandle(), GXxor);
    XSetLineAttributes(display(), m_backBufferPicture->gcHandle(), 1, LineSolid, CapNotLast, JoinMiter);

    forEachPlugin(i, plugin) {
        plugin->reconfigureRectRenderActions(rect, m_backBufferPicture->gcHandle());
    }
    XDrawRectangles(display(), m_backBufferPicture->drawableHandle(),
                    m_backBufferPicture->gcHandle(), &rect, 1);
}

// Render a particular window onto the screen.
void XRenderScreen::renderWindow(XRenderWindow &window) {
    XRenderPlugin *plugin = NULL;

    // Update window contents.
    if (window.isDamaged()) {
        window.updateContents();
    }

    // Extra rendering jobs before the window is drawn.
    Picture srcPic, maskPic;
    int op, srcX, srcY, maskX, maskY, destX, destY, width, height;

    forEachPlugin(i, plugin) {
        plugin->extraPreWindowRenderingJob(window, op, srcPic, srcX, srcY, maskPic,
                                           maskX, maskY, destX, destY, width, height);
        if (op != PictOpClear) {
            XRenderComposite(display(), op, srcPic, maskPic, m_backBufferPicture->pictureHandle(), srcX, srcY,
                             maskX, maskY, destX, destY, width, height);
        }
    }

    // Render the window.
    op = PictOpOver;
    maskPic = window.maskPicture()->pictureHandle();

    forEachPlugin(i, plugin) {
        plugin->windowRenderingJobInit(window, op, maskPic);
    }
    XRenderComposite(display(), op, window.contentPicture()->pictureHandle(), maskPic, m_backBufferPicture->pictureHandle(),
                     0, 0, 0, 0, window.x(), window.y(), window.realWidth(), window.realHeight());
    forEachPlugin(i, plugin) {
        plugin->windowRenderingJobCleanup(window);
    }

    // Extra rendering jobs after the window is drawn.
    forEachPlugin(i, plugin) {
        plugin->extraPostWindowRenderingJob(window, op, srcPic, srcX, srcY, maskPic,
                                            maskX, maskY, destX, destY, width, height);
        if (op != PictOpClear) {
            XRenderComposite(display(), op, srcPic, maskPic, m_backBufferPicture->pictureHandle(), srcX, srcY,
                             maskX, maskY, destX, destY, width, height);
        }
    }
}

// Swap back and front buffers.
void XRenderScreen::swapBuffers() {
    XRenderComposite(display(), PictOpSrc, m_backBufferPicture->pictureHandle(), None, m_renderingPicture->pictureHandle(),
                     0, 0, 0, 0, 0, 0, rootWindow().width(), rootWindow().height());
}


//--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS --------------------------------

// Creates a window object from its XID.
BaseCompWindow *XRenderScreen::createWindowObject(Window window) {
    XRenderWindow *newWindow = new XRenderWindow(*this, window, m_pictFilter);
    return newWindow;
}
