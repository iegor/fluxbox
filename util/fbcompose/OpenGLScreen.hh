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
#include "BaseCompWindow.hh"
#include "Exceptions.hh"
#include "OpenGLWindow.hh"

#include <GL/glew.h>
#include <GL/glx.h>


namespace FbCompositor {

    class BaseCompWindow;
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
        OpenGLScreen(int screenNumber);

        /** Destructor. */
        ~OpenGLScreen();


        //--- OTHER INITIALIZATION ---------------------------------------------

        /** Initializes all of the windows on the screen. */
        void initWindows();


        //--- SCREEN RENDERING -------------------------------------------------

        /** Renders the screen's contents. */
        void renderScreen();

    protected:
        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Creates a window object from its XID. */
        BaseCompWindow *createWindowObject(Window window);

        /** Cleans up a window object before it is deleted. */
        void cleanupWindowObject(BaseCompWindow *window);

        /** Damages a window object. */
        void damageWindowObject(BaseCompWindow *window);

        /** Maps a window object. */
        void mapWindowObject(BaseCompWindow *window);

        /** Updates window's configuration. */
        void reconfigureWindowObject(BaseCompWindow *window);

        /** Unmaps a window object. */
        void unmapWindowObject(BaseCompWindow *window);

        /** Updates the value of some window's property. */
        void updateWindowObjectProperty(BaseCompWindow *window, Atom property, int state);

    private:
        //--- CONSTANTS --------------------------------------------------------
        
        /** Size of the info log buffer. */
        static const int INFO_LOG_BUFFER_SIZE = 256;


        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Initializes GLEW. */
        void initGlew() throw(InitException);

        /** Initializes the rendering context. */
        void initRenderingContext() throw(InitException);

        /** Initializes the rendering surface. */
        void initRenderingSurface() throw(InitException);

        /** Initializes shaders. */
        void initShaders() throw(InitException);


        //--- CONVENIENCE OPENGL WRAPPERS --------------------------------------

        /** Creates a shader. */
        GLuint createShader(GLenum shaderType, GLint sourceLength, const GLchar *source) throw(InitException);

        /** Creates a shader program. */
        GLuint createShaderProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader) throw(InitException);


        //--- RENDERING FUNCTIONS ----------------------------------------------

        /** A function to render a particular window onto the screen. */
        void renderWindow(OpenGLWindow &window);


        //--- MAIN RENDERING-RELATED VARIABLES ---------------------------------

        /** The main FBConfig. */
        GLXFBConfig m_fbConfig;

        /** The GLX context. */
        GLXContext m_glxContext;

        /** GLX handle to the rendering window. */
        GLXWindow m_glxRenderingWindow;

        /** Rendering window. */
        Window m_renderingWindow;


        //--- SHADER-RELATED ---------------------------------------------------

        /** The vertex shader. */
        GLuint m_vertexShader;

        /** The fragment shader. */
        GLuint m_fragmentShader;
        
        /** The shader program. */
        GLuint m_shaderProgram;
    };
}


#endif  // FBCOMPOSITOR_XRENDERAUTOSCREEN_HH