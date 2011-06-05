/** OpenGLScreen.cc file for the fluxbox compositor. */

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


#include "OpenGLScreen.hh"
#include "OpenGLWindow.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>

#include <list>
#include <iostream>
#include <sstream>
#include <string>

#include <cstring>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLScreen::OpenGLScreen(int screenNumber) :
    BaseScreen(screenNumber) {

    initRenderingContext();
    checkOpenGLVersion();
    initRenderingSurface();
    initShaders();
}

// Destructor.
OpenGLScreen::~OpenGLScreen() {
    XUnmapWindow(display(), m_renderingWindow);

    glDetachShader(m_shaderProgram, m_vertexShader);
    glDetachShader(m_shaderProgram, m_fragmentShader);
    glDeleteProgram(m_shaderProgram);
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);

    glXDestroyWindow(display(), m_glxRenderingWindow);
    glXDestroyContext(display(), m_glxContext);
    XDestroyWindow(display(), m_renderingWindow);
}


//--- OTHER INITIALIZATION ---------------------------------------------

// Initializes all of the windows on the screen.
void OpenGLScreen::initWindows() {
    Window root;
    Window parent;
    Window *children;
    unsigned int childCount;

    XQueryTree(display(), rootWindow().window(), &root, &parent, &children, &childCount);
    for (unsigned int i = 0; i < childCount; i++) {
        createWindow(children[i]);
    }

    if (children) {
        XFree(children);
    }
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Initializes the rendering context.
void OpenGLScreen::initRenderingContext() throw(InitException) {
    // Selecting the framebuffer configuration.
    static const int PREFERRED_FBCONFIG_ATTRIBUTES[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER, true,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        None
    };

    int nConfigs;
    GLXFBConfig *fbConfigs = glXChooseFBConfig(display(), screenNumber(), PREFERRED_FBCONFIG_ATTRIBUTES, &nConfigs);
    if (!fbConfigs) {
        // TODO: Better failure handling (i.e. fallback configurations).
        throw InitException("Screen does not support the required GLXFBConfig.");
    }
    m_fbConfig = fbConfigs[0];

    // Creating the GLX rendering context.
    m_glxContext = glXCreateNewContext(display(), m_fbConfig, GLX_RGBA_TYPE, NULL, True);
    if (!m_glxContext) {
        throw InitException("Cannot create the OpenGL rendering context.");
    }
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);

    // Initialize GLEW.
    GLenum glewErr = glewInit();
    if(glewErr != GLEW_OK) {
        std::stringstream ss;
        ss << "GLEW Error: " << (const char*)(glewGetErrorString(glewErr));
        throw InitException(ss.str().c_str());
    }
}

// Checks for the appropriate OpenGL version.
void OpenGLScreen::checkOpenGLVersion() throw(InitException) {
    if (!GLEW_VERSION_2_1) {
        throw InitException("OpenGL 2.1 not available.");
    }
}

// Initializes the rendering surface.
void OpenGLScreen::initRenderingSurface() throw(InitException) {
    // Creating an X window for rendering.
    Window compOverlay = XCompositeGetOverlayWindow(display(), rootWindow().window());

    XVisualInfo *visualInfo = glXGetVisualFromFBConfig(display(), m_fbConfig);
    Colormap colormap = XCreateColormap(display(), rootWindow().window(), visualInfo->visual, AllocNone);

    XSetWindowAttributes wa;
    wa.colormap = colormap;
    long waMask = CWColormap;

    m_renderingWindow = XCreateWindow(display(), compOverlay, 0, 0, rootWindow().width(), rootWindow().height(), 0,
                                      visualInfo->depth, InputOutput, visualInfo->visual, waMask, &wa);
    XmbSetWMProperties(display(), m_renderingWindow, "fbcompose", "fbcompose", NULL, 0, NULL, NULL, NULL);
    XMapWindow(display(), m_renderingWindow);

    // Make sure the overlays do not consume any input events.
    XserverRegion emptyRegion = XFixesCreateRegion(display(), NULL, 0);
    XFixesSetWindowShapeRegion(display(), compOverlay, ShapeInput, 0, 0, emptyRegion);
    XFixesSetWindowShapeRegion(display(), m_renderingWindow, ShapeInput, 0, 0, emptyRegion);
    XFixesDestroyRegion(display(), emptyRegion);

    // Creating a GLX handle for the above window.
    m_glxRenderingWindow = glXCreateWindow(display(), m_fbConfig, m_renderingWindow, NULL);
    if (!m_glxRenderingWindow) {
        throw InitException("Cannot create the rendering surface.");
    }
}

// Initializes shaders.
void OpenGLScreen::initShaders() throw(InitException) {
    // Vertex shader source code (TODO: move somewhere else when everything is working).
    GLchar vShaderSource[] =
        "#version 120\n"
        "\n"
        "attribute vec2 fb_InitPointPos;\n"
        "attribute vec2 fb_InitTexCoord;\n"
        "\n"
        "varying vec2 fb_TexCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = vec4(fb_InitPointPos, 0.0, 1.0);\n"
        "    fb_TexCoord = fb_InitTexCoord;\n"
        "}";
    GLint vShaderSourceLength = (GLint)strlen(vShaderSource);

    // Fragment shader source code (TODO: move somewhere else when everything is working).
    GLchar fShaderSource[] =
        "#version 120\n"
        "\n"
        "uniform sampler2D fb_Texture;\n"
        "\n"
        "varying vec2 fb_TexCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(fb_Texture, fb_TexCoord);\n"
        "}";
    GLint fShaderSourceLength = (GLint)strlen(fShaderSource);

    m_vertexShader = createShader(GL_VERTEX_SHADER, vShaderSourceLength, vShaderSource);
    m_fragmentShader = createShader(GL_FRAGMENT_SHADER, fShaderSourceLength, fShaderSource);
    m_shaderProgram = createShaderProgram(m_vertexShader, 0, m_fragmentShader);
}


