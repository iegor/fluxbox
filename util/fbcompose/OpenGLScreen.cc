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

#include "CompositorConfig.hh"
#include "Logging.hh"
#include "OpenGLPlugin.hh"
#include "OpenGLShaders.hh"
#include "OpenGLUtility.hh"
#include "ResourceWrappers.hh"

#include "FbTk/FbString.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>

#include <list>
#include <sstream>

using namespace FbCompositor;


//--- MACROS -------------------------------------------------------------------

// Macro for plugin iteration.
#define forEachPlugin(i, plugin)                                                       \
    (plugin) = ((pluginManager().plugins().size() > 0)                                 \
                   ? (dynamic_cast<OpenGLPlugin*>(pluginManager().plugins()[0]))       \
                   : NULL);                                                            \
    for(size_t (i) = 0;                                                                \
        ((i) < pluginManager().plugins().size());                                      \
        (i)++,                                                                         \
        (plugin) = (((i) < pluginManager().plugins().size())                           \
                       ? (dynamic_cast<OpenGLPlugin*>(pluginManager().plugins()[(i)])) \
                       : NULL))


//--- CONSTANTS ----------------------------------------------------------------

namespace {

    /** Size of the info log buffer. */
    static const int INFO_LOG_BUFFER_SIZE = 256;


    /** The preferred framebuffer configuration. */
    static const int PREFERRED_FBCONFIG_ATTRIBUTES[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
        GLX_DOUBLEBUFFER, GL_TRUE,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        None
    };

    /** The fallback framebuffer configuration. */
    static const int FALLBACK_FBCONFIG_ATTRIBUTES[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
        GLX_DOUBLEBUFFER, GL_FALSE,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        None
    };


    /** Default element array for texture rendering. */
    static const GLushort DEFAULT_ELEMENT_ARRAY[] = {
        0, 1, 2, 3
    };

    /** Default primitive position array for texture rendering. */
    static const GLfloat DEFAULT_PRIM_POS_ARRAY[] = {
        -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0
    };

    /** Default texture position array for texture rendering. */
    static const GLfloat DEFAULT_TEX_POS_ARRAY[] = {
        0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0
    };


    /** Element array for drawing the reconfigure rectangle. */
    static const GLushort RECONFIGURE_RECT_ELEMENT_ARRAY[] = {
        0, 1, 2, 3, 0
    };

}


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLScreen::OpenGLScreen(int screenNumber, const CompositorConfig &config) :
    BaseScreen(screenNumber, Plugin_OpenGL, config) {

    m_backgroundChanged = true;
    m_rootWindowChanged = false;

    earlyInitGLXPointers();
    initRenderingContext();
    initRenderingSurface();
    initGlew();
    finishRenderingInit();

    initShaders();
    createDefaultElements();
    createBackgroundTexture();
    createReconfigureRectElements();
    initPlugins();
}

// Destructor.
OpenGLScreen::~OpenGLScreen() {
    XUnmapWindow(display(), m_renderingWindow);

    glDetachShader(m_shaderProgram, m_vertexShader);
    glDetachShader(m_shaderProgram, m_fragmentShader);
    glDeleteProgram(m_shaderProgram);
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);

    glDeleteTextures(1, &m_blackTexture);
    glDeleteBuffers(1, &m_defaultElementBuffer);
    glDeleteBuffers(1, &m_defaultPrimPosBuffer);
    glDeleteBuffers(1, &m_defaultTexCoordBuffer);
    glDeleteTextures(1, &m_whiteTexture);

    glDeleteTextures(1, &m_backgroundTexture);

    glDeleteBuffers(1, &m_reconfigureRectElementBuffer);
    glDeleteBuffers(1, &m_reconfigureRectLinePosBuffer);

    glXDestroyWindow(display(), m_glxRenderingWindow);
    glXDestroyContext(display(), m_glxContext);
    XDestroyWindow(display(), m_renderingWindow);
}


