/** OpenGLResources.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLRESOURCES_HH
#define FBCOMPOSITOR_OPENGLRESOURCES_HH


#include "Exceptions.hh"

#include "FbTk/RefCount.hh"

#include <GL/glxew.h>
#include <GL/glx.h>

#include <X11/Xlib.h>

#include <algorithm>
#include <vector>


namespace FbCompositor {

    class OpenGLScreen;
    

    //--- OPENGL BUFFER WRAPPER ------------------------------------------------

    /**
     * A wrapper for OpenGL buffers.
     */
    class OpenGLBuffer {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLBuffer(const OpenGLScreen &screen, GLenum targetBuffer);

        /** Destructor. */
        ~OpenGLBuffer();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the handle to the buffer held. */
        GLuint handle() const;

        /** \returns the target of the buffer. */
        GLenum target() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Bind the buffer to its target. */
        void bind();

        /** Loads the given data into the buffer. */
        void bufferData(int elementSize, const GLvoid *data, GLenum usageHint);


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        OpenGLBuffer(const OpenGLBuffer &other);

        /** Assignment operator. */
        OpenGLBuffer &operator=(const OpenGLBuffer &other);


        //--- INTERNALS --------------------------------------------------------

        /** The buffer in question. */
        GLuint m_buffer;

        /** The target buffer. */
        GLenum m_target;


        /** Current connection to the X server. */
        Display *m_display;

        /** Screen that manages this buffer. */
        const OpenGLScreen &m_screen;
    };

    // Bind the buffer to its target.
    inline void OpenGLBuffer::bind() {
        glBindBuffer(m_target, m_buffer);
    }

    // Loads the given data into the buffer.
    inline void OpenGLBuffer::bufferData(int elementSize, const GLvoid *data, GLenum usageHint) {
        bind();
        glBufferData(m_target, elementSize, data, usageHint);
    }

    // Returns the handle to the buffer held.
    inline GLuint OpenGLBuffer::handle() const {
        return m_buffer;
    }

    // Returns the target of the buffer.
    inline GLenum OpenGLBuffer::target() const {
        return m_target;
    }


    /** OpenGL buffer wrapper smart pointer. */
    typedef FbTk::RefCount<OpenGLBuffer> OpenGLBufferPtr;


    //--- OPENGL TEXTURE WRAPPER -----------------------------------------------

    /**
     * OpenGL texture wrapper.
     */
    class OpenGL2DTexture {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGL2DTexture(const OpenGLScreen &screen, bool swizzleAlphaToOne);

        /** Destructor. */
        ~OpenGL2DTexture();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the handle to the texture held. */
        GLuint handle() const;

        /** \returns the height of the texture. */
        int height() const;

        /** \returns the width of the texture. */
        int width() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Bind the texture to its target. */
        void bind();

        /** Sets the texture's contents to the given pixmap. */
        void setPixmap(Pixmap pixmap, bool managePixmap, int width, int height, bool forceDirect = false);


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        OpenGL2DTexture(const OpenGL2DTexture &other);

        /** Assignment operator. */
        OpenGL2DTexture &operator=(const OpenGL2DTexture &other);


        //--- INTERNALS --------------------------------------------------------

        /** Pixmap of the texture's contents. */
        Pixmap m_pixmap;

        /** GLX pixmap of the texture's contents. */
        GLXPixmap m_glxPixmap;

        /** The texture in question. */
        GLuint m_texture;


        /** Height of the texture. */
        int m_height;

        /** Width of the texture. */
        int m_width;


        /** Current connection to the X server. */
        Display *m_display;

        /** Screen that manages this texture. */
        const OpenGLScreen &m_screen;
    };

    // Bind the texture to its target.
    inline void OpenGL2DTexture::bind() {
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }

    // Returns the handle to the texture held.
    inline GLuint OpenGL2DTexture::handle() const {
        return m_texture;
    }

    // Returns the height of the texture.
    inline int OpenGL2DTexture::height() const {
        return m_height;
    }

    // Returns the width of the texture.
    inline int OpenGL2DTexture::width() const {
        return m_width;
    }


    /** OpenGL texture wrapper smart pointer. */
    typedef FbTk::RefCount<OpenGL2DTexture> OpenGL2DTexturePtr;


    //--- OPENGL TEXTURE PARTITION ---------------------------------------------

    /**
     * A single texture partition.
     */
    struct TexturePart {
        OpenGL2DTexturePtr texture;   ///< Partition's contents.
        int borders;                ///< A bitfield showing borders, adjacent to this partition.
    };

    /** North border flag. */
    static const int BORDER_NORTH = 1 << 0;

    /** East border flag. */
    static const int BORDER_EAST = 1 << 1;

    /** South border flag. */
    static const int BORDER_SOUTH = 1 << 2;

    /** West border flag. */
    static const int BORDER_WEST = 1 << 3;


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

        /** \returns maximum supported texture size. */
        int maxTextureSize() const;

        /** \returns the partitions of the current texture. */
        std::vector<TexturePart> partitions() const;


        /** \returns the full height of the current texture. */
        int fullHeight() const;

        /** \returns the full width of the current texture. */
        int fullWidth() const;

        /** \returns the index of the given partition. */
        int partitionIndex(int x, int y);

        /** \returns the number of partitions along the Y axis. */
        int unitHeight() const;

        /** \returns the number of partitions along the X axis. */
        int unitWidth() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Sets the texture's contents to the given pixmap. */
        void setPixmap(Pixmap pixmap, bool managePixmap, int width, int height, int depth, bool forceDirect = false);


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

        /** Number of partitions along Y axis. */
        int m_unitHeight;

        /** Number of partitions along X axis. */
        int m_unitWidth;


        /** Current connection to the X server. */
        Display *m_display;

        /** Screen that manages this texture. */
        const OpenGLScreen &m_screen;
    };

    // Returns the partitions of the current texture.
    inline std::vector<TexturePart> OpenGL2DTexturePartition::partitions() const {
        return m_partitions;
    }

    // Returns the full height of the current texture.
    inline int OpenGL2DTexturePartition::fullHeight() const {
        return m_fullHeight;
    }

    // Returns the full width of the current texture.
    inline int OpenGL2DTexturePartition::fullWidth() const {
        return m_fullWidth;
    }

    // Returns maximum supported texture size.
    inline int OpenGL2DTexturePartition::maxTextureSize() const {
        return m_maxTextureSize;
    }

    // Returns the index of the given partition.
    inline int OpenGL2DTexturePartition::partitionIndex(int x, int y) {
        if ((x < 0) || (x >= m_unitWidth) || (y < 0) || (y >= m_unitHeight)) {
            throw RuntimeException("Out of bounds index in OpenGL2DTexturePartition::partitionIndex.");
        } else {
            return y * m_unitWidth + x;
        }
    }

    // Returns the number of partitions along the Y axis.
    inline int OpenGL2DTexturePartition::unitHeight() const {
        return m_unitHeight;
    }

    // Returns the number of partitions along the X axis.
    inline int OpenGL2DTexturePartition::unitWidth() const {
        return m_unitWidth;
    }


    /** OpenGL texture partition smart pointer. */
    typedef FbTk::RefCount<OpenGL2DTexturePartition> OpenGL2DTexturePartitionPtr;
}

#endif  // FBCOMPOSITOR_OPENGLRESOURCES_HH
