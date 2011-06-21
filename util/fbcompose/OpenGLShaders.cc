/** OpenGLShaders.cc file for the fluxbox compositor. */

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

#include "OpenGLShaders.hh"

#include <cstring>

using namespace FbCompositor;


namespace {
    //--- SHADER SOURCES -------------------------------------------------------

    /** Vertex shader source code. */
    static const GLchar VERTEX_SHADER_SOURCE[] = "\
        #version 120                                                         \n\
                                                                             \n\
        attribute vec2 fb_InitPointPos;                                      \n\
        attribute vec2 fb_InitTexCoord;                                      \n\
                                                                             \n\
        varying vec2 fb_TexCoord;                                            \n\
                                                                             \n\
        void main() {                                                        \n\
            gl_Position = vec4(fb_InitPointPos, 0.0, 1.0);                   \n\
            fb_TexCoord = fb_InitTexCoord;                                   \n\
        }                                                                    \n\
    ";

    /** Fragment shader source code. */
    static const GLchar FRAGMENT_SHADER_SOURCE[] = "\
        #version 120                                                         \n\
                                                                             \n\
        uniform float fb_Alpha;                                              \n\
        uniform sampler2D fb_Texture;                                        \n\
                                                                             \n\
        varying vec2 fb_TexCoord;                                            \n\
                                                                             \n\
        void main() {                                                        \n\
            gl_FragColor = texture2D(fb_Texture, fb_TexCoord)                \n\
                    * vec4(1.0, 1.0, 1.0, fb_Alpha);                         \n\
        }                                                                    \n\
    ";
}


//--- ACCESSORS ----------------------------------------------------------------

// Returns the the fragment shader source code.
const GLchar *OpenGLShaders::fragmentShaderSource() throw() {
    return FRAGMENT_SHADER_SOURCE;
}

// Returns the length of the fragment shader source code.
int OpenGLShaders::fragmentShaderSourceLength() throw() {
    static GLint sourceLength = (GLint)strlen(FRAGMENT_SHADER_SOURCE);
    return sourceLength;
}


// Returns the the vertex shader source code.
const GLchar *OpenGLShaders::vertexShaderSource() throw() {
    return VERTEX_SHADER_SOURCE;
}

// Returns the length of the vertex shader source code.
int OpenGLShaders::vertexShaderSourceLength() throw() {
    static GLint sourceLength = (GLint)strlen(VERTEX_SHADER_SOURCE);
    return sourceLength;
}
