/** OpenGLScreen.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERAUTOSCREEN_HH
#define FBCOMPOSITOR_XRENDERAUTOSCREEN_HH


#include "BaseScreen.hh"
#include "Exceptions.hh"
#include "OpenGLPlugin.hh"
#include "OpenGLShaders.hh"
#include "OpenGLWindow.hh"

#include <GL/glxew.h>
#include <GL/glew.h>
#include <GL/glx.h>


namespace FbCompositor {

    class BaseCompWindow;
    class CompositorConfig;
    class InitException;
    class OpenGLScreen;
    class OpenGLWindow;


    /**
     * Manages screen in OpenGL rendering mode.
     */
    class OpenGLScreen : public BaseScreen {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLScreen(int screenNumber, const CompositorConfig &config);

        /** Destructor. */
        ~OpenGLScreen();


        //--- DEFAULT OPENGL OBJECT ACCESSORS ----------------------------------

        /** \returns the default element buffer (rectangle, corners in order of NW, NE, SW, SE). */
        OpenGLBufferPtr defaultElementBuffer() const;

        /** \returns the default primitive position buffer (four corners of the root window). */
        OpenGLBufferPtr defaultPrimPosBuffer() const;

        /** \returns the default texture position buffer (the whole texture). */
        OpenGLBufferPtr defaultTexCoordBuffer() const;


        /** \returns the default black texture. */
        OpenGLTexturePtr blackTexture() const;

        /** \returns the default white texture. */
        OpenGLTexturePtr whiteTexture() const;


        //--- OTHER ACCESSORS --------------------------------------------------

        /** \return the main GLX context. */
        GLXContext context() const;

        /** \returns the main GLXFBConfig. */
        GLXFBConfig fbConfig() const;

        /** \returns maximum supported texture size. */
        int maxTextureSize() const;


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Notifies the screen of the background change. */
        void setRootPixmapChanged();

        /** Notifies the screen of a root window change. */
        void setRootWindowSizeChanged();


        //--- SCREEN RENDERING -------------------------------------------------

        /** Renders the screen's contents. */
        void renderScreen();


    protected:
        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Creates a window object from its XID. */
        BaseCompWindow *createWindowObject(Window window);


    private:
        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Creates OpenGL resources. */
        void createResources();

        /** Early initialization of GLX function pointers. */
        void earlyInitGLXPointers();

        /** Finds the maximum usable texture size. */
        void findMaxTextureSize();

        /** Finishes the initialization of the rendering context and surface. */
        void finishRenderingInit();

        /** Initializes GLEW. */
        void initGlew();

        /** Finish plugin initialization. */
        void initPlugins();

        /** Initializes the rendering context. */
        void initRenderingContext();

        /** Initializes the rendering surface. */
        void initRenderingSurface();


        //--- OTHER FUNCTIONS --------------------------------------------------

        /** Renews the background texture. */
        void updateBackgroundTexture();

        /** React to the geometry change of the root window. */
        void updateOnRootWindowResize();


        //--- RENDERING FUNCTIONS ----------------------------------------------

        /** Render the desktop background. */
        void renderBackground();

        /** Perform extra rendering jobs from plugins. */
        void renderExtraJobs();

        /** Render the reconfigure rectangle. */
        void renderReconfigureRect();

        /** Render a particular window onto the screen. */
        void renderWindow(OpenGLWindow &window);

        /** Execute a given rendering job. */
        void executeRenderingJob(OpenGLRenderingJob job);

        /** Render something onto the screen. */
        void render(GLenum renderingMode, OpenGLBufferPtr primPosBuffer,
                    OpenGLBufferPtr mainTexCoordBuffer, OpenGLTexturePtr mainTexture,
                    OpenGLBufferPtr shapeTexCoordBuffer, OpenGLTexturePtr shapeTexture,
                    OpenGLBufferPtr elementBuffer, GLuint elementCount, GLfloat alpha);


        //--- MAIN RENDERING-RELATED VARIABLES ---------------------------------

        /** The main FBConfig. */
        GLXFBConfig m_fbConfig;

        /** The GLX context. */
        GLXContext m_glxContext;

        /** Shaders. */
        OpenGLShaderProgramPtr m_shaderProgram;


        /** GLX handle to the rendering window. */
        GLXWindow m_glxRenderingWindow;

        /** Rendering window. */
        Window m_renderingWindow;

        /** Whether the root window has changed since the last update. */
        bool m_rootWindowChanged;


        //--- DESKTOP BACKGROUND RELATED ---------------------------------------

        /** The background texture. */
        OpenGLTexturePtr m_backgroundTexture;

        /** Whether the background changed since the last update. */
        bool m_backgroundChanged;


        //--- DEFAULT OPENGL ELEMENTS ------------------------------------------

        /** Default element buffer. */
        OpenGLBufferPtr m_defaultElementBuffer;

        /** Default primitive position buffer. */
        OpenGLBufferPtr m_defaultPrimPosBuffer;

        /** Default texture position buffer. */
        OpenGLBufferPtr m_defaultTexCoordBuffer;


        /** Default black texture. */
        OpenGLTexturePtr m_blackTexture;

        /** Default white texture. */
        OpenGLTexturePtr m_whiteTexture;


        //--- RESIZE FRAME RELATED ---------------------------------------------

        /** The reconfigure rectangle element buffer. */
        OpenGLBufferPtr m_reconfigureRectElementBuffer;

        /** The reconfigure rectangle primitive position array buffer. */
        OpenGLBufferPtr m_reconfigureRectLinePosBuffer;


        //--- OTHER VARIABLES --------------------------------------------------

        /** Whether we have a double-buffered window. */
        bool m_haveDoubleBuffering;

        /** Maximum texture size. */
        int m_maxTextureSize;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the default black texture.
    inline OpenGLTexturePtr OpenGLScreen::blackTexture() const {
        return m_blackTexture;
    }

    // Return the main GLX context.
    inline GLXContext OpenGLScreen::context() const {
        return m_glxContext;
    }

    // Returns the default element buffer.
    inline OpenGLBufferPtr OpenGLScreen::defaultElementBuffer() const {
        return m_defaultElementBuffer;
    }

    // Returns the default primitive position buffer.
    inline OpenGLBufferPtr OpenGLScreen::defaultPrimPosBuffer() const {
        return m_defaultPrimPosBuffer;
    }

    // Returns the default texture position buffer.
    inline OpenGLBufferPtr OpenGLScreen::defaultTexCoordBuffer() const {
        return m_defaultTexCoordBuffer;
    }

    // Returns the main GLXFBConfig.
    inline GLXFBConfig OpenGLScreen::fbConfig() const {
        return m_fbConfig;
    }

    // Returns maximum supported texture size.
    inline int OpenGLScreen::maxTextureSize() const {
        return m_maxTextureSize;
    }

    // Returns the default white texture.
    inline OpenGLTexturePtr OpenGLScreen::whiteTexture() const {
        return m_whiteTexture;
    }
}

#endif  // FBCOMPOSITOR_XRENDERAUTOSCREEN_HH
