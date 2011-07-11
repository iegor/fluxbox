/** XRenderUtility.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERUTILITY_HH
#define FBCOMPOSITOR_XRENDERUTILITY_HH


#include "FbTk/RefCount.hh"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>


namespace FbCompositor {

    //--- XRENDER PICTURE HOLDER -----------------------------------------------

    /**
     * XRender picture holder.
     *
     * Use this class together with FbTk::RefCount or any other pointer wrapper
     * when you need to provide access to an XRender picture from multiple
     * locations in the code.
     */
    class XRenderPictureHolder {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderPictureHolder(Display *display, const XRenderPictFormat *pictFormat, const char *pictFilter) throw();

        /** Destructor. */
        ~XRenderPictureHolder() throw();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the picture held. */
        Picture picture() const throw();


        //--- MUTATORS ---------------------------------------------------------

        /** (Re)associate the picture with the given pixmap. */
        void setPixmap(Pixmap pixmap, XRenderPictureAttributes pa = XRenderPictureAttributes(), long paMask = 0) throw();


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        XRenderPictureHolder(const XRenderPictureHolder &other) throw();

        /** Assignment operator. */
        XRenderPictureHolder &operator=(const XRenderPictureHolder &other) throw();


        //--- INTERNALS --------------------------------------------------------

        /** The picture in question. */
        Picture m_picture;


        /** Pointer to the current X connection. */
        Display *m_display;

        /** Picture filter to use. */
        const char *m_pictFilter;

        /** Picture format to use. */
        const XRenderPictFormat *m_pictFormat;
    };

    // Returns the picture held.
    inline Picture XRenderPictureHolder::picture() const throw() {
        return m_picture;
    }


    /** XRender picture holder pointer. Use this if you want to provide an
     *  XRender picture to other places in the code. */
    typedef FbTk::RefCount<XRenderPictureHolder> XRenderPicturePtr;

}

#endif  // FBCOMPOSITOR_XRENDERUTILITY_HH
