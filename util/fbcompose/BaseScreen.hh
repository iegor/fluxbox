/** BaseScreen.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_SCREEN_HH
#define FBCOMPOSITOR_SCREEN_HH

#include "BaseCompWindow.hh"

#include <X11/Xlib.h>

#include <list>


namespace FbCompositor {

    class BaseCompWindow;
    class BaseScreen;

    /**
     * Base class for screen managing classes.
     */
    class BaseScreen {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        BaseScreen(int screenNumber);

        /** Destructor. */
        ~BaseScreen();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the list of windows on the screen. */
        std::list<BaseCompWindow> &allWindows() throw();

        /** \returns the list of windows on the screen (const version). */
        const std::list<BaseCompWindow> &allWindows() const throw();

        /** \returns screen's root window. */
        BaseCompWindow &rootWindow() throw();

        /** \returns screen's root window (const version). */
        const BaseCompWindow &rootWindow() const throw();

        /** \returns screen's number. */
        int screenNumber() const throw();


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Creates a new window and inserts it into the list of windows. */
        void createWindow(const BaseCompWindow &window);

        /** Destroys a window on this screen. */
        void destroyWindow(Window window);

        /** Maps a window on this screen. */
        void mapWindow(Window window);

        /** Unmaps a window on this screen. */
        void unmapWindow(Window window);


    private:
        //--- INTERNAL CONVENIENCE FUNCTIONS -----------------------------------

        /** Returns an iterator of m_windows that points to the given window. */
        std::list<BaseCompWindow>::iterator getWindowIterator(Window windowXID);


        //--- PRIVATE VARIABLES ------------------------------------------------

        /** Current connection to the X server. */
        Display *m_display;

        /** Screen's number. */
        int m_screenNumber;

        /** Screen's root window. */
        BaseCompWindow m_rootWindow;

        /** Screen's windows. */
        std::list<BaseCompWindow> m_windows;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns all of screen's windows (const version).
    inline std::list<BaseCompWindow> &BaseScreen::allWindows() throw() {
        return m_windows;
    }

    // Returns all of screen's windows (const version).
    inline const std::list<BaseCompWindow> &BaseScreen::allWindows() const throw() {
        return m_windows;
    }

    // Returns screen's root window.
    inline BaseCompWindow &BaseScreen::rootWindow() throw() {
        return m_rootWindow;
    }

    // Returns screen's root window (const version).
    inline const BaseCompWindow &BaseScreen::rootWindow() const throw() {
        return m_rootWindow;
    }

    // Returns the screen's number.
    inline int BaseScreen::screenNumber() const throw() {
        return m_screenNumber;
    }

}


#endif  // FBCOMPOSITOR_SCREEN_HH
