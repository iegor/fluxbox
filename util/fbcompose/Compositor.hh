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


#include "Enumerations.hh"
#include "Exceptions.hh"
#include "TickTracker.hh"

#include "FbTk/App.hh"

#include <X11/Xlib.h>

#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class CompositorConfig;


    //--- TYPEDEFS -------------------------------------------------------------

    /** A pointer to an X query extension function. */
    typedef Bool (*QueryExtensionFunction)(Display*, int*, int*);

    /** A pointer to an X query version function. */
    typedef Status (*QueryVersionFunction)(Display*, int*, int*);


    //--- MAIN COMPOSITOR CLASS ------------------------------------------------

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
        Compositor(const CompositorConfig &configuration);

        /** Destructor. */
        ~Compositor();


        //--- EVENT LOOP -------------------------------------------------------

        /** Enters the event loop. */
        void eventLoop();


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        Compositor(const Compositor&);

        /** Assignment operator. */
        Compositor &operator=(const Compositor&);


        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Acquire the ownership of compositing manager selections. */
        Window getCMSelectionOwnership(int screenNumber);

        /** Initializes all relevant X's extensions. */
        void initAllExtensions();

        /** Initializes a particular X server extension. */
        void initExtension(const char *extensionName, QueryExtensionFunction extensionFunc,
                           QueryVersionFunction versionFunc, const int minMajorVer, const int minMinorVer,
                           int *eventBase, int *errorBase);

        /** Initializes monitor heads on every screen. */
        void initHeads();


        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Locates the screen an event affects. Returns -1 on failure. */
        int screenOfEvent(const XEvent &event);


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
    };


    //--- VARIOUS HANDLERS -----------------------------------------------------

    /** Custom signal handler. */
    void handleSignal(int signal);


    /** Custom X error handler (ignore). */
    int ignoreXError(Display *display, XErrorEvent *error);

    /** Custom X error handler (print, continue). */
    int printXError(Display *display, XErrorEvent *error);
}


#endif  // FBCOMPOSITOR_COMPOSITOR_HH
