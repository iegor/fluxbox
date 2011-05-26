/** BaseCompWindow.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_WINDOW_HH
#define FBCOMPOSITOR_WINDOW_HH

#include "FbTk/FbWindow.hh"

#include <X11/Xlib.h>

#include <iosfwd>


namespace FbCompositor {

    class BaseCompWindow;

    /** << output stream operator for the BaseCompWindow class. */
    std::ostream &operator<<(std::ostream& out, const BaseCompWindow& window);


    /**
     * Base class for composited windows.
     */
    class BaseCompWindow : public FbTk::FbWindow {

        //--- FRIEND OPERATORS -------------------------------------------------
        friend std::ostream &operator<<(std::ostream& out, const BaseCompWindow& window);

    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        BaseCompWindow(Window windowXID);

        /** Destructor. */
        ~BaseCompWindow();


        //--- ACCESSORS --------------------------------------------------------

        /** Returns whether the screen is mapped or not. */
        bool isMapped() const throw();


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Marks the window as mapped. */
        void setMapped() throw();

        /** Marks the window as unmapped. */
        void setUnmapped() throw();

    private:
        //--- WINDOW ATTRIBUTES ------------------------------------------------

        /** Window's map state. */
        bool m_isMapped;
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns whether the window is mapped or not.
    inline bool BaseCompWindow::isMapped() const throw() {
        return m_isMapped;
    }

}

#endif  // FBCOMPOSITOR_WINDOW_HH
