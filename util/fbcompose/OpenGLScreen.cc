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

#include "FbTk/FbString.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>

#include <list>
#include <iostream>
#include <sstream>

#include <cstring>

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

// The preferred framebuffer configuration.
const int OpenGLScreen::PREFERRED_FBCONFIG_ATTRIBUTES[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, GL_TRUE,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    None
};

// Default element array for texture rendering.
const GLushort OpenGLScreen::DEFAULT_ELEMENT_ARRAY[] = {
    0, 1, 2, 3
};

// Default primitive position array for texture rendering.
const GLfloat OpenGLScreen::DEFAULT_PRIM_POS_ARRAY[] = {
    -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0
};

// Default texture position array for texture rendering.
const GLfloat OpenGLScreen::DEFAULT_TEX_POS_ARRAY[] = {
    0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0
};


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLScreen::OpenGLScreen(int screenNumber) :
    BaseScreen(screenNumber) {

    earlyInitGLXPointers();
    initRenderingContext();
    initRenderingSurface();
    initGlew();
    initShaders();
    createDefaultBuffers();
    createBackgroundTexture();
}

// Destructor.
OpenGLScreen::~OpenGLScreen() {
    XUnmapWindow(display(), m_renderingWindow);

    glDetachShader(m_shaderProgram, m_vertexShader);
    glDetachShader(m_shaderProgram, m_fragmentShader);
    glDeleteProgram(m_shaderProgram);
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);

    glDeleteTextures(1, &m_backgroundTexture);
    glDeleteBuffers(1, &m_defaultElementBuffer);
    glDeleteBuffers(1, &m_defaultPrimPosBuffer);
    glDeleteBuffers(1, &m_defaultTexPosBuffer);

    glXDestroyWindow(display(), m_glxRenderingWindow);
    glXDestroyContext(display(), m_glxContext);
    XDestroyWindow(display(), m_renderingWindow);
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Early initialization of GLX functions.
// We need GLX functions to create an OpenGL context and initialize GLEW. But
// using GLXEW zeroes the pointers, since glewInit() initializes them. And we
// have to use GLXEW to have easy access to GLX's extensions. So, this function
// performs minimal function initialization - just enough to create a context.
void OpenGLScreen::earlyInitGLXPointers() throw(InitException) {
    glXCreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)glXGetProcAddress((GLubyte*)"glXCreateNewContext");
    if (!glXCreateNewContext) {
        throw InitException("Cannot initialize glXCreateNewContext function.");
    }

    glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddress((GLubyte*)"glXChooseFBConfig");
    if (!glXChooseFBConfig) {
        throw InitException("Cannot initialize glXChooseFBConfig function.");
    }

    glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)glXGetProcAddress((GLubyte*)"glXGetVisualFromFBConfig");
    if (!glXGetVisualFromFBConfig) {
        throw InitException("Cannot initialize glXGetVisualFromFBConfig function.");
    }

    glXCreateWindow = (PFNGLXCREATEWINDOWPROC)glXGetProcAddress((GLubyte*)"glXCreateWindow");
    if (!glXCreateWindow) {
        throw InitException("Cannot initialize glXCreateWindow function.");
    }

    // TODO: glXMakeCurrent?
}

// Initializes the rendering context.
void OpenGLScreen::initRenderingContext() throw(InitException) {
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

    addWindowToIgnoreList(compOverlay);
    addWindowToIgnoreList(m_renderingWindow);

    // Creating a GLX handle for the above window.
    m_glxRenderingWindow = glXCreateWindow(display(), m_fbConfig, m_renderingWindow, NULL);
    if (!m_glxRenderingWindow) {
        throw InitException("Cannot create the rendering surface.");
    }
}

