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

#include "config.h"

#include "Exceptions.hh"

#include "FbTk/FbWindow.hh"

#include <X11/Xlib.h>
#ifdef HAVE_XDAMAGE
    #include <X11/extensions/Xdamage.h>
#endif  // HAVE_XDAMAGE

#include <iosfwd>
#include <vector>


namespace FbCompositor {

    class BaseCompWindow;
    class BaseScreen;


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
        BaseCompWindow(const BaseScreen &screen, Window windowXID) throw(InitException);

        /** Destructor. */
        virtual ~BaseCompWindow() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the window's opacity. */
        int alpha() const throw();

        /** \returns the window's contents as a pixmap. */
        Pixmap contentPixmap() const throw();

        /** \returns whether the window is damaged or not. */
        bool isDamaged() const throw();

        /** \returns whether the screen is mapped or not. */
        bool isMapped() const throw();

        /** \returns the window's screen. */
        const BaseScreen &screen() const throw();
        
        /** \returns the window's visual. */
        Visual *visual() throw();

        /** \returns the window's visual (const version). */
        const Visual *visual() const throw();

        /** \returns the window's class. */
        int windowClass() const throw();


        /** \returns the window's dimensions as an XRectangle. */
        XRectangle dimensions() const throw();

        /** \returns the window's height with borders factored in. */
        unsigned int realHeight() const throw();

        /** \returns the window's width with borders factored in. */
        unsigned int realWidth() const throw();


        //--- PROPERTY ACCESS --------------------------------------------------

        /** \returns the value of the specified property. */
        template<class T>
        std::vector<T> propertyValue(Atom propertyAtom) throw();

        /** Convenience function for accessing properties with a single value. */
        template<class T>
        T singlePropertyValue(Atom propertyAtom, T defaultValue) throw();


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Add damage to a window. */
        virtual void addDamage() throw();

        /** Mark the window as mapped. */
        virtual void setMapped() throw();

        /** Mark the window as unmapped. */
        virtual void setUnmapped() throw();

        /** Update the window's contents. */
        virtual void updateContents() throw(RuntimeException);

        /** Update window's geometry. */
        virtual void updateGeometry(const XConfigureEvent &event) throw();

        /** Update window's property. */
        virtual void updateProperty(Atom property, int state) throw(RuntimeException);


        /** Set the clip shape as changed. */
        void setClipShapeChanged() throw();


    protected:
        //--- PROTECTED ACCESSORS ----------------------------------------------

        /** \returns whether the chip shape changed since the last update. */
        int clipShapeChanged() const throw();

        /** \returns the number of rectangles that make up the clip shape. */
        int clipShapeRectCount() const throw();

        /** \returns the ordering of rectangles that make up the clip shape. */
        int clipShapeRectOrder() const throw();

        /** \returns the rectangles that make up the clip shape. */
        XRectangle *clipShapeRects() const throw();


        /** \returns whether the window has been resized since the last update. */
        bool isResized() const throw();


        //--- PROTECTED WINDOW MANIPULATION ------------------------------------

        /** Removes all damage from the window. */
        void clearDamage() throw();

        /** Updates the window's content pixmap. */
        void updateContentPixmap() throw();

        /** Update the window's clip shape. */
        virtual void updateShape() throw(RuntimeException);


        //--- OTHER FUNCTIONS --------------------------------------------------

        /** Checks whether the current window is bad. */
        bool isWindowBad() throw();


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        BaseCompWindow(const BaseCompWindow&);

        /** Assignment operator. */
        BaseCompWindow &operator=(const BaseCompWindow&);


        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Returns the raw contents of a property. */
        bool rawPropertyData(Atom propertyAtom, Atom propertyType,
                             unsigned long *itemCount_return, unsigned char **data_return) throw();


        //--- WINDOW ATTRIBUTES ------------------------------------------------

        /** Window's screen. */
        const BaseScreen &m_screen;


        /** Window opacity. */
        int m_alpha;

        /** Window's class. */
        int m_class;

        /** Window's map state. */
        bool m_isMapped;

        /** Window's visual. */
        Visual *m_visual;


