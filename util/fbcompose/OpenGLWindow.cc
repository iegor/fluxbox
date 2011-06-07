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


#include "OpenGLWindow.hh"

#include "Compositor.hh"

#include "FbTk/App.hh"

#include <X11/Xutil.h>

#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLWindow::OpenGLWindow(Window windowXID) throw() :
    BaseCompWindow(windowXID) {

    m_rootWidth = dynamic_cast<Compositor*>(FbTk::App::instance())->getScreen(screenNumber()).rootWindow().width();
    m_rootHeight = dynamic_cast<Compositor*>(FbTk::App::instance())->getScreen(screenNumber()).rootWindow().height();

    // Create OpenGL elements.
    glGenTextures(1, &m_contentTexture);
    glGenBuffers(1, &m_windowPosBuffer);

    // Fill window position array.
    updateArrays();

    // Initialize the content texture.
    glBindTexture(GL_TEXTURE_2D, m_contentTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// Destructor.
OpenGLWindow::~OpenGLWindow() throw() {
    glDeleteTextures(1, &m_contentTexture);
    glDeleteBuffers(1, &m_windowPosBuffer);
}


//--- WINDOW UPDATE FUNCTIONS ------------------------------------------

// Update the appropriate window's arrays.
void OpenGLWindow::updateArrays() throw() {
    m_windowPosArray[0] = m_windowPosArray[4] = ((x() * 2.0) / m_rootWidth) - 1.0;
    m_windowPosArray[2] = m_windowPosArray[6] = (((x() + realWidth()) * 2.0) / m_rootWidth) - 1.0;
    m_windowPosArray[1] = m_windowPosArray[3] = 1.0 - ((y() * 2.0) / m_rootHeight);
    m_windowPosArray[5] = m_windowPosArray[7] = 1.0 - (((y() + realHeight()) * 2.0) / m_rootHeight);

    glBindBuffer(GL_ARRAY_BUFFER, m_windowPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_windowPosArray), (const GLvoid*)(m_windowPosArray), GL_STATIC_DRAW);
}

// Updates the window's contents.
void OpenGLWindow::updateContents() throw(RuntimeException) {
    BaseCompWindow::updateContents();

    if (contents()) {
        glBindTexture(GL_TEXTURE_2D, contentTexture());

        XImage *image = XGetImage(display(), contents(), 0, 0, realWidth(), realHeight(), AllPlanes, ZPixmap);
        if (!image) {
            // throw RuntimeException("Cannot create window's XImage.");
            return;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, realWidth(), realHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));
        XDestroyImage(image);
    }
}