//--- CONVENIENCE OPENGL WRAPPERS ----------------------------------------------

// Creates a shader.
GLuint OpenGLScreen::createShader(GLenum shaderType, GLint sourceLength, const GLchar *source) throw(InitException) {
    std::string shaderName;
    if (shaderType == GL_VERTEX_SHADER) {
        shaderName = "vertex";
    } else if (shaderType == GL_GEOMETRY_SHADER) {
        shaderName = "geometry";
    } else if (shaderType == GL_FRAGMENT_SHADER) {
        shaderName = "fragment";
    } else {
        throw InitException("createShader() was given an invalid shader type.");
    }

    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        std::stringstream ss;
        ss << "Could not create " << shaderName << " shader.";
        throw InitException(ss.str());
    }

    glShaderSource(shader, 1, &source, &sourceLength);
    glCompileShader(shader);

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus) {
        GLsizei infoLogSize;
        GLchar infoLog[INFO_LOG_BUFFER_SIZE];
        glGetShaderInfoLog(shader, INFO_LOG_BUFFER_SIZE, &infoLogSize, infoLog);

        std::stringstream ss;
        ss << "Error in compilation of the " << shaderName << " shader: " << (const char*)(infoLog);
        throw InitException(ss.str());
    }

    return shader;
}

// Creates a shader program.
GLuint OpenGLScreen::createShaderProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader) throw(InitException) {
    GLuint program = glCreateProgram();
    if (!program) {
        throw InitException("Cannot create a shader program.");
    }

    if (vertexShader) {
        glAttachShader(program, vertexShader);
    }
    if (geometryShader) {
        glAttachShader(program, geometryShader);
    }
    if (fragmentShader) {
        glAttachShader(program, fragmentShader);
    }
    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus) {
        GLsizei infoLogSize;
        GLchar infoLog[INFO_LOG_BUFFER_SIZE];
        glGetProgramInfoLog(program, INFO_LOG_BUFFER_SIZE, &infoLogSize, infoLog);

        std::stringstream ss;
        ss << "Error in linking of the shader program: " << (const char*)(infoLog);
        throw InitException(ss.str());
    }

    return program;
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Creates a window object from its XID.
BaseCompWindow *OpenGLScreen::createWindowObject(Window window) {
    OpenGLWindow *newWindow = new OpenGLWindow(window);
    return newWindow;
}

// Cleans up a window object before it is deleted.
void OpenGLScreen::cleanupWindowObject(BaseCompWindow *window) {
    BaseScreen::cleanupWindowObject(window);
}

// Damages a window object.
void OpenGLScreen::damageWindowObject(BaseCompWindow *window) {
    BaseScreen::damageWindowObject(window);
}

// Maps a window object.
void OpenGLScreen::mapWindowObject(BaseCompWindow *window) {
    BaseScreen::mapWindowObject(window);
}

// Updates window's configuration.
void OpenGLScreen::reconfigureWindowObject(BaseCompWindow *window) {
    BaseScreen::reconfigureWindowObject(window);
    (dynamic_cast<OpenGLWindow*>(window))->updateArrays();
}

// Unmaps a window object.
void OpenGLScreen::unmapWindowObject(BaseCompWindow *window) {
    BaseScreen::unmapWindowObject(window);
}

// Updates the value of some window's property.
void OpenGLScreen::updateWindowObjectProperty(BaseCompWindow *window, Atom property, int state) {
    BaseScreen::updateWindowObjectProperty(window, property, state);
}


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void OpenGLScreen::renderScreen() {
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);
    glUseProgram(m_shaderProgram);

    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        if ((*it)->isMapped()) {
            renderWindow(*(dynamic_cast<OpenGLWindow*>(*it)));
        }
        it++;
    }

    glXSwapBuffers(display(), m_glxRenderingWindow);
}

// A function to render a particular window onto the screen.
void OpenGLScreen::renderWindow(OpenGLWindow &window) {
    // Temporary hack until we stop tracking windows belonging to the compositor.
    if (window.window() == 102) {
        return;
    }

    // TODO: Fix the content refresh issue.
    window.updateContents();

    // Load window position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, window.windowPosBuffer());

    GLuint windowPosAttrib = glGetAttribLocation(m_shaderProgram, "fb_InitPointPos");
    glVertexAttribPointer(windowPosAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(windowPosAttrib);

    // Load texture position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, window.texturePosBuffer());

    GLuint texPosAttrib = glGetAttribLocation(m_shaderProgram, "fb_InitTexCoord");
    glVertexAttribPointer(texPosAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(texPosAttrib);

    // Load element array.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window.elementBuffer());

    // Set up the texture uniforms.
    GLuint texturePos = glGetUniformLocation(m_shaderProgram, "fb_Texture");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, window.contentTexture());
    glUniform1i(texturePos, 0);

    // Rendering.
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0);

    // Cleanup.
    glDisableVertexAttribArray(windowPosAttrib);
    glDisableVertexAttribArray(texPosAttrib);
}
