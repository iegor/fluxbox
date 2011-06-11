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

#include "Logging.hh"

#include "FbTk/FbWindow.hh"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xdamage.h>

#include <iosfwd>
#include <vector>


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
        BaseCompWindow(Window windowXID) throw();

        /** Destructor. */
        virtual ~BaseCompWindow() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the window's contents as a pixmap. */
        Pixmap contentPixmap() const throw();

        /** \returns whether the window is damaged or not. */
        bool isDamaged() const throw();

        /** \returns whether the screen is mapped or not. */
        bool isMapped() const throw();

        /** \returns the window's class. */
        int windowClass() const throw();


        /** \returns the window's height with borders factored in. */
        unsigned int realHeight() const throw();

        /** \returns the window's width with borders factored in. */
        unsigned int realWidth() const throw();


        //--- PROPERTY ACCESS --------------------------------------------------

        /** \returns the value of the specified property. */
        template<class T>
        std::vector<T> propertyValue(Atom propertyAtom);

        /** Convenience function for accessing properties with a single value. */
        template<class T>
        T singlePropertyValue(Atom propertyAtom, T defaultValue, const char *propertyName = 0);


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Add damage to a window. */
        virtual void addDamage(XRectangle area) throw();

        /** Set the clip shape as changed. */
        void setClipShapeChanged() throw();

        /** Mark the window as mapped. */
        virtual void setMapped() throw();

        /** Mark the window as unmapped. */
        virtual void setUnmapped() throw();

        /** Update the window's contents. */
        virtual void updateContents();

        /** Update window's geometry. */
        virtual void updateGeometry(const XConfigureEvent &event) throw();

        /** Update window's property. */
        virtual void updateProperty(Atom property, int state);


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


        /** \returns the vector, containing the damaged window's areas. */
        const std::vector<XRectangle> &damagedArea() const throw();

        /** \returns whether the window has been resized since the last update. */
        bool isResized() const throw();


        //--- PROTECTED WINDOW MANIPULATION ------------------------------------

        /** Removes all damage from the window. */
        void clearDamage() throw();

        /** Updates the window's content pixmap. */
        void updateContentPixmap() throw();

        /** Update the window's clip shape. */
        virtual void updateShape() throw();


    private:
        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Returns the raw contents of a property. */
        bool rawPropertyData(Atom propertyAtom, Atom propertyType,
                             unsigned long *itemCount_return, unsigned char **data_return);


        //--- WINDOW ATTRIBUTES ------------------------------------------------

        /** Window's class. */
        int m_class;

        /** Window's map state. */
        bool m_isMapped;


        /** Window's content pixmap. */
        Pixmap m_contentPixmap;

        /** Window's damage object. */
        Damage m_damage;

        /** A list of damaged rectangles. */
        std::vector<XRectangle> m_damagedArea;

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

    // Returns the vector, containing the damaged window's areas.
    inline const std::vector<XRectangle> &BaseCompWindow::damagedArea() const throw() {
        return m_damagedArea;
    }

    // Returns whether the window is damaged or not.
    inline bool BaseCompWindow::isDamaged() const throw() {
        return (!m_damagedArea.empty());
    }

    // Returns whether the window is mapped or not.
    inline bool BaseCompWindow::isMapped() const throw() {
        return m_isMapped;
    }

    // Returns whether the window has been resized since the last update.
    inline bool BaseCompWindow::isResized() const throw() {
        return m_isResized;
    }

    // Returns the window's height with borders factored in.
    inline unsigned int BaseCompWindow::realHeight() const throw() {
        return height() + 2 * borderWidth();
    }

    // Returns the window's width with borders factored in.
    inline unsigned int BaseCompWindow::realWidth() const throw() {
        return width() + 2 * borderWidth();
    }

    // Returns the window's class.
    inline int BaseCompWindow::windowClass() const throw() {
        return m_class;
    }


    //--- PROPERTY ACCESS ----------------------------------------------------------

    // Returns the value of the specified property.
    template<class T>
    std::vector<T> BaseCompWindow::propertyValue(Atom propertyAtom) {
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
    T BaseCompWindow::singlePropertyValue(Atom propertyAtom, T defaultValue, const char *propertyName) {
        if (!propertyAtom) {
            return defaultValue;
        }

        std::vector<T> values = propertyValue<T>(propertyAtom);
        if (values.size() == 0) {
            if (!propertyName) {
                propertyName = XGetAtomName(display(), propertyAtom);
            }
            fbLog_warn << "Property " << propertyName << " is undefined on window " << window()
                       << ", using default value." << std::endl;
            return defaultValue;
        } else {
            return values[0];
        }
    }
}

#endif  // FBCOMPOSITOR_WINDOW_HH
