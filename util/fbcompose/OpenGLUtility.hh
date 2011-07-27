/** OpenGLUtility.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLUTILITY_HH
#define FBCOMPOSITOR_OPENGLUTILITY_HH


#include <GL/glew.h>
#include <GL/gl.h>

#include <X11/Xlib.h>


namespace FbCompositor {

    //--- FUNCTIONS ------------------------------------------------------------

    /** Converts screen coordinates to OpenGL coordinates. */
    void toOpenGLCoords(int screenWidth, int screenHeight, int x, int y, int width, int height,
                        GLfloat *xLow_return, GLfloat *xHigh_return, GLfloat *yLow_return, GLfloat *yHigh_return);

    /** Converts screen coordinates to OpenGL coordinates. */
    void toOpenGLCoords(int screenWidth, int screenHeight, XRectangle rect,
                        GLfloat *xLow_return, GLfloat *xHigh_return, GLfloat *yLow_return, GLfloat *yHigh_return);

}

#endif  // FBCOMPOSITOR_OPENGLUTILITY_HH
