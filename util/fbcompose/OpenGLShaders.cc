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

using namespace FbCompositor;


namespace {
    //--- SHADER SOURCES -------------------------------------------------------

    /** Head of the vertex shader source code. */
    static const GLchar VERTEX_SHADER_HEAD[] = "\
        #version 120                                                         \n\
                                                                             \n\
        attribute vec2 fb_InitMainTexCoord;                                  \n\
        attribute vec2 fb_InitPrimPos;                                       \n\
        attribute vec2 fb_InitShapeTexCoord;                                 \n\
                                                                             \n\
        varying vec2 fb_MainTexCoord;                                        \n\
        varying vec2 fb_ShapeTexCoord;                                       \n\
    ";

    /** Middle of the vertex shader source code. */
    static const GLchar VERTEX_SHADER_MIDDLE[] = "\
        void main() {                                                        \n\
            gl_Position = vec4(fb_InitPrimPos, 0.0, 1.0);                    \n\
            fb_MainTexCoord = fb_InitMainTexCoord;                           \n\
            fb_ShapeTexCoord = fb_InitShapeTexCoord;                         \n\
    ";

    /** Tail of the vertex shader source code. */
    static const GLchar VERTEX_SHADER_TAIL[] = "\
        }                                                                    \n\
    ";


    /** Head of the fragment shader source code. */
    static const GLchar FRAGMENT_SHADER_HEAD[] = "\
        #version 120                                                         \n\
                                                                             \n\
        uniform float fb_Alpha;                                              \n\
        uniform sampler2D fb_MainTexture;                                    \n\
        uniform sampler2D fb_ShapeTexture;                                   \n\
                                                                             \n\
        varying vec2 fb_MainTexCoord;                                        \n\
        varying vec2 fb_ShapeTexCoord;                                       \n\
    ";

    /** Middle of the fragment shader source code. */
    static const GLchar FRAGMENT_SHADER_MIDDLE[] = "\
        void main() {                                                        \n\
            gl_FragColor = texture2D(fb_MainTexture, fb_MainTexCoord)        \n\
                           * texture2D(fb_ShapeTexture, fb_ShapeTexCoord)    \n\
                           * vec4(1.0, 1.0, 1.0, fb_Alpha);                  \n\
    ";

    /** Tail of the fragment shader source code. */
    static const GLchar FRAGMENT_SHADER_TAIL[] = "\
        }                                                                    \n\
    ";
}


//--- ACCESSORS ----------------------------------------------------------------

// Returns the head of the fragment shader source code.
const GLchar *OpenGLShaders::fragmentShaderHead() {
    return FRAGMENT_SHADER_HEAD;
}

// Returns the middle of the fragment shader source code.
const GLchar *OpenGLShaders::fragmentShaderMiddle() {
    return FRAGMENT_SHADER_MIDDLE;
}

// Returns the tail of the fragment shader source code.
const GLchar *OpenGLShaders::fragmentShaderTail() {
    return FRAGMENT_SHADER_TAIL;
}

// Returns the head of the vertex shader source code.
const GLchar *OpenGLShaders::vertexShaderHead() {
    return VERTEX_SHADER_HEAD;
}

// Returns the middle of the vertex shader source code.
const GLchar *OpenGLShaders::vertexShaderMiddle() {
    return VERTEX_SHADER_MIDDLE;
}

// Returns the tail of the vertex shader source code.
const GLchar *OpenGLShaders::vertexShaderTail() {
    return VERTEX_SHADER_TAIL;
}
