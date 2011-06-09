/** OpenGLWindow.cc file for the fluxbox compositor. */

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


#include "Compositor.hh"
#include "Logging.hh"
#include "OpenGLWindow.hh"

#include "FbTk/App.hh"

#include <X11/Xutil.h>

#include <algorithm>
#include <iostream>

using namespace FbCompositor;

//--- CONSTANTS ----------------------------------------------------------------

#ifdef GLXEW_EXT_texture_from_pixmap

// Attributes of the contents' GLX pixmap.
const int OpenGLWindow::TEX_PIXMAP_ATTRIBUTES[] = {
    GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
    GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGB_EXT,
    None
};

#endif  // GLXEW_EXT_texture_from_pixmap


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLWindow::OpenGLWindow(Window windowXID, GLXFBConfig fbConfig) throw() :
    BaseCompWindow(windowXID) {

    m_fbConfig = fbConfig;
    m_rootWidth = dynamic_cast<Compositor*>(FbTk::App::instance())->getScreen(screenNumber()).rootWindow().width();
    m_rootHeight = dynamic_cast<Compositor*>(FbTk::App::instance())->getScreen(screenNumber()).rootWindow().height();

    // Create OpenGL elements.
    glGenTextures(1, &m_contentTexture);
    glGenBuffers(1, &m_windowPosBuffer);

    // Fill window position array.
    updateWindowPosArray();

    // Initialize the content texture.
    glBindTexture(GL_TEXTURE_2D, m_contentTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef GLXEW_EXT_texture_from_pixmap
    m_glxContents = 0;
#endif
}

// Destructor.
OpenGLWindow::~OpenGLWindow() throw() {
    glDeleteTextures(1, &m_contentTexture);
    glDeleteBuffers(1, &m_windowPosBuffer);
}


//--- WINDOW UPDATE FUNCTIONS ------------------------------------------

// Reconfigures the window.
void OpenGLWindow::reconfigure(const XConfigureEvent &event) throw() {
    BaseCompWindow::reconfigure(event);
    updateWindowPosArray();
}

// Updates the window's contents.
void OpenGLWindow::updateContents() throw(RuntimeException) {
    updateContentPixmap();

    if (contentPixmap()) {

#ifdef GLXEW_EXT_texture_from_pixmap
        if (m_glxContents) {
            glXDestroyPixmap(display(), m_glxContents);
            m_glxContents = 0;
        }
        m_glxContents = glXCreatePixmap(display(), m_fbConfig, contentPixmap(), TEX_PIXMAP_ATTRIBUTES);

        glBindTexture(GL_TEXTURE_2D, contentTexture());
        glXBindTexImageEXT(display(), m_glxContents, GLX_FRONT_LEFT_EXT, NULL);

#else
        XImage *image = XGetImage(display(), contentPixmap(), 0, 0, realWidth(), realHeight(), AllPlanes, ZPixmap);
        if (!image) {
            fbLog_warn << "Cannot create XImage for window " << window() << ". It's probably nothing - skipping." << std::endl;
            return;
        }

        glBindTexture(GL_TEXTURE_2D, m_contentTexture);

        if (isResized()) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, realWidth(), realHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));
        } else {
            XImage *subImage;
            for (size_t i = 0; i < damagedArea().size(); i++) {
                int damageWidth = std::min(damagedArea()[i].width + 1, realWidth());
                int damageHeight = std::min(damagedArea()[i].height + 1, realHeight());

                subImage = XSubImage(image, damagedArea()[i].x, damagedArea()[i].y, damageWidth, damageHeight);
                glTexSubImage2D(GL_TEXTURE_2D, 0, damagedArea()[i].x, damagedArea()[i].y, damageWidth,
                                damageHeight, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(subImage->data[0])));
                XDestroyImage(subImage);
            }
        }

        XDestroyImage(image);

#endif  // GLXEW_EXT_texture_from_pixmap
    }

    clearDamage();
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Updates the window position vertex array.
void OpenGLWindow::updateWindowPosArray() throw() {
    m_windowPosArray[0] = m_windowPosArray[4] = ((x() * 2.0) / m_rootWidth) - 1.0;
    m_windowPosArray[2] = m_windowPosArray[6] = (((x() + realWidth()) * 2.0) / m_rootWidth) - 1.0;
    m_windowPosArray[1] = m_windowPosArray[3] = 1.0 - ((y() * 2.0) / m_rootHeight);
    m_windowPosArray[5] = m_windowPosArray[7] = 1.0 - (((y() + realHeight()) * 2.0) / m_rootHeight);

    glBindBuffer(GL_ARRAY_BUFFER, m_windowPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_windowPosArray), (const GLvoid*)(m_windowPosArray), GL_STATIC_DRAW);
}
