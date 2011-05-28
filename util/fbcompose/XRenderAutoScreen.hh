/** XRenderAutoScreen.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERAUTOSCREEN_HH
#define FBCOMPOSITOR_XRENDERAUTOSCREEN_HH

#include "BaseScreen.hh"
#include "BaseCompWindow.hh"


namespace FbCompositor {

    class BaseCompWindow;
    class XRenderAutoScreen;


    /**
     * Manages screen in XRender auto mode.
     *
     * This class is, strictly speaking, necessary, since pretty much
     * everything is handled by the X server in the XRender auto mode. I will
     * use this class to lay out the interface for other screen management
     * classes and for testing purposes.
     */
    class XRenderAutoScreen : public BaseScreen {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------
        
        /** Constructor. */
        XRenderAutoScreen(int screenNumber);

        /** Destructor. */
        ~XRenderAutoScreen();


    protected:
        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Creates a window object from its XID. */
        BaseCompWindow *createWindowObject(Window window);

        /** Cleans up a window object before it is deleted. */
        void cleanupWindowObject(BaseCompWindow &window);

        /** Damages a window object. */
        void damageWindowObject(BaseCompWindow &window);

        /** Maps a window object. */
        void mapWindowObject(BaseCompWindow &window);

        /** Updates window's configuration. */
        void reconfigureWindowObject(BaseCompWindow &window);

        /** Unmaps a window object. */
        void unmapWindowObject(BaseCompWindow &window);

        /** Updates the value of some window's property. */
        void updateWindowObjectProperty(BaseCompWindow &window, Atom property, int state);
    };

}


#endif  // FBCOMPOSITOR_XRENDERAUTOSCREEN_HH
