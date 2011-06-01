/** Constants.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_CONSTANTS_HH
#define FBCOMPOSITOR_CONSTANTS_HH

namespace FbCompositor {

    //--- MINIMUM EXTENSION VERSIONS -------------------------------------------

    /** Minimum major version for the GLX extension. */
    const int MIN_GLX_MAJOR_VERSION = 1;

    /** Minimum minor version for the GLX extension. */
    const int MIN_GLX_MINOR_VERSION = 4;

    /** Minimum major version for the XComposite extension. */
    const int MIN_XCOMPOSITE_MAJOR_VERSION = 0;

    /** Minimum minor version for the XComposite extension. */
    const int MIN_XCOMPOSITE_MINOR_VERSION = 3;

    /** Minimum major version for the XDamage extension. */
    const int MIN_XDAMAGE_MAJOR_VERSION = 1;

    /** Minimum minor version for the XDamage extension. */
    const int MIN_XDAMAGE_MINOR_VERSION = 0;


    //--- ENUMERATIONS ---------------------------------------------------------

    /** Rendering mode enumeration. */
    enum RenderingMode { RM_OpenGL, RM_XRender, RM_ServerAuto };

}

#endif  // FBCOMPOSITOR_CONSTANTS_HH
