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

#include "config.h"

#ifdef USE_OPENGL_COMPOSITING


#include "BasePlugin.hh"
#include "Constants.hh"
#include "Exceptions.hh"

#include "FbTk/FbString.hh"

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>


namespace FbCompositor {

    class BasePlugin;
    class BaseScreen;
    class InitException;


    /**
     * Plugin for OpenGL renderer.
     */
    class OpenGLPlugin : public BasePlugin {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) throw(InitException);

        /** Destructor. */
        ~OpenGLPlugin() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the additional source code for the fragment shader. */
        virtual const char *fragmentShader() const throw() = 0;

        /** \returns the additional source code for the vertex shader. */
        virtual const char *vertexShader() const throw() = 0;


        //--- PLUGIN ACTIONS ---------------------------------------------------

        /** Pre-rendering actions (uniform setup etc). */
        virtual void preRenderActions(GLuint shaderProgram) throw();

        /** Post-rendering actions (plugin-specific cleanup etc). */
        virtual void postRenderActions(GLuint shaderProgram) throw();
    };
}

#endif  // USE_OPENGL_COMPOSITING

#endif  // FBCOMPOSITOR_OPENGLPLUGIN_HH
