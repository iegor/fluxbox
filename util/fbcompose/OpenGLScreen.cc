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
#include "Exceptions.hh"
#include "OpenGLResources.hh"
#include "OpenGLUtility.hh"
#include "Utility.hh"

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

/** The preferred framebuffer configuration. */
const int PREFERRED_FBCONFIG_ATTRIBUTES[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
    GLX_DOUBLEBUFFER, GL_TRUE,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
#ifdef GLXEW_EXT_texture_from_pixmap
    GLX_BIND_TO_TEXTURE_RGBA_EXT, GL_TRUE,
#endif  // GLXEW_EXT_texture_from_pixmap
    None
};

/** The fallback framebuffer configuration. */
const int FALLBACK_FBCONFIG_ATTRIBUTES[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
    GLX_DOUBLEBUFFER, GL_FALSE,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
#ifdef GLXEW_EXT_texture_from_pixmap
    GLX_BIND_TO_TEXTURE_RGBA_EXT, GL_TRUE,
#endif  // GLXEW_EXT_texture_from_pixmap
    None
};


/** Default element array for texture rendering. */
const GLushort DEFAULT_ELEMENT_ARRAY[] = {
    0, 1, 2, 3
};

/** Default primitive position array for texture rendering. */
const GLfloat DEFAULT_PRIM_POS_ARRAY[] = {
    -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0
};

/** Default texture position array for texture rendering. */
const GLfloat DEFAULT_TEX_POS_ARRAY[] = {
    0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0
};


/** Element array for drawing the reconfigure rectangle. */
const GLushort RECONFIGURE_RECT_ELEMENT_ARRAY[] = {
    0, 1, 2, 3, 0
};


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLScreen::OpenGLScreen(int screenNumber, const CompositorConfig &config) :
    BaseScreen(screenNumber, Plugin_OpenGL, config) {

    m_bgChanged = true;
    m_rootWindowChanged = false;

    earlyInitGLXPointers();
    initRenderingContext();
    initRenderingSurface();
    initGlew();
    finishRenderingInit();

    findMaxTextureSize();
    createResources();
    initPlugins();
}

// Destructor.
OpenGLScreen::~OpenGLScreen() {
    XUnmapWindow(display(), m_renderingWindow);
    glXDestroyWindow(display(), m_glxRenderingWindow);
    glXDestroyContext(display(), m_glxContext);
    XDestroyWindow(display(), m_renderingWindow);
}


//--- SCREEN MANIPULATION ----------------------------------------------

// Notifies the screen of the background change.
void OpenGLScreen::setRootPixmapChanged() {
    BaseScreen::setRootPixmapChanged();
    m_bgChanged = true;
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

    ignoreWindow(compOverlay);
    ignoreWindow(m_renderingWindow);

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

    GLenum glewErr = glewInit();
    if(glewErr != GLEW_OK) {
        std::stringstream ss;
        ss << "GLEW Error: " << (const char*)(glewGetErrorString(glewErr));
        throw InitException(ss.str());
    }

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

#ifndef GL_ARB_texture_swizzle
#ifndef GL_EXT_texture_swizzle
    fbLog_warn << "Could not find GL_ARB_texture_swizzle or GL_EXT_texture_swizzle extensions. Expect glitches." << std::endl;
#endif
#endif
}

// Finds the maximum usable texture size.
void OpenGLScreen::findMaxTextureSize() {
    GLint texSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
    texSize = (GLint)(largestSmallerPowerOf2((int)(texSize)));

    while (texSize > 0) {
        GLint width;

        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);

        if (width == 0) {
            texSize >>= 1;
        } else {
            break;
        }
    }

    m_maxTextureSize = (int)(texSize);
    if ((m_maxTextureSize < (int)(rootWindow().width()))
            || (m_maxTextureSize < (int)(rootWindow().height()))) {
        fbLog_warn << "Maximum supported OpenGL texture size on this machine is less than one "
                   << "of the root window's dimensions. There may be a performance hit." << std::endl;
    }
}

