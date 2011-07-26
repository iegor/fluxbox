/** OpenGLTexPartitioner.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLTEXPARTITIONER_HH
#define FBCOMPOSITOR_OPENGLTEXPARTITIONER_HH


#include "Exceptions.hh"
#include "OpenGLResources.hh"

#include "FbTk/RefCount.hh"

#include <GL/glxew.h>
#include <GL/glx.h>

#include <X11/Xlib.h>

#include <algorithm>
#include <vector>


namespace FbCompositor {

    class OpenGLScreen;


    //--- CONSTANTS ------------------------------------------------------------
    
    /** North border flag. */
    const int BORDER_NORTH = 1 << 0;

    /** East border flag. */
    const int BORDER_EAST = 1 << 1;

    /** South border flag. */
    const int BORDER_SOUTH = 1 << 2;

    /** West border flag. */
    const int BORDER_WEST = 1 << 3;

    /** A bitfield with all border flags set. */
    const int BORDER_ALL = BORDER_NORTH | BORDER_EAST | BORDER_SOUTH | BORDER_WEST;


    //--- SUPPORTING STRUCTURES ------------------------------------------------

    /**
     * A single 2D texture partition.
     */
    struct TexturePart {
        OpenGL2DTexturePtr texture;     ///< Partition's contents.
        unsigned int borders;           ///< A bitfield showing borders, adjacent to this partition.
    };


    //--- TEXTURE PARTITIONER --------------------------------------------------

    /**
     * A wrapper that automatically splits large textures into manageable (i.e.
     * with supported size) parts.
     */
    class OpenGL2DTexturePartition {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGL2DTexturePartition(const OpenGLScreen &screen, bool swizzleAlphaToOne);

        /** Destructor. */
        ~OpenGL2DTexturePartition();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the partitions of the current texture. */
        std::vector<TexturePart> partitions() const;


        /** \returns the full height of the current texture. */
        int fullHeight() const;

        /** \returns the full width of the current texture. */
        int fullWidth() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Sets the texture's contents to the given pixmap. */
        void setPixmap(Pixmap pixmap, bool managePixmap, int width, int height, int depth);


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        OpenGL2DTexturePartition(const OpenGL2DTexturePartition &other);

        /** Assignment operator. */
        OpenGL2DTexturePartition &operator=(const OpenGL2DTexturePartition &other);


        //--- INTERNALS --------------------------------------------------------

        /** Maximum supported texture size. */
        int m_maxTextureSize;

        /** Whether alpha channel should be swizzled to one. */
        bool m_swizzleAlphaToOne;

        
        /** Partitions of the texture. */
        std::vector<TexturePart> m_partitions;

        /** Pixmap of the texture's contents. */
        Pixmap m_pixmap;


        /** Full texture height. */
        int m_fullHeight;

        /** Full texture width. */
        int m_fullWidth;


        /** Current connection to the X server. */
        Display *m_display;

        /** Screen that manages this texture. */
        const OpenGLScreen &m_screen;
    };

    // Returns the full height of the current texture.
    inline int OpenGL2DTexturePartition::fullHeight() const {
        return m_fullHeight;
    }

    // Returns the full width of the current texture.
    inline int OpenGL2DTexturePartition::fullWidth() const {
        return m_fullWidth;
    }

    // Returns the partitions of the current texture.
    inline std::vector<TexturePart> OpenGL2DTexturePartition::partitions() const {
        return m_partitions;
    }


    //--- SUPPORTING FUNCTIONS -------------------------------------------------

    /** Returns the border bitfield of the given partition. */
    unsigned int getBorderBitfield(int unitWidth, int unitHeight, int x, int y);

    /** Space partitioning function. */
    std::vector<XRectangle> partitionSpace(int x, int y, int width, int height, int maxPartitionSize,
                                           int *unitWidth_return = 0, int *unitHeight_return = 0);


    //--- TYPEDEFS -------------------------------------------------------------

    /** OpenGL texture partition smart pointer. */
    typedef FbTk::RefCount<OpenGL2DTexturePartition> OpenGL2DTexturePartitionPtr;
}

#endif  // FBCOMPOSITOR_OPENGLTEXPARTITIONER_HH

