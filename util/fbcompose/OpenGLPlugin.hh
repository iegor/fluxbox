/** OpenGLPlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLPLUGIN_HH
#define FBCOMPOSITOR_OPENGLPLUGIN_HH


#include "BasePlugin.hh"
#include "OpenGLResources.hh"
#include "OpenGLShaders.hh"

#include "FbTk/Command.hh"
#include "FbTk/FbString.hh"

#include <GL/glew.h>
#include <GL/gl.h>

#include <X11/Xlib.h>

#include <algorithm>
#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class OpenGLScreen;
    class OpenGLWindow;


    //--- SUPPORTING STRUCTURES AND CLASSES ------------------------------------

    /** Rendering job initialization functor.  */
    typedef FbTk::Command<void> InitAction;

    /**
     * Null initialization action.
     */
    class NullInitAction : public InitAction {
    public :
        virtual ~NullInitAction() { }
        void execute() { }
    };


    /** Rendering job cleanup functor. */
    typedef FbTk::Command<void> CleanupAction;

    /** 
     * Null cleanup action.
     */
    class NullCleanupAction : public CleanupAction {
    public :
        virtual ~NullCleanupAction() { }
        void execute() { }
    };


    /**
     * Information about an extra rendering job.
     */
    struct OpenGLRenderingJob {
        OpenGLBufferPtr primPosBuffer;          ///< Primitive's position buffer.
        OpenGLBufferPtr mainTexCoordBuffer;     ///< Main texture's position buffer.
        OpenGLBufferPtr shapeTexCoordBuffer;    ///< Shape texture's position buffer.
        OpenGL2DTexturePtr shapeTexture;        ///< Shape texture.
        OpenGL2DTexturePtr mainTexture;         ///< Main texture.
        GLfloat alpha;                          ///< Alpha value.

        InitAction *initAction;                 ///< Rendering job initialization.
        CleanupAction *cleanupAction;           ///< Rendering job cleanup.
    };


    //--- OPENGL PLUGIN BASE CLASS ---------------------------------------------

    /**
     * Plugin for OpenGL renderer.
     */
    class OpenGLPlugin : public BasePlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args);

        /** Destructor. */
        virtual ~OpenGLPlugin();


        //--- OTHER INITIALIZATION ---------------------------------------------

        /** Initialize OpenGL-specific code. */
        virtual void initOpenGL(OpenGLShaderProgramPtr shaderProgram);


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the screen object, cast into the correct class. */
        const OpenGLScreen &openGLScreen() const;


        /** \returns the additional source code for the fragment shader. */
        virtual const char *fragmentShader() const = 0;

        /** \returns the additional source code for the vertex shader. */
        virtual const char *vertexShader() const = 0;


        //--- RENDERING ACTIONS ------------------------------------------------

        /** Background rendering initialization. */
        virtual void backgroundRenderInit(int partId);

        /** Background rendering cleanup. */
        virtual void backgroundRenderCleanup(int partId);

        /** Post background rendering actions. */
        virtual std::vector<OpenGLRenderingJob> postBackgroundRenderActions();


        /** Pre window rendering actions and jobs. */
        virtual std::vector<OpenGLRenderingJob> preWindowRenderActions(const OpenGLWindow &window);

        /** Window rendering initialization. */
        virtual void windowRenderInit(const OpenGLWindow &window, int partId);

        /** Window rendering cleanup. */
        virtual void windowRenderCleanup(const OpenGLWindow &window, int partId);

        /** Post window rendering actions and jobs. */
        virtual std::vector<OpenGLRenderingJob> postWindowRenderActions(const OpenGLWindow &window);


        /** Reconfigure rectangle rendering initialization. */
        virtual void recRectRenderInit(XRectangle recRect);

        /** Reconfigure rectangle rendering cleanup. */
        virtual void recRectRenderCleanup(XRectangle recRect);


        /** Extra rendering actions and jobs. */
        virtual std::vector<OpenGLRenderingJob> extraRenderingActions();

        /** Post extra rendering actions. */
        virtual void postExtraRenderingActions();


        /** Null rendering job initialization. */
        virtual void nullRenderInit();
    };
}

#endif  // FBCOMPOSITOR_OPENGLPLUGIN_HH