// Creates OpenGL resources.
void OpenGLScreen::createResources() {
    Pixmap pixmap;

    // Default element buffer.
    m_defaultElementBuffer = new OpenGLBuffer(*this, GL_ELEMENT_ARRAY_BUFFER);
    m_defaultElementBuffer->bufferData(sizeof(DEFAULT_ELEMENT_ARRAY), (const GLvoid*)(DEFAULT_ELEMENT_ARRAY), GL_STATIC_DRAW);

    // Default primitive position buffer.
    m_defaultPrimPosBuffer = new OpenGLBuffer(*this, GL_ARRAY_BUFFER);
    m_defaultPrimPosBuffer->bufferData(sizeof(DEFAULT_PRIM_POS_ARRAY), (const GLvoid*)(DEFAULT_PRIM_POS_ARRAY), GL_STATIC_DRAW);

    // Default texture position buffer.
    m_defaultTexCoordBuffer = new OpenGLBuffer(*this, GL_ARRAY_BUFFER);
    m_defaultTexCoordBuffer->bufferData(sizeof(DEFAULT_TEX_POS_ARRAY), (const GLvoid*)(DEFAULT_TEX_POS_ARRAY), GL_STATIC_DRAW);


    // Reconfigure rectangle position buffer.
    m_recRectLinePosBuffer = new OpenGLBuffer(*this, GL_ARRAY_BUFFER);

    // Reconfigure rectangle element buffer.
    m_recRectElementBuffer = new OpenGLBuffer(*this, GL_ELEMENT_ARRAY_BUFFER);
    m_recRectElementBuffer->bufferData(sizeof(RECONFIGURE_RECT_ELEMENT_ARRAY),
                                       (const GLvoid*)(RECONFIGURE_RECT_ELEMENT_ARRAY), GL_STATIC_DRAW);


    // Background texture.
    m_bgTexture = new OpenGL2DTexturePartition(*this, true);

    // Background texture partition buffers.
    m_bgPosBuffers = partitionSpaceToBuffers(*this, 0, 0, rootWindow().width(), rootWindow().height());


    // Plain black texture.
    pixmap = createSolidPixmap(display(), rootWindow().window(), 1, 1, 0x00000000);
    m_blackTexture = new OpenGL2DTexture(*this, false);
    m_blackTexture->setPixmap(pixmap, false, 1, 1, true);
    XFreePixmap(display(), pixmap);

    // Plain white texture.
    pixmap = createSolidPixmap(display(), rootWindow().window(), 1, 1, 0xffffffff);
    m_whiteTexture = new OpenGL2DTexture(*this, false);
    m_whiteTexture->setPixmap(pixmap, false, 1, 1, true);
    XFreePixmap(display(), pixmap);


    // Shader program.
    m_shaderProgram = new OpenGLShaderProgram(pluginManager().plugins());
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
    int depth = rootWindow().depth();
    if (!wmSetRootWindowPixmap()) {
        depth = 32;
    }

    m_bgTexture->setPixmap(rootWindowPixmap(), false, rootWindow().width(), rootWindow().height(), depth);
    m_bgChanged = false;
}

// React to the geometry change of the root window.
void OpenGLScreen::updateOnRootWindowResize() {
    XResizeWindow(display(), m_renderingWindow, rootWindow().width(), rootWindow().height());

    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        (dynamic_cast<OpenGLWindow*>(*it))->updateWindowPos();
        ++it;
    }

    m_bgPosBuffers = partitionSpaceToBuffers(*this, 0, 0, rootWindow().width(), rootWindow().height());
    m_rootWindowChanged = false;
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Creates a window object from its XID.
BaseCompWindow *OpenGLScreen::createWindowObject(Window window) {
    OpenGLWindow *newWindow = new OpenGLWindow(*this, window);
    return newWindow;
}


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void OpenGLScreen::renderScreen() {
    // React to root window changes.
    if (m_rootWindowChanged) {
        updateOnRootWindowResize();
    }

    // Set up the rendering context.
    glXMakeCurrent(display(), m_glxRenderingWindow, m_glxContext);
    m_shaderProgram->use();

    // Render background.
    renderBackground();

    // Render windows.
    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        if (!(*it)->isIgnored() && (*it)->isMapped()) {
            renderWindow(*(dynamic_cast<OpenGLWindow*>(*it)));
        }
        ++it;
    }

    // Render reconfigure rectangle.
    if ((reconfigureRectangle().width != 0) && (reconfigureRectangle().height != 0)) {
        renderReconfigureRect();
    }

    // Execute any extra jobs from plugins.
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

    // Update background texture if needed.
    if (m_bgChanged) {
        updateBackgroundTexture();
    }

    // Render background.
    for (size_t i = 0; i < m_bgTexture->partitions().size(); i++) {
        forEachPlugin(j, plugin) {
            plugin->backgroundRenderInit(i);
        }
        render(GL_TRIANGLE_STRIP, m_bgPosBuffers[i],
               m_defaultTexCoordBuffer, m_bgTexture->partitions()[i].texture,
               m_defaultTexCoordBuffer, m_whiteTexture,
               m_defaultElementBuffer, 4, 1.0);
        forEachPlugin(j, plugin) {
            plugin->backgroundRenderCleanup(i);
        }
    }

    // Execute extra post background rendering jobs.
    forEachPlugin(i, plugin) {
        plugin->nullRenderInit();
    }
    forEachPlugin(i, plugin) {
        executeMultipleJobs(plugin, plugin->postBackgroundRenderActions());
    }
}

// Perform extra rendering jobs from plugins.
void OpenGLScreen::renderExtraJobs() {
    OpenGLPlugin *plugin = NULL;

    forEachPlugin(i, plugin) {
        plugin->nullRenderInit();
    }
    forEachPlugin(i, plugin) {
        executeMultipleJobs(plugin, plugin->extraRenderingActions());
        plugin->postExtraRenderingActions();
    }
}