//--- SCREEN MANIPULATION ----------------------------------------------

// Notifies the screen of the background change.
void OpenGLScreen::setRootPixmapChanged() {
    BaseScreen::setRootPixmapChanged();
    m_backgroundChanged = true;
}

// Notifies the screen of a root window change.
void OpenGLScreen::setRootWindowSizeChanged() {
    BaseScreen::setRootWindowSizeChanged();
    m_rootWindowChanged = true;
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Early initialization of GLX functions.
// We need GLX functions to create an OpenGL context and initialize GLEW. But
// using GLXEW zeroes the pointers, since glewInit() initializes them. And we
// have to use GLXEW to have easy access to GLX's extensions. So, this function
// performs minimal function initialization - just enough to create a context.
void OpenGLScreen::earlyInitGLXPointers() {
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
}

// Initializes the rendering context.
void OpenGLScreen::initRenderingContext() {
    int nConfigs;

    GLXFBConfig *fbConfigs = glXChooseFBConfig(display(), screenNumber(), PREFERRED_FBCONFIG_ATTRIBUTES, &nConfigs);
    m_haveDoubleBuffering = true;

    if (!fbConfigs) {
        fbConfigs = glXChooseFBConfig(display(), screenNumber(), FALLBACK_FBCONFIG_ATTRIBUTES, &nConfigs);
        m_haveDoubleBuffering = false;

        fbLog_warn << "Could not get a double-buffered framebuffer config, trying single buffer. Expect tearing." << std::endl;

        if (!fbConfigs) {
            throw InitException("Screen does not support the required framebuffer configuration.");
        }
    }

    m_fbConfig = fbConfigs[0];
    XFree(fbConfigs);

    // Creating the GLX rendering context.
    m_glxContext = glXCreateNewContext(display(), m_fbConfig, GLX_RGBA_TYPE, NULL, True);
    if (!m_glxContext) {
        throw InitException("Cannot create the OpenGL rendering context.");
    }
}

// Initializes the rendering surface.
void OpenGLScreen::initRenderingSurface() {
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

    XFree(visualInfo);
}

// Initializes GLEW.
void OpenGLScreen::initGlew() {
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);

    // Initialize GLEW.
    GLenum glewErr = glewInit();
    if(glewErr != GLEW_OK) {
        std::stringstream ss;
        ss << "GLEW Error: " << (const char*)(glewGetErrorString(glewErr));
        throw InitException(ss.str());
    }

    // Check for an appropriate OpenGL version.
    if (!GLEW_VERSION_2_1) {
        throw InitException("OpenGL 2.1 not available.");
    }
}

// Finishes the initialization of the rendering context and surface.
void OpenGLScreen::finishRenderingInit() {
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef GLXEW_EXT_texture_from_pixmap
    fbLog_warn << "GLX_EXT_texture_from_pixmap extension not found, expect a performance hit." << std::endl;
#endif
}

