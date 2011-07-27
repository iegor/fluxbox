/** XRenderPlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERPLUGIN_HH
#define FBCOMPOSITOR_XRENDERPLUGIN_HH

#include "BasePlugin.hh"
#include "Enumerations.hh"
#include "Exceptions.hh"
#include "XRenderResources.hh"

#include "FbTk/FbString.hh"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include <vector>


namespace FbCompositor {

    class BasePlugin;
    class BaseScreen;
    class InitException;
    class XRenderScreen;
    class XRenderWindow;
    class RenderingException;


    /**
     * A rendering job.
     */
    struct XRenderRenderingJob {
        int operation;                      ///< Compositing operation to use.
        XRenderPicturePtr sourcePicture;    ///< Picture to render.
        XRenderPicturePtr maskPicture;      ///< Mask picture to use.
        int sourceX;                        ///< X offset on the source picture.
        int sourceY;                        ///< Y offset on the source picture.
        int maskX;                          ///< X offset on the mask picture.
        int maskY;                          ///< Y offset on the mask picture.
        int destinationX;                   ///< X offset on the destination picture.
        int destinationY;                   ///< Y offset on the destination picture.
        int width;                          ///< Width of the picture to render.
        int height;                         ///< Height of the picture to render.
    };


    /**
     * Plugin for XRender renderer.
     */
    class XRenderPlugin : public BasePlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args);

        /** Destructor. */
        virtual ~XRenderPlugin();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the screen object, cast into the correct class. */
        const XRenderScreen &xrenderScreen() const;


        //--- RENDERING ACTIONS ------------------------------------------------

        /** Extra background rendering job. */
        virtual XRenderRenderingJob extraBackgroundRenderingJob();

        /** Reconfigure rectangle rendering options. */
        virtual void reconfigureRectRenderActions(XRectangle &rect_return, GC gc);


        /** Extra pre window rendering job. */
        virtual XRenderRenderingJob extraPreWindowRenderingJob(const XRenderWindow &window);

        /** Window rendering job initialization. */
        virtual void windowRenderingJobInit(const XRenderWindow &window, XRenderRenderingJob &job);

        /** Window rendering job cleanup. */
        virtual void windowRenderingJobCleanup(const XRenderWindow &window);

        /** Extra post window rendering job. */
        virtual XRenderRenderingJob extraPostWindowRenderingJob(const XRenderWindow &window);


        /** Called before the extra rendering jobs are executed. */
        virtual void preExtraRenderingActions();

        /** \returns the number of extra rendering jobs the plugin will do. */
        virtual int extraRenderingJobCount();

        /** Initialize the specified extra rendering job. */
        virtual XRenderRenderingJob extraRenderingJobInit(int jobId);

        /** Clean up after an extra rendering job. */
        virtual void extraRenderingJobCleanup(int jobId);

        /** Called after the extra rendering jobs are executed. */
        virtual void postExtraRenderingActions();
    };
}

#endif  // FBCOMPOSITOR_XRENDERPLUGIN_HH