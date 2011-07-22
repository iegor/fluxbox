/** XRenderResources.cc file for the fluxbox compositor. */

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


#include "XRenderResources.hh"

#include "XRenderScreen.hh"

using namespace FbCompositor;


//--- XRENDER PICTURE WRAPPER --------------------------------------------------

//------- CONSTRUCTORS AND DESTRUCTORS -----------------------------------------

// Constructor.
XRenderPicture::XRenderPicture(const XRenderScreen &screen, XRenderPictFormat *pictFormat, const char *pictFilter) :
    m_picture(None),
    m_pictFilter(pictFilter),
    m_pictFormat(pictFormat),
    m_screen(screen) {

    m_display = (Display*)(screen.display());
}

// Destructor.
XRenderPicture::~XRenderPicture() {
    if (m_picture) {
        XRenderFreePicture(m_display, m_picture);
    }
}


//------- MUTATORS -------------------------------------------------------------

// Associate the picture with the given pixmap.
void XRenderPicture::setPixmap(Pixmap pixmap, XRenderPictureAttributes pa, long paMask) {
    if (m_picture) {
        XRenderFreePicture(m_display, m_picture);
        m_picture = None;
    }

    m_picture = XRenderCreatePicture(m_display, pixmap, m_pictFormat, paMask, &pa);
    XRenderSetPictureFilter(m_display, m_picture, m_pictFilter, NULL, 0);
}
