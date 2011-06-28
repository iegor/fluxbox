/** XRenderScreen.hh file for the fluxbox compositor. */

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

#ifndef FBCOMPOSITOR_XRENDERSCREEN_HH
#define FBCOMPOSITOR_XRENDERSCREEN_HH

#include "config.h"

#ifdef USE_XRENDER_COMPOSITING


#include "BaseScreen.hh"
#include "CompositorConfig.hh"
#include "XRenderWindow.hh"

#include <X11/extensions/Xrender.h>
#include <X11/Xlib.h>


namespace FbCompositor {

    class BaseScreen;
    class CompositorConfig;
    class XRenderScreen;


    /**
     * Manages the screen in XRender rendering mode.
     */
    class XRenderScreen : public BaseScreen {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderScreen(int screenNumber, const CompositorConfig &config);

        /** Destructor. */
        ~XRenderScreen();


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Notifies the screen of a background change. */
        void setRootPixmapChanged();

        /** Notifies the screen of a root window change. */
        void setRootWindowSizeChanged();


        //--- SCREEN RENDERING -------------------------------------------------

        /** Renders the screen's contents. */
        void renderScreen();


    protected:
        //--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS ------------------------

        /** Creates a window object from its XID. */
        BaseCompWindow *createWindowObject(Window window);


    private:
        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Initializes the rendering surface. */
        void initRenderingSurface() throw(InitException);

        /** Initializes background picture. */
        void initBackgroundPicture();


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Update the background picture. */
        void updateBackgroundPicture();


        //--- RENDERING FUNCTIONS ----------------------------------------------

        /** Render the desktop wallpaper. */
        void renderBackground();

        /** Render the reconfigure rectangle. */
        void renderReconfigureRect();

        /** Render a particular window onto the screen. */
        void renderWindow(XRenderWindow &window);

        /** Swap back and front buffers. */
        void swapBuffers();


        //--- MAIN RENDERING-RELATED VARIABLES ---------------------------------

        /** The back buffer graphical context. */
        GC m_backBufferGC;

        /** The picture format of the back buffer. */
        XRenderPictFormat *m_backBufferPictFormat;

        /** The picture of the back buffer. */
        Picture m_backBufferPicture;

        /** Back buffer pixmap. */
        Pixmap m_backBufferPixmap;


        /** The picture format of the rendering window. */
        XRenderPictFormat *m_renderingPictFormat;

        /** The picture of the rendering window. */
        Picture m_renderingPicture;

        /** The rendering window. */
        Window m_renderingWindow;


        /** The format of the root window picture. */
        XRenderPictFormat *m_rootPictFormat;

        /** The picture of the root window. */
        Picture m_rootPicture;

        /** Whether the root window has changed since the last update. */
        bool m_rootChanged;


        /** The picture filter to use. */
        const char *m_pictFilter;
    };

}

#endif  // USE_XRENDER_COMPOSITING

#endif  // FBCOMPOSITOR_XRENDERSCREEN_HH
