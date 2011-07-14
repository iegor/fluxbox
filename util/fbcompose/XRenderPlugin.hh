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
#include "Constants.hh"
#include "Exceptions.hh"

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
        virtual void extraBackgroundRenderingJob(int &op_return,
                                                 Picture &srcPic_return, int &srcX_return, int &srcY_return,
                                                 Picture &maskPic_return, int &maskX_return, int &maskY_return,
                                                 int &destX_return, int &destY_return,
                                                 int &width_return, int &height_return);

        /** Reconfigure rectangle rendering options. */
        virtual void reconfigureRectRenderActions(XRectangle &rect_return, GC gc);


        /** Extra pre window rendering job. */
        virtual void extraPreWindowRenderingJob(const XRenderWindow &window, int &op_return,
                                                Picture &srcPic_return, int &srcX_return, int &srcY_return,
                                                Picture &maskPic_return, int &maskX_return, int &maskY_return,
                                                int &destX_return, int &destY_return,
                                                int &width_return, int &height_return);

        /** Window rendering job initialization. */
        virtual void windowRenderingJobInit(const XRenderWindow &window, int &op_return,
                                            Picture &maskPic_return);

        /** Window rendering job cleanup. */
        virtual void windowRenderingJobCleanup(const XRenderWindow &window);

        /** Extra post window rendering job. */
        virtual void extraPostWindowRenderingJob(const XRenderWindow &window, int &op_return,
                                                 Picture &srcPic_return, int &srcX_return, int &srcY_return,
                                                 Picture &maskPic_return, int &maskX_return, int &maskY_return,
                                                 int &destX_return, int &destY_return,
                                                 int &width_return, int &height_return);


        /** Called before the extra rendering jobs are executed. */
        virtual void preExtraRenderingActions();

        /** \returns the number of extra rendering jobs the plugin will do. */
        virtual int extraRenderingJobCount();

        /** Initialize the specified extra rendering job. */
        virtual void extraRenderingJobInit(int job, int &op_return,
                                           Picture &srcPic_return, int &srcX_return, int &srcY_return,
                                           Picture &maskPic_return, int &maskX_return, int &maskY_return,
                                           int &destX_return, int &destY_return,
                                           int &width_return, int &height_return);

        /** Clean up after an extra rendering job. */
        virtual void extraRenderingJobCleanup(int job);

        /** Called after the extra rendering jobs are executed. */
        virtual void postExtraRenderingActions();
    };
}

#endif  // FBCOMPOSITOR_XRENDERPLUGIN_HH
