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

#include "config.h"

#ifdef USE_OPENGL_COMPOSITING


#include "BaseScreen.hh"
#include "Exceptions.hh"
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
        OpenGLScreen(int screenNumber, const CompositorConfig &config) throw(InitException);

        /** Destructor. */
        ~OpenGLScreen() throw();


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Notifies the screen of the background change. */
        void setRootPixmapChanged() throw();

        /** Notifies the screen of a root window change. */
        void setRootWindowSizeChanged() throw();


        //--- SCREEN RENDERING -------------------------------------------------

        /** Renders the screen's contents. */
        void renderScreen();


    protected:
        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Creates a window object from its XID. */
        BaseCompWindow *createWindowObject(Window window);


    private:
        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Creates the background texture. */
        void createBackgroundTexture() throw(InitException);

        /** Creates default texture rendering buffers. */
        void createDefaultElements() throw(InitException);

        /** Creates all elements, needed to draw the reconfigure rectangle. */
        void createReconfigureRectElements() throw(InitException);

        /** Early initialization of GLX function pointers. */
        void earlyInitGLXPointers() throw(InitException);

        /** Finishes the initialization of the rendering context and surface. */
        void finishRenderingInit() throw(InitException);

        /** Initializes GLEW. */
        void initGlew() throw(InitException);

        /** Finish plugin initialization. */
        void initPlugins() throw(InitException);

        /** Initializes the rendering context. */
        void initRenderingContext() throw(InitException);

        /** Initializes the rendering surface. */
        void initRenderingSurface() throw(InitException);

        /** Initializes shaders. */
        void initShaders() throw(InitException);


        //--- OTHER FUNCTIONS --------------------------------------------------

        /** Renews the background texture. */
        void updateBackgroundTexture() throw();

        /** React to the geometry change of the root window. */
        void updateOnRootWindowResize() throw();


        //--- CONVENIENCE OPENGL WRAPPERS --------------------------------------

        /** Creates a shader. */
        GLuint createShader(GLenum shaderType, GLint sourceLength, const GLchar *source) throw(InitException);

        /** Creates a shader program. */
        GLuint createShaderProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader) throw(InitException);


        //--- RENDERING FUNCTIONS ----------------------------------------------

        /** Render the desktop background. */
        void renderBackground();

        /** Render the reconfigure rectangle. */
        void renderReconfigureRect();

        /** Render a particular window onto the screen. */
        void renderWindow(OpenGLWindow &window);


        /** Render something onto the screen. */
        void render(GLenum renderingMode, GLuint primPosBuffer, GLuint texturePosBuffer,
                    GLuint elementBuffer, GLuint elementCount, GLuint texture, GLfloat alpha);


        //--- MAIN RENDERING-RELATED VARIABLES ---------------------------------

        /** The main FBConfig. */
        GLXFBConfig m_fbConfig;

        /** The GLX context. */
        GLXContext m_glxContext;


        /** GLX handle to the rendering window. */
        GLXWindow m_glxRenderingWindow;

        /** Rendering window. */
        Window m_renderingWindow;

        /** Whether the root window has changed since the last update. */
        bool m_rootWindowChanged;


        //--- DESKTOP BACKGROUND RELATED ---------------------------------------

        /** The background texture. */
        GLuint m_backgroundTexture;

        /** Whether the background changed since the last update. */
        bool m_backgroundChanged;


        //--- RESIZE FRAME RELATED ---------------------------------------------

        /** The reconfigure rectangle element buffer. */
        GLuint m_reconfigureRectElementBuffer;

        /** The reconfigure rectangle primitive position array buffer. */
        GLuint m_reconfigureRectLinePosBuffer;


        //--- DEFAULT OPENGL ELEMENTS ------------------------------------------

        /** Blank texture. */
        GLuint m_blankTexture;

        /** Default element buffer. */
        GLuint m_defaultElementBuffer;

        /** Default primitive position buffer. */
        GLuint m_defaultPrimPosBuffer;

        /** Default texture position buffer. */
        GLuint m_defaultTexPosBuffer;


        //--- SHADER-RELATED ---------------------------------------------------

        /** The vertex shader. */
        GLuint m_vertexShader;

        /** The fragment shader. */
        GLuint m_fragmentShader;
        
        /** The shader program. */
        GLuint m_shaderProgram;


        //--- OTHER VARIABLES --------------------------------------------------

        /** Whether we have a double-buffered window. */
        bool m_haveDoubleBuffering;
    };
}

#endif  // USE_OPENGL_COMPOSITING

#endif  // FBCOMPOSITOR_XRENDERAUTOSCREEN_HH