// Initializes shaders.
void OpenGLScreen::initShaders() {
    std::stringstream ss;
    OpenGLPlugin *plugin;

    // Assemble vertex shader.
    ss.str("");
    ss << OpenGLShaders::vertexShaderHead();
    forEachPlugin(i, plugin) {
        ss << plugin->vertexShader() << "\n";
    }
    ss << OpenGLShaders::vertexShaderMiddle();
    forEachPlugin(i, plugin) {
        ss << plugin->pluginName() << "();\n";
    }
    ss << OpenGLShaders::vertexShaderTail();
    m_vertexShader = createShader(GL_VERTEX_SHADER, ss.str().length(), ss.str().c_str());

    // Assemble fragment shader.
    ss.str("");
    ss << OpenGLShaders::fragmentShaderHead();
    forEachPlugin(i, plugin) {
        ss << plugin->fragmentShader() << "\n";
    }
    ss << OpenGLShaders::fragmentShaderMiddle();
    forEachPlugin(i, plugin) {
        ss << plugin->pluginName() << "();\n";
    }
    ss << OpenGLShaders::fragmentShaderTail();
    m_fragmentShader = createShader(GL_FRAGMENT_SHADER, ss.str().length(), ss.str().c_str());

    // Create shader program.
    m_shaderProgram = createShaderProgram(m_vertexShader, 0, m_fragmentShader);

    // Initialize attribute locations.
    m_mainTexCoordAttrib = glGetAttribLocation(m_shaderProgram, "fb_InitMainTexCoord");
    m_primPosAttrib = glGetAttribLocation(m_shaderProgram, "fb_InitPrimPos");
    m_shapeTexCoordAttrib = glGetAttribLocation(m_shaderProgram, "fb_InitShapeTexCoord");

    // Initialize uniform locations.
    m_alphaUniform = glGetUniformLocation(m_shaderProgram, "fb_Alpha");
    m_mainTexUniform = glGetUniformLocation(m_shaderProgram, "fb_MainTexture");
    m_shapeTexUniform = glGetUniformLocation(m_shaderProgram, "fb_ShapeTexture");
}


// Creates default texture rendering buffers.
void OpenGLScreen::createDefaultElements() {
    int textureData[1][1];

    // Default black texture.
    glGenTextures(1, &m_blackTexture);
    glBindTexture(GL_TEXTURE_2D, m_blackTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    textureData[0][0] = 0x00000000;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)(textureData));

    // Default element buffer.
    glGenBuffers(1, &m_defaultElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_defaultElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(DEFAULT_ELEMENT_ARRAY),
                 (const GLvoid*)(DEFAULT_ELEMENT_ARRAY), GL_STATIC_DRAW);

    // Default primitive position buffer.
    glGenBuffers(1, &m_defaultPrimPosBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_defaultPrimPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DEFAULT_PRIM_POS_ARRAY),
                 (const GLvoid*)(DEFAULT_PRIM_POS_ARRAY), GL_STATIC_DRAW);

    // Default texture position buffer.
    glGenBuffers(1, &m_defaultTexCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_defaultTexCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DEFAULT_TEX_POS_ARRAY),
                 (const GLvoid*)(DEFAULT_TEX_POS_ARRAY), GL_STATIC_DRAW);

    // Default white texture.
    glGenTextures(1, &m_whiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    textureData[0][0] = 0xffffffff;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)(textureData));
}

// Creates the background texture.
void OpenGLScreen::createBackgroundTexture() {
    glGenTextures(1, &m_backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// Creates all elements, needed to draw the reconfigure rectangle.
void OpenGLScreen::createReconfigureRectElements() {
    // Buffers.
    glGenBuffers(1, &m_reconfigureRectLinePosBuffer);

    glGenBuffers(1, &m_reconfigureRectElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_reconfigureRectElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(RECONFIGURE_RECT_ELEMENT_ARRAY),
                 (const GLvoid*)(RECONFIGURE_RECT_ELEMENT_ARRAY), GL_STATIC_DRAW);
}

// Finish plugin initialization.
void OpenGLScreen::initPlugins() {
    OpenGLPlugin *plugin = NULL;
    forEachPlugin(i, plugin) {
        plugin->initOpenGL(m_shaderProgram);
    }
}


//--- OTHER FUNCTIONS --------------------------------------------------

// Renews the background texture.
void OpenGLScreen::updateBackgroundTexture() {
    Pixmap bgPixmap = rootWindowPixmap();

    if (bgPixmap) {
        XImage *image = XGetImage(display(), bgPixmap, 0, 0, rootWindow().width(), rootWindow().height(), AllPlanes, ZPixmap);
        if (!image) {
            fbLog_warn << "Cannot create background texture (reason: cannot create XImage)." << std::endl;
            return;
        }

        glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rootWindow().width(), rootWindow().height(),
                     0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));

        XDestroyImage(image);
        m_backgroundChanged = false;
    } else {
        fbLog_warn << "Cannot create background texture (reason: cannot find bg pixmap atom)." << std::endl;
    }
}

