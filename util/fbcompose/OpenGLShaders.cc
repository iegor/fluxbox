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

#include "Logging.hh"
#include "OpenGLPlugin.hh"

#include <sstream>

using namespace FbCompositor;


namespace {
    //--- CONSTANTS ------------------------------------------------------------

    /** Size of the info log buffer. */
    static const int INFO_LOG_BUFFER_SIZE = 256;


    //--- VERTEX SHADER SOURCE -------------------------------------------------

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


    //--- FRAGMENT SHADER SOURCE -----------------------------------------------

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


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLShaderProgram::OpenGLShaderProgram(const std::vector<BasePlugin*> &plugins) {
    std::stringstream ss;

    // Assemble vertex shader.
    ss.str("");
    ss << VERTEX_SHADER_HEAD;
    for (size_t i = 0; i < plugins.size(); i++) {
        ss << ((OpenGLPlugin*)(plugins[i]))->vertexShader() << "\n";
    }
    ss << VERTEX_SHADER_MIDDLE;
    for (size_t i = 0; i < plugins.size(); i++) {
        ss << ((OpenGLPlugin*)(plugins[i]))->pluginName() << "();\n";
    }
    ss << VERTEX_SHADER_TAIL;
    m_vertexShader = createShader(GL_VERTEX_SHADER, ss.str().length(), ss.str().c_str());

    // Assemble fragment shader.
    ss.str("");
    ss << FRAGMENT_SHADER_HEAD;
    for (size_t i = 0; i < plugins.size(); i++) {
        ss << ((OpenGLPlugin*)(plugins[i]))->fragmentShader() << "\n";
    }
    ss << FRAGMENT_SHADER_MIDDLE;
    for (size_t i = 0; i < plugins.size(); i++) {
        ss << ((OpenGLPlugin*)(plugins[i]))->pluginName() << "();\n";
    }
    ss << FRAGMENT_SHADER_TAIL;
    m_fragmentShader = createShader(GL_FRAGMENT_SHADER, ss.str().length(), ss.str().c_str());

    // Create shader program.
    m_shaderProgram = createShaderProgram(m_vertexShader, m_fragmentShader);

    // Initialize attribute locations.
    m_mainTexCoordAttrib = getAttributeLocation("fb_InitMainTexCoord");
    m_primPosAttrib = getAttributeLocation("fb_InitPrimPos");
    m_shapeTexCoordAttrib = getAttributeLocation("fb_InitShapeTexCoord");

    // Initialize uniform locations.
    m_alphaUniform = getUniformLocation("fb_Alpha");
    m_mainTexUniform = getUniformLocation("fb_MainTexture");
    m_shapeTexUniform = getUniformLocation("fb_ShapeTexture");
}

// Destructor.
OpenGLShaderProgram::~OpenGLShaderProgram() {
    glDetachShader(m_shaderProgram, m_vertexShader);
    glDetachShader(m_shaderProgram, m_fragmentShader);

    glDeleteProgram(m_shaderProgram);
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Creates a shader.
GLuint OpenGLShaderProgram::createShader(GLenum shaderType, GLint sourceLength, const GLchar *source) {
    // Determine shader type.
    FbTk::FbString shaderName;
    if (shaderType == GL_VERTEX_SHADER) {
        shaderName = "vertex";
    } else if (shaderType == GL_GEOMETRY_SHADER) {  // For completeness.
        shaderName = "geometry";
    } else if (shaderType == GL_FRAGMENT_SHADER) {
        shaderName = "fragment";
    } else {
        throw InitException("createShader() was given an invalid shader type.");
    }

    // Create and compile.
    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        std::stringstream ss;
        ss << "Could not create " << shaderName << " shader.";
        throw InitException(ss.str());
    }

    glShaderSource(shader, 1, &source, &sourceLength);
    glCompileShader(shader);

    // Check for errors.
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus) {
        GLsizei infoLogSize;
        GLchar infoLog[INFO_LOG_BUFFER_SIZE];
        glGetShaderInfoLog(shader, INFO_LOG_BUFFER_SIZE, &infoLogSize, infoLog);

        std::stringstream ss;
        ss << "Error in compilation of the " << shaderName << " shader: "
           << std::endl << (const char*)(infoLog);
        throw InitException(ss.str());
    }

    return shader;
}

// Creates a shader program.
GLuint OpenGLShaderProgram::createShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    if (!program) {
        throw InitException("Cannot create a shader program.");
    }

    // Link program.
    if (vertexShader) {
        glAttachShader(program, vertexShader);
    }
    if (fragmentShader) {
        glAttachShader(program, fragmentShader);
    }
    glLinkProgram(program);

    // Check for errors.
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus) {
        GLsizei infoLogSize;
        GLchar infoLog[INFO_LOG_BUFFER_SIZE];
        glGetProgramInfoLog(program, INFO_LOG_BUFFER_SIZE, &infoLogSize, infoLog);

        std::stringstream ss;
        ss << "Error in linking of the shader program: " << std::endl << (const char*)(infoLog);
        throw InitException(ss.str());
    }

    return program;
}
