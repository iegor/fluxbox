/** Compositor.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_COMPOSITOR_HH
#define FBCOMPOSITOR_COMPOSITOR_HH

#include "BaseScreen.hh"
#include "CompositorConfig.hh"
#include "Constants.hh"
#include "Exceptions.hh"

#include "FbTk/App.hh"
#include "FbTk/Command.hh"
#include "FbTk/Timer.hh"

#include <X11/Xlib.h>

#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class Compositor;
    class CompositorConfig;
    class InitException;
    class RuntimeException;

    /**
     * Main class for the compositor.
     */
    class Compositor : public FbTk::App {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        Compositor(const CompositorConfig &configuration) throw(InitException);

        /** Destructor. */
        ~Compositor();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns The number of available screens. */
        int screenCount() const throw();

        /** \returns the reference to a particular screen. */
        BaseScreen &getScreen(int screenNumber) throw(RuntimeException);

        /** \returns the application's rendering mode. */
        RenderingMode renderingMode() const throw();


        //--- EVENT LOOP -------------------------------------------------------

        /** Enters the event loop. */
        void eventLoop();

    private:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Copy constructor. */
        Compositor(const Compositor&);

        /** Assignment operator. */
        Compositor operator=(const Compositor&);


        //--- COMMANDS ---------------------------------------------------------

        /** A command that renders the screens. */
        class RenderScreensCommand : public FbTk::Command<void> {
        public :
            RenderScreensCommand(Compositor *instance) {
                m_instance = instance;
            }
            void execute() {
                m_instance->renderScreens();
            }
        private :
            Compositor *m_instance;
        };


        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Acquire the ownership of compositing manager selections. */
        void getCMSelectionOwnership(int screenNumber) throw(InitException);

        /** Initializes all relevant X's extensions. */
        void initAllExtensions() throw(InitException);

        /** Initializes a particular X server extension. */
        void initExtension(const char *extensionName, QueryExtensionFunction extensionFunc,
                           QueryVersionFunction versionFunc, const int minMajorVer, const int minMinorVer,
                           int *eventBase, int *errorBase) throw(InitException);


        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Render the screens. */
        void renderScreens();

        /** Locates the screen an event affects. Returns -1 on failure. */
        int screenOfEvent(const XEvent &event);


        //--- COMPOSITOR VARIABLES ---------------------------------------------

        /** A timer that controls when the screen is redrawn. */
        FbTk::Timer m_redrawTimer;

        /** Rendering mode in use. */
        RenderingMode m_renderingMode;

        /** The array of available screens. */
        std::vector<BaseScreen*> m_screens;


        //--- EXTENSION BASE VARIABLES -----------------------------------------

        /** Event base of the X Composite extension. */
        int m_compositeEventBase;

        /** Error base of the X Composite extension. */
        int m_compositeErrorBase;

        /** Event base of the X Damage extension. */
        int m_damageEventBase;

        /** Error base of the X Damage extension. */
        int m_damageErrorBase;

        /** Event base of the GLX extension. */
        int m_glxEventBase;

        /** Error base of the GLX extension. */
        int m_glxErrorBase;

        /** Event base of the X Fixes extension. */
        int m_fixesEventBase;

        /** Error base of the X Fixes extension. */
        int m_fixesErrorBase;

        /** Event base of the X Shape extension. */
        int m_shapeEventBase;

        /** Error base of the X Shape extension. */
        int m_shapeErrorBase;
    };


    //--- VARIOUS HANDLERS -----------------------------------------------------

    /** Custom X error handler. */
    int handleXError(Display *display, XErrorEvent *error);


    //--- OTHER FUNCTIONS ------------------------------------------------------

    /** \returns a properly type cast pointer to the app object. */
    Compositor *compositorInstance() throw();


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns a properly type cast pointer to the app object.
    inline Compositor *compositorInstance() throw() {
        return dynamic_cast<Compositor*>(FbTk::App::instance());
    }

    // Returns a particular screen.
    inline BaseScreen &Compositor::getScreen(int screenNumber) throw(RuntimeException) {
        if ((screenNumber < 0) || (screenNumber >= screenCount())) {
            throw RuntimeException("getScreen(int) was given an out of bounds index.");
        }
        return *m_screens[screenNumber];
    }

    // Returns the rendering mode.
    inline RenderingMode Compositor::renderingMode() const throw() {
        return m_renderingMode;
    }

    // Returns the number of screens.
    inline int Compositor::screenCount() const throw() {
        return m_screens.size();
    }
}


#endif  // FBCOMPOSITOR_COMPOSITOR_HH
