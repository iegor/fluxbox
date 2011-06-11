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

#include <iosfwd>
#include <list>


namespace FbCompositor {

    class BaseCompWindow;
    class BaseScreen;

    /** << output stream operator for the BaseScreen class. */
    std::ostream &operator<<(std::ostream& out, const BaseScreen& s);

    /**
     * Base class for screen managing classes.
     */
    class BaseScreen {

        //--- FRIEND OPERATORS -------------------------------------------------
        friend std::ostream &operator<<(std::ostream& out, const BaseScreen& s);

    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        BaseScreen(int screenNumber);

        /** Destructor. */
        virtual ~BaseScreen();


        //--- OTHER INITIALIZATION ---------------------------------------------

        /** Initializes all of the windows on the screen. */
        void initWindows();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the active window XID. */
        Window activeWindow() const throw();

        /** \returns the index of the current workspace. */
        int currentWorkspace() const throw();

        /** \returns the current connection to the X server. */
        Display *display() throw();

        /** \returns screen's root window. */
        BaseCompWindow &rootWindow() throw();

        /** \returns screen's root window (const version). */
        const BaseCompWindow &rootWindow() const throw();

        /** \returns screen's number. */
        int screenNumber() const throw();

        /** \returns the index of the currently active workspace. */
        int workspaceCount() const throw();


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Creates a new window on this screen. */
        void createWindow(Window window);

        /** Damages a window on this screen. */
        void damageWindow(Window window, XRectangle area);

        /** Destroys a window on this screen. */
        void destroyWindow(Window window);

        /** Maps a window on this screen. */
        void mapWindow(Window window);

        /** Updates window's configuration. */
        void reconfigureWindow(const XConfigureEvent &event);

        /** Updates window's shape. */
        void updateShape(Window window);

        /** Unmaps a window on this screen. */
        void unmapWindow(Window window);

        /** Updates the value of some window's property. */
        void updateWindowProperty(Window window, Atom property, int state);


        /** Adds a window to ignore list, stops tracking it if it is being tracked. */
        void addWindowToIgnoreList(Window window);

        /** Checks whether a given window is managed by the current screen. */
        bool isWindowManaged(Window window);


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Notifies the screen of a background change. */
        virtual void setBackgroundChanged();

        /** Notifies the screen of a root window change. */
        virtual void setRootWindowChanged();


        //--- SCREEN RENDERING -------------------------------------------------

        /** Renders the screen's contents. */
        virtual void renderScreen() = 0;


    protected:
        //--- PROTECTED ACCESSORS ----------------------------------------------

        /** \returns the list of windows on the screen. */
        const std::list<BaseCompWindow*> &allWindows() const throw();


        //--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS ------------------------

        /** Creates a window object from its XID. */
        virtual BaseCompWindow *createWindowObject(Window window) = 0;


    private:
        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** \returns an iterator of m_windows that points to the given window. */
        std::list<BaseCompWindow*>::iterator getWindowIterator(Window windowXID);


        //--- PROPERTIES OF INTEREST -------------------------------------------

        /** Property that denotes the currently active window. */
        Atom m_activeWindowAtom;

        /** Property that denotes the pixmap of the root window. */
        Atom m_rootPixmapAtom;

        /** Property that denotes the index of active workspace. */
        Atom m_workspaceAtom;

        /** Property that denotes the number of workspaces. */
        Atom m_workspaceCountAtom;


        //--- PRIVATE VARIABLES ------------------------------------------------

        /** Current connection to the X server. */
        Display *m_display;

        /** Windows that should be ignored. */
        std::vector<Window> m_ignoreList;

        /** Screen's number. */
        int m_screenNumber;

        /** Screen's root window. */
        BaseCompWindow m_rootWindow;

        /** Screen's windows. */
        std::list<BaseCompWindow*> m_windows;


        /** XID of the active window. */
        Window m_activeWindowXID;

        /** The index of the current workspace. */
        int m_currentWorkspace;

        /** The total number of workspaces. */
        int m_workspaceCount;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the active window XID.
    inline Window BaseScreen::activeWindow() const throw() {
        return m_activeWindowXID;
    }

    // Returns all of screen's windows.
    inline const std::list<BaseCompWindow*> &BaseScreen::allWindows() const throw() {
        return m_windows;
    }

    // Returns the index of the current workspace.
    inline int BaseScreen::currentWorkspace() const throw() {
        return m_currentWorkspace;
    }

    // Returns the current connection to the X server.
    inline Display *BaseScreen::display() throw() {
        return m_display;
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

    // Returns the index of the currently active workspace.
    inline int BaseScreen::workspaceCount() const throw() {
        return m_workspaceCount;
    }
}


#endif  // FBCOMPOSITOR_SCREEN_HH
