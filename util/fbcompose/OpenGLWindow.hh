/** OpenGLWindow.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
#define FBCOMPOSITOR_XRENDERAUTOWINDOW_HH


#include "BaseCompWindow.hh"
#include "Exceptions.hh"
#include "OpenGLUtility.hh"
#include "OpenGLResources.hh"

#include <GL/glxew.h>
#include <GL/glx.h>

#include <vector>


namespace FbCompositor {

    class InitException;
    class OpenGLWindow;


    /**
     * Manages windows in OpenGL rendering mode.
     */
    class OpenGLWindow : public BaseCompWindow {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLWindow(const OpenGLScreen &screen, Window windowXID);

        /** Destructor. */
        virtual ~OpenGLWindow();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns an object, holding the window's contents as an OpenGL texture. */
        OpenGL2DTexturePartitionPtr contentTexturePartition() const;

        /** \returns an object, holding the window's shape as an OpenGL texture. */
        OpenGL2DTexturePartitionPtr shapeTexturePartition() const;

        /** \returns the number of contents' partitions. */
        int partitionCount() const;

        /** \returns an object, holding the window position buffer. */
        std::vector<OpenGLBufferPtr> windowPosBuffer() const;


        //--- WINDOW UPDATE FUNCTIONS ------------------------------------------

        /** Updates the window's contents. */
        void updateContents();

        /** Updates window's geometry. */
        void updateGeometry(const XConfigureEvent &event);

        /** Updates the window's shape. */
        void updateShape();

        /** Updates the window position vertex array. */
        void updateWindowPosArray();


    private :
        //--- RENDERING-RELATED VARIABLES --------------------------------------

        /** Window's content texture. */
        OpenGL2DTexturePartitionPtr m_contentTexturePartition;

        /** Window's shape texture. */
        OpenGL2DTexturePartitionPtr m_shapeTexturePartition;


        /** Window position array. */
        GLfloat m_windowPosArray[8];

        /** Window position buffer holder. */
        std::vector<OpenGLBufferPtr> m_windowPosBuffer;
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns the window's contents as an OpenGL texture.
    inline OpenGL2DTexturePartitionPtr OpenGLWindow::contentTexturePartition() const {
        return m_contentTexturePartition;
    }

    // Returns the number of contents' partitions.
    inline int OpenGLWindow::partitionCount() const {
        return m_contentTexturePartition->partitions().size();
    }

    // Returns an object, holding the window's shape as an OpenGL texture.
    inline OpenGL2DTexturePartitionPtr OpenGLWindow::shapeTexturePartition() const {
        return m_shapeTexturePartition;
    }

    // Returns the window position buffer.
    inline std::vector<OpenGLBufferPtr> OpenGLWindow::windowPosBuffer() const {
        return m_windowPosBuffer;
    }
}

#endif  // FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
