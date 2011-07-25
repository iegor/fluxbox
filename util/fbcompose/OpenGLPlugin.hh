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
#include "Enumerations.hh"
#include "Exceptions.hh"
#include "OpenGLResources.hh"
#include "OpenGLShaders.hh"

#include "FbTk/FbString.hh"

#include <GL/glew.h>
#include <GL/gl.h>

#include <X11/Xlib.h>

#include <vector>


namespace FbCompositor {

    class BasePlugin;
    class BaseScreen;
    class InitException;
    class OpenGLScreen;
    class OpenGLWindow;


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
    };


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

        /** Pre background rendering actions. */
        virtual void preBackgroundRenderActions();

        /** Post background rendering actions. */
        virtual void postBackgroundRenderActions();

        /** Pre reconfigure rectangle rendering actions. */
        virtual void preReconfigureRectRenderActions(XRectangle reconfigureRect);

        /** Post reconfigure rectangle rendering actions. */
        virtual void postReconfigureRectRenderActions(XRectangle reconfigureRect);


        /** Extra rendering job before window rendering. */
        virtual OpenGLRenderingJob extraPreWindowRenderJob(const OpenGLWindow &window);

        /** Pre window rendering actions. */
        virtual void preWindowRenderActions(const OpenGLWindow &window);

        /** Post window rendering actions. */
        virtual void postWindowRenderActions(const OpenGLWindow &window);

        /** Extra rendering job after window rendering. */
        virtual OpenGLRenderingJob extraPostWindowRenderJob(const OpenGLWindow &window);


        /** Called before the extra rendering jobs are executed. */
        virtual void preExtraRenderingActions();

        /** \returns the number of extra rendering jobs the plugin will do. */
        virtual int extraRenderingJobCount();

        /** Initialize the specified extra rendering job. */
        virtual OpenGLRenderingJob extraRenderingJobInit(int jobId);

        /** Clean up after an extra rendering job. */
        virtual void extraRenderingJobCleanup(int jobId);

        /** Called after the extra rendering jobs are executed. */
        virtual void postExtraRenderingActions();
    };
}

#endif  // FBCOMPOSITOR_OPENGLPLUGIN_HH
