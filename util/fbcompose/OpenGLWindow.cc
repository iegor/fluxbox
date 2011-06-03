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

#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLWindow::OpenGLWindow(Window windowXID) :
    BaseCompWindow(windowXID) {

    m_rootWidth = dynamic_cast<Compositor*>(FbTk::App::instance())->getScreen(screenNumber()).rootWindow().width();
    m_rootHeight = dynamic_cast<Compositor*>(FbTk::App::instance())->getScreen(screenNumber()).rootWindow().height();

    // Create buffers.
    glGenBuffers(1, &m_vertexBuffer);
    glGenBuffers(1, &m_elementBuffer);

    // Fill buffers.
    updateArrays();

    for (int i = 0; i < 4; i++) {
        m_elementArray[i] = i;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_elementArray), (const GLvoid*)(m_elementArray), GL_STATIC_DRAW);
}

// Destructor.
OpenGLWindow::~OpenGLWindow() {
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_elementBuffer);
}


//--- WINDOW UPDATE FUNCTIONS ------------------------------------------

// Update window's vertex and element arrays.
void OpenGLWindow::updateArrays() {
    m_vertexArray[0] = m_vertexArray[4] = ((x() * 2.0) / m_rootWidth) - 1.0;
    m_vertexArray[2] = m_vertexArray[6] = (((x() + width()) * 2.0) / m_rootWidth) - 1.0;
    m_vertexArray[1] = m_vertexArray[3] = 1.0 - ((y() * 2.0) / m_rootHeight);
    m_vertexArray[5] = m_vertexArray[7] = 1.0 - (((y() + height()) * 2.0) / m_rootHeight);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexArray), (const GLvoid*)(m_vertexArray), GL_STATIC_DRAW);
}