// Initializes GLEW.
void OpenGLScreen::initGlew() throw(InitException) {
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);

    // Initialize GLEW.
    GLenum glewErr = glewInit();
    if(glewErr != GLEW_OK) {
        std::stringstream ss;
        ss << "GLEW Error: " << (const char*)(glewGetErrorString(glewErr));
        throw InitException(ss.str().c_str());
    }

    // Check for an appropriate OpenGL version.
    if (!GLEW_VERSION_2_1) {
        throw InitException("OpenGL 2.1 not available.");
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

// Creates default texture rendering buffers.
void OpenGLScreen::createDefaultBuffers() {
    glGenBuffers(1, &m_defaultElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_defaultElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(DEFAULT_ELEMENT_ARRAY),
                 (const GLvoid*)(DEFAULT_ELEMENT_ARRAY), GL_STATIC_DRAW);

    glGenBuffers(1, &m_defaultPrimPosBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_defaultPrimPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DEFAULT_PRIM_POS_ARRAY),
                 (const GLvoid*)(DEFAULT_PRIM_POS_ARRAY), GL_STATIC_DRAW);

    glGenBuffers(1, &m_defaultTexPosBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_defaultTexPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DEFAULT_TEX_POS_ARRAY),
                 (const GLvoid*)(DEFAULT_TEX_POS_ARRAY), GL_STATIC_DRAW);
}

// Creates the background texture.
void OpenGLScreen::createBackgroundTexture() throw(InitException) {
    Atom bgPixmapAtom = XInternAtom(display(), "_XROOTPMAP_ID", False);
    Pixmap bgPixmap = rootWindow().singlePropertyValue<Pixmap>(bgPixmapAtom, 0);

    glGenTextures(1, &m_backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (bgPixmap) {
        XImage *image = XGetImage(display(), bgPixmap, 0, 0, rootWindow().width(), rootWindow().height(), AllPlanes, ZPixmap);
        if (!image) {
            throw InitException("Cannot create background texture.");
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rootWindow().width(), rootWindow().height(),
                     0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));
        XDestroyImage(image);
    }
}


//--- CONVENIENCE OPENGL WRAPPERS ----------------------------------------------

// Creates a shader.
GLuint OpenGLScreen::createShader(GLenum shaderType, GLint sourceLength, const GLchar *source) throw(InitException) {
    FbTk::FbString shaderName;
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
    OpenGLWindow *newWindow = new OpenGLWindow(window, m_fbConfig);
    return newWindow;
}


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void OpenGLScreen::renderScreen() {
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);
    glUseProgram(m_shaderProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    renderBackground();

    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        if ((*it)->isMapped()) {
            renderWindow(*(dynamic_cast<OpenGLWindow*>(*it)));
        }
        it++;
    }

    glXSwapBuffers(display(), m_glxRenderingWindow);
}

// A function to render the desktop background.
void OpenGLScreen::renderBackground() {
    renderTexture(m_defaultPrimPosBuffer, m_defaultTexPosBuffer, m_defaultElementBuffer, m_backgroundTexture);
}

// A function to render a particular window onto the screen.
void OpenGLScreen::renderWindow(OpenGLWindow &window) {
    if (window.isDamaged()) {
        window.updateContents();
    }

    renderTexture(window.windowPosBuffer(), m_defaultTexPosBuffer, m_defaultElementBuffer, window.contentTexture());
}

// A function to render some texture onto the screen.
void OpenGLScreen::renderTexture(GLuint primPosBuffer, GLuint texturePosBuffer, GLuint elementBuffer, GLuint texture) {
    // Load primitive position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, primPosBuffer);

    GLuint windowPosAttrib = glGetAttribLocation(m_shaderProgram, "fb_InitPointPos");
    glVertexAttribPointer(windowPosAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(windowPosAttrib);

    // Load texture position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, texturePosBuffer);

    GLuint texPosAttrib = glGetAttribLocation(m_shaderProgram, "fb_InitTexCoord");
    glVertexAttribPointer(texPosAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(texPosAttrib);

    // Load element array.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

    // Set up the texture uniforms.
    GLuint texturePos = glGetUniformLocation(m_shaderProgram, "fb_Texture");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(texturePos, 0);

    // Rendering.
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0);

    // Cleanup.
    glDisableVertexAttribArray(windowPosAttrib);
    glDisableVertexAttribArray(texPosAttrib);
}