        /** Window's content pixmap. */
        Pixmap m_contentPixmap;

#ifdef HAVE_XDAMAGE
        /** Window's damage object. */
        Damage m_damage;
#endif  // HAVE_XDAMAGE

        /** Shows whether the window is damaged. */
        bool m_isDamaged;

        /** Shows whether the window has been resized since the last update. */
        bool m_isResized;


        /** The number of rectangles of window's clip shape. */
        int m_clipShapeRectCount;

        /** The ordering of window clip shape rectangles. */
        int m_clipShapeRectOrder;

        /** Rectangles, that make up the window's clip shape. */
        XRectangle *m_clipShapeRects;

        /** Shows whether the clip shape changed since the last update. */
        bool m_clipShapeChanged;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the window's opacity.
    inline int BaseCompWindow::alpha() const throw() {
        return m_alpha;
    }

    // Returns whether the chip shape changed since the last update.
    inline int BaseCompWindow::clipShapeChanged() const throw() {
        return m_clipShapeChanged;
    }

    // Returns the number of rectangles that make up the clip shape.
    inline int BaseCompWindow::clipShapeRectCount() const throw() {
        return m_clipShapeRectCount;
    }

    // Returns the ordering of rectangles that make up the clip shape.
    inline int BaseCompWindow::clipShapeRectOrder() const throw() {
        return m_clipShapeRectOrder;
    }

    // Returns the rectangles that make up the clip shape.
    inline XRectangle *BaseCompWindow::clipShapeRects() const throw() {
        return m_clipShapeRects;
    }

    // Returns the window's contents as a pixmap.
    inline Pixmap BaseCompWindow::contentPixmap() const throw() {
        return m_contentPixmap;
    }

    // Returns the window's dimensions as an XRectangle.
    inline XRectangle BaseCompWindow::dimensions() const throw() {
        XRectangle dim = { x(), y(), realWidth(), realHeight() };
        return dim;
    }

    // Returns whether the window is damaged or not.
    inline bool BaseCompWindow::isDamaged() const throw() {
        return m_isDamaged;
    }

    // Returns whether the window is mapped or not.
    inline bool BaseCompWindow::isMapped() const throw() {
        return m_isMapped;
    }

    // Returns whether the window has been resized since the last update.
    inline bool BaseCompWindow::isResized() const throw() {
        return m_isResized;
    }

    // Returns the window's screen.
    inline const BaseScreen &BaseCompWindow::screen() const throw() {
        return m_screen;
    }

    // Returns the window's height with borders factored in.
    inline unsigned int BaseCompWindow::realHeight() const throw() {
        return height() + 2 * borderWidth();
    }

    // Returns the window's width with borders factored in.
    inline unsigned int BaseCompWindow::realWidth() const throw() {
        return width() + 2 * borderWidth();
    }

    // Returns the window's visual.
    inline Visual *BaseCompWindow::visual() throw() {
        return m_visual;
    }

    // Returns the window's visual (const version).
    inline const Visual *BaseCompWindow::visual() const throw() {
        return m_visual;
    }

    // Returns the window's class.
    inline int BaseCompWindow::windowClass() const throw() {
        return m_class;
    }


    //--- PROPERTY ACCESS ----------------------------------------------------------

    // Returns the value of the specified property.
    template<class T>
    std::vector<T> BaseCompWindow::propertyValue(Atom propertyAtom) throw() {
        unsigned long nItems;
        T *data;

        if (rawPropertyData(propertyAtom, AnyPropertyType, &nItems, reinterpret_cast<unsigned char**>(&data))) {
            std::vector<T> actualData(data, data + nItems);
            XFree(data);
            return actualData;
        }

        return std::vector<T>();
    }

    // Convenience function for accessing properties with a single value.
    template<class T>
    T BaseCompWindow::singlePropertyValue(Atom propertyAtom, T defaultValue) throw() {
        if (!propertyAtom) {
            return defaultValue;
        }

        std::vector<T> values = propertyValue<T>(propertyAtom);
        if (values.size() == 0) {
            return defaultValue;
        } else {
            return values[0];
        }
    }
}

#endif  // FBCOMPOSITOR_WINDOW_HH
