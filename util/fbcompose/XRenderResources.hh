/** XRenderResources.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERRESOURCES_HH
#define FBCOMPOSITOR_XRENDERRESOURCES_HH


#include "config.h"

#include "FbTk/RefCount.hh"

#include <X11/extensions/Xrender.h>
#include <X11/Xlib.h>


namespace FbCompositor {

    class XRenderScreen;


    //--- XRENDER PICTURE WRAPPER ----------------------------------------------

    /**
     * XRender picture wrapper.
     */
    class XRenderPicture {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderPicture(const XRenderScreen &screen, XRenderPictFormat *pictFormat, const char *pictFilter);

        /** Destructor. */
        ~XRenderPicture();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the handle of the picture held. */
        Picture handle() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Set a new PictFormat. */
        void setPictFormat(XRenderPictFormat *pictFormat);

        /** Associate the picture with the given pixmap. */
        void setPixmap(Pixmap pixmap, XRenderPictureAttributes pa = XRenderPictureAttributes(), long paMask = 0);


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        XRenderPicture(const XRenderPicture &other);

        /** Assignment operator. */
        XRenderPicture &operator=(const XRenderPicture &other);


        //--- INTERNALS --------------------------------------------------------

        /** The picture in question. */
        Picture m_picture;

        /** Picture filter to use. */
        const char *m_pictFilter;

        /** Picture format to use. */
        XRenderPictFormat *m_pictFormat;


        /** Current connection to the X server. */
        Display *m_display;

        /** The screen that manages the current picture. */
        const XRenderScreen &m_screen;
    };

    // Returns the picture held.
    inline Picture XRenderPicture::handle() const {
        return m_picture;
    }

    // Set a new PictFormat.
    inline void XRenderPicture::setPictFormat(XRenderPictFormat *pictFormat) {
        if (pictFormat) {
            m_pictFormat = pictFormat;
        }
    }


    //--- TYPEDEFS -------------------------------------------------------------

    /** XRender picture wrapper smart pointer. */
    typedef FbTk::RefCount<XRenderPicture> XRenderPicturePtr;
}

#endif  // FBCOMPOSITOR_XRENDERRESOURCES_HH
