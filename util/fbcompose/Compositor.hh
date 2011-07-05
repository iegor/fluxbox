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

#include "config.h"

#include "Constants.hh"
#include "Exceptions.hh"
#include "TickTracker.hh"

#include "FbTk/App.hh"

#include <X11/Xlib.h>

#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class Compositor;
    class CompositorConfig;
    class InitException;
    class RuntimeException;
    class TickTracker;


    //--- TYPEDEFS -------------------------------------------------------------

    /** A pointer to an X query extension function. */
    typedef Bool (*QueryExtensionFunction)(Display*, int*, int*);

    /** A pointer to an X query version function. */
    typedef Status (*QueryVersionFunction)(Display*, int*, int*);


    /**
     * Main class for the compositor.
     *
     * Note that if the serverauto rendering mode is selected, ServerAutoApp is
     * used instead. This was done to take out the ad-hoc code out of this
     * class, since this class is too complex for serverauto rendering.
     */
    class Compositor : public FbTk::App {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        Compositor(const CompositorConfig &configuration) throw(InitException);

        /** Destructor. */
        ~Compositor() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns The number of available screens. */
        int screenCount() const throw();

        /** \returns the reference to a particular screen. */
        BaseScreen &getScreen(int screenNumber) throw(RuntimeException);

        /** \returns the application's rendering mode. */
        RenderingMode renderingMode() const throw();

        /** \returns whether the X errors should be printed or not. */
        bool showXErrors() const throw();


        //--- EVENT LOOP -------------------------------------------------------

        /** Enters the event loop. */
        void eventLoop() throw(RuntimeException);


    private:
        //--- CONSTANTS --------------------------------------------------------

        /** How many micro seconds to sleep before restarting the event loop. */
        static const int SLEEP_TIME;


        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        Compositor(const Compositor&);

        /** Assignment operator. */
        Compositor &operator=(const Compositor&);


        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Acquire the ownership of compositing manager selections. */
        void getCMSelectionOwnership(int screenNumber) throw(InitException);

        /** Initializes all relevant X's extensions. */
        void initAllExtensions() throw(InitException);

        /** Initializes a particular X server extension. */
        void initExtension(const char *extensionName, QueryExtensionFunction extensionFunc,
                           QueryVersionFunction versionFunc, const int minMajorVer, const int minMinorVer,
                           int *eventBase, int *errorBase) throw(InitException);

        /** Initializes monitor heads on every screen. */
        void initHeads() throw();


        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Locates the screen an event affects. Returns -1 on failure. */
        int screenOfEvent(const XEvent &event) throw();


        //--- COMPOSITOR VARIABLES ---------------------------------------------

        /** A tick tracker that controls when the screen is redrawn. */
        TickTracker m_timer;

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

        /** Event base of the X Render extension. */
        int m_renderEventBase;

        /** Error base of the X Render extension. */
        int m_renderErrorBase;

        /** Event base of the X Shape extension. */
        int m_shapeEventBase;

        /** Error base of the X Shape extension. */
        int m_shapeErrorBase;

        /** Event base of the Xinerama extension. */
        int m_xineramaEventBase;

        /** Error base of the Xinerama extension. */
        int m_xineramaErrorBase;


        //--- OTHER VARIABLES --------------------------------------------------

        /** Whether the X errors should be printed or not. */
        bool m_showXErrors;
    };


    //--- VARIOUS HANDLERS -----------------------------------------------------

    /** Custom signal handler. */
    void handleSignal(int signal) throw();

    /** Custom X error handler. */
    int handleXError(Display *display, XErrorEvent *error) throw();


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
        return *(m_screens[screenNumber]);
    }

    // Returns the rendering mode.
    inline RenderingMode Compositor::renderingMode() const throw() {
        return m_renderingMode;
    }

    // Returns the number of screens.
    inline int Compositor::screenCount() const throw() {
        return m_screens.size();
    }

    // Returns whether the X errors should be printed.
    inline bool Compositor::showXErrors() const throw() {
        return m_showXErrors;
    }
}


#endif  // FBCOMPOSITOR_COMPOSITOR_HH