// Render the reconfigure rectangle.
void OpenGLScreen::renderReconfigureRect() {
    OpenGLPlugin *plugin = NULL;

    // Convert reconfigure rectangle to OpenGL coordinates.
    m_recRectLinePosBuffer->bufferPosRectangle(rootWindow().width(), rootWindow().height(), reconfigureRectangle());

    // Render it.
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_XOR);

    forEachPlugin(i, plugin) {
        plugin->recRectRenderInit(reconfigureRectangle());
    }
    render(GL_LINE_STRIP, m_recRectLinePosBuffer, m_defaultTexCoordBuffer, m_whiteTexture,
           m_defaultTexCoordBuffer, m_whiteTexture, m_recRectElementBuffer, 5, 1.0);
    forEachPlugin(i, plugin) {
        plugin->recRectRenderCleanup(reconfigureRectangle());
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

    // Extra rendering jobs before a window is drawn.
    forEachPlugin(i, plugin) {
        plugin->nullRenderInit();
    }
    forEachPlugin(i, plugin) {
        executeMultipleJobs(plugin, plugin->preWindowRenderActions(window));
    }

    // Render it.
    for (int i = 0; i < window.partitionCount(); i++) {
        forEachPlugin(j, plugin) {
            plugin->windowRenderInit(window, i);
        }
        render(GL_TRIANGLE_STRIP, window.partitionPosBuffer(i),
               m_defaultTexCoordBuffer, window.contentTexturePartition(i),
               m_defaultTexCoordBuffer, window.shapeTexturePartition(i),
               m_defaultElementBuffer, 4, window.alpha() / 255.0);
        forEachPlugin(j, plugin) {
            plugin->windowRenderCleanup(window, i);
        }
    }

    // Extra rendering jobs after a window is drawn.
    forEachPlugin(i, plugin) {
        plugin->nullRenderInit();
    }
    forEachPlugin(i, plugin) {
        executeMultipleJobs(plugin, plugin->postWindowRenderActions(window));
    }
}


// Execute multiple rendering jobs.
void OpenGLScreen::executeMultipleJobs(OpenGLPlugin *plugin, const std::vector<OpenGLRenderingJob> &jobs) {
    for (size_t i = 0; i < jobs.size(); i++) {
        executeRenderingJob(jobs[i]);
    }
    plugin->nullRenderInit();
}

// Execute a given rendering job.
void OpenGLScreen::executeRenderingJob(const OpenGLRenderingJob &job) {
    if ((job.alpha >= 0.0) && (job.alpha <= 1.0)) {
        job.shaderInit->execute();
        render(GL_TRIANGLE_STRIP, job.primPosBuffer, job.mainTexCoordBuffer, job.mainTexture,
               job.shapeTexCoordBuffer, job.shapeTexture, m_defaultElementBuffer, 4, job.alpha);
        job.shaderDeinit->execute();
    }
}

// A function to render something onto the screen.
void OpenGLScreen::render(GLenum renderingMode, OpenGLBufferPtr primPosBuffer,
                          OpenGLBufferPtr mainTexCoordBuffer, OpenGL2DTexturePtr mainTexture,
                          OpenGLBufferPtr shapeTexCoordBuffer, OpenGL2DTexturePtr shapeTexture,
                          OpenGLBufferPtr elementBuffer, GLuint elementCount, GLfloat alpha) {
    // Load primitive position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, primPosBuffer->handle());
    glVertexAttribPointer(m_shaderProgram->primPosAttrib(), 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_shaderProgram->primPosAttrib());

    // Load main texture position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, mainTexCoordBuffer->handle());
    glVertexAttribPointer(m_shaderProgram->mainTexCoordAttrib(), 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_shaderProgram->mainTexCoordAttrib());

    // Load shape texture position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, shapeTexCoordBuffer->handle());
    glVertexAttribPointer(m_shaderProgram->shapeTexCoordAttrib(), 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_shaderProgram->shapeTexCoordAttrib());

    // Set up textures.
    glUniform1i(m_shaderProgram->mainTexUniform(), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mainTexture->handle());

    glUniform1i(m_shaderProgram->shapeTexUniform(), 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shapeTexture->handle());

    // Set up other uniforms.
    glUniform1f(m_shaderProgram->alphaUniform(), alpha);

    // Load element array.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer->handle());

    // Final setup.
    if (m_haveDoubleBuffering) {
        glDrawBuffer(GL_BACK);
    }
    glViewport(0, 0, rootWindow().width(), rootWindow().height());

    // Render!
    glDrawElements(renderingMode, elementCount, GL_UNSIGNED_SHORT, (void*)0);

    // Cleanup.
    glDisableVertexAttribArray(m_shaderProgram->mainTexCoordAttrib());
    glDisableVertexAttribArray(m_shaderProgram->primPosAttrib());
    glDisableVertexAttribArray(m_shaderProgram->shapeTexCoordAttrib());
}