// React to the geometry change of the root window.
void OpenGLScreen::updateOnRootWindowResize() {
    XResizeWindow(display(), m_renderingWindow, rootWindow().width(), rootWindow().height());

    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        (dynamic_cast<OpenGLWindow*>(*it))->updateWindowPosArray();
        ++it;
    }

    m_rootWindowChanged = false;
}


//--- CONVENIENCE OPENGL WRAPPERS ----------------------------------------------

// Creates a shader.
GLuint OpenGLScreen::createShader(GLenum shaderType, GLint sourceLength, const GLchar *source) {
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
        ss << "Error in compilation of the " << shaderName << " shader: "
           << std::endl << (const char*)(infoLog);
        throw InitException(ss.str());
    }

    return shader;
}

// Creates a shader program.
GLuint OpenGLScreen::createShaderProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader) {
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
        ss << "Error in linking of the shader program: " << std::endl << (const char*)(infoLog);
        throw InitException(ss.str());
    }

    return program;
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Creates a window object from its XID.
BaseCompWindow *OpenGLScreen::createWindowObject(Window window) {
    OpenGLWindow *newWindow = new OpenGLWindow(*this, window, m_fbConfig);
    return newWindow;
}


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void OpenGLScreen::renderScreen() {
    // React to root window changes.
    if (m_rootWindowChanged) {
        updateOnRootWindowResize();
    }

    // Prepare for rendering.
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);
    glUseProgram(m_shaderProgram);

    // Render desktop background.
    renderBackground();

    // Render the windows.
    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        if ((*it)->isMapped()) {
            renderWindow(*(dynamic_cast<OpenGLWindow*>(*it)));
        }
        ++it;
    }

    // Render the reconfigure rectangle.
    if ((reconfigureRectangle().width != 0) && (reconfigureRectangle().height != 0)) {
        renderReconfigureRect();
    }

    // Execute any extra jobs plugins may request.
    renderExtraJobs();

    // Finish.
    glFlush();
    if (m_haveDoubleBuffering) {
        glXSwapBuffers(display(), m_glxRenderingWindow);
    }
}


// A function to render the desktop background.
void OpenGLScreen::renderBackground() {
    OpenGLPlugin *plugin = NULL;

    // Update desktop background texture.
    if (m_backgroundChanged) {
        updateBackgroundTexture();
    }

    // Render it.
    forEachPlugin(i, plugin) {
        plugin->preBackgroundRenderActions();
    }
    render(GL_TRIANGLE_STRIP, m_defaultPrimPosBuffer, m_defaultTexCoordBuffer, m_backgroundTexture,
           m_defaultTexCoordBuffer, m_whiteTexture, m_defaultElementBuffer, 4, 1.0);
    forEachPlugin(i, plugin) {
        plugin->postBackgroundRenderActions();
    }
}

// Perform extra rendering jobs from plugins.
void OpenGLScreen::renderExtraJobs() {
    OpenGLPlugin *plugin = NULL;

    GLfloat alpha;
    GLuint mainTexCoordBuffer;
    GLuint mainTexture;
    GLuint primPosBuffer;
    GLuint shapeTexCoordBuffer;
    GLuint shapeTexture;

    forEachPlugin(i, plugin) {
        plugin->preExtraRenderingActions();

        for (int j = 0; j < plugin->extraRenderingJobCount(); j++) {
            plugin->extraRenderingJobInit(j, primPosBuffer, mainTexCoordBuffer, mainTexture,
                                          shapeTexCoordBuffer, shapeTexture, alpha);
            render(GL_TRIANGLE_STRIP, primPosBuffer, mainTexCoordBuffer, mainTexture,
                   shapeTexCoordBuffer, shapeTexture, m_defaultElementBuffer, 4, alpha);
            plugin->extraRenderingJobCleanup(j);
        }

        plugin->postExtraRenderingActions();
    }
}

// Render the reconfigure rectangle.
void OpenGLScreen::renderReconfigureRect() {
    OpenGLPlugin *plugin = NULL;

    // Convert reconfigure rectangle to OpenGL coordinates.
    GLfloat xLow, xHigh, yLow, yHigh;
    toOpenGLCoordinates(rootWindow().width(), rootWindow().height(),
            reconfigureRectangle().x, reconfigureRectangle().y, reconfigureRectangle().width, reconfigureRectangle().height,
            &xLow, &xHigh, &yLow, &yHigh);
    GLfloat linePosArray[] = { xLow, yLow, xHigh, yLow, xHigh, yHigh, xLow, yHigh };

    glBindBuffer(GL_ARRAY_BUFFER, m_reconfigureRectLinePosBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(linePosArray), (const GLvoid*)(linePosArray), GL_STATIC_DRAW);

    // Render it.
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_XOR);

    forEachPlugin(i, plugin) {
        plugin->preReconfigureRectRenderActions(reconfigureRectangle());
    }
    render(GL_LINE_STRIP, m_reconfigureRectLinePosBuffer, m_defaultTexCoordBuffer, m_whiteTexture,
           m_defaultTexCoordBuffer, m_whiteTexture, m_reconfigureRectElementBuffer, 5, 1.0);
    forEachPlugin(i, plugin) {
        plugin->postReconfigureRectRenderActions(reconfigureRectangle());
    }

    glDisable(GL_COLOR_LOGIC_OP);
}

// A function to render a particular window onto the screen.
void OpenGLScreen::renderWindow(OpenGLWindow &window) {
    OpenGLPlugin *plugin = NULL;

    // Update window's contents.
    if (window.isDamaged()) {
        window.updateContents();
    }

    // Render it.
    forEachPlugin(i, plugin) {
        plugin->preWindowRenderActions(window);
    }
    render(GL_TRIANGLE_STRIP, window.windowPosBuffer()->unwrap(),
           m_defaultTexCoordBuffer, window.contentTexture()->unwrap(),
           m_defaultTexCoordBuffer, window.shapeTexture()->unwrap(),
           m_defaultElementBuffer, 4, window.alpha() / 255.0);
    forEachPlugin(i, plugin) {
        plugin->postWindowRenderActions(window);
    }
}


// A function to render something onto the screen.
void OpenGLScreen::render(GLenum renderingMode, GLuint primPosBuffer, GLuint mainTexCoordBuffer, GLuint mainTexture,
                          GLuint shapeTexCoordBuffer, GLuint shapeTexture, GLuint elementBuffer, GLuint elementCount,
                          GLfloat alpha) {
    // Load primitive position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, primPosBuffer);
    glVertexAttribPointer(m_primPosAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_primPosAttrib);

    // Load main texture position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, mainTexCoordBuffer);
    glVertexAttribPointer(m_mainTexCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_mainTexCoordAttrib);

    // Load shape texture position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, shapeTexCoordBuffer);
    glVertexAttribPointer(m_shapeTexCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_shapeTexCoordAttrib);

    // Set up textures.
    glUniform1i(m_mainTexUniform, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mainTexture);

    glUniform1i(m_shapeTexUniform, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shapeTexture);

    // Set up other uniforms.
    glUniform1f(m_alphaUniform, alpha);

    // Load element array.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

    // Final setup.
    if (m_haveDoubleBuffering) {
        glDrawBuffer(GL_BACK);
    }
    glViewport(0, 0, rootWindow().width(), rootWindow().height());

    // Render!
    glDrawElements(renderingMode, elementCount, GL_UNSIGNED_SHORT, (void*)0);

    // Cleanup.
    glDisableVertexAttribArray(m_mainTexCoordAttrib);
    glDisableVertexAttribArray(m_primPosAttrib);
    glDisableVertexAttribArray(m_shapeTexCoordAttrib);
}
