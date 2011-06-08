/** Logging.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_LOGGING_HH
#define FBCOMPOSITOR_LOGGING_HH


namespace FbCompositor {

    //--- LOGGING LEVELS -------------------------------------------------------

    const int LOG_LEVEL_NONE = 0;
    const int LOG_LEVEL_ERROR = 1;
    const int LOG_LEVEL_WARN = 2;
    const int LOG_LEVEL_INFO = 3;
    const int LOG_LEVEL_DEBUG = 4;


    //--- LOG MANAGER ----------------------------------------------------------

    /**
     * The log manager class.
     */
    class Logger {
    public :
        /** \returns the current logging level. */
        static int loggingLevel();

        /** Sets a new logging level. */
        static void setLoggingLevel(int newLevel);

    private :
        /** The logging level. */
        static int m_level;
    };

}


#define fbLog_error fbLog_internal(FbCompositor::LOG_LEVEL_ERROR, "[Error] ")
#define fbLog_warn fbLog_internal(FbCompositor::LOG_LEVEL_WARN, "[Warn] ")
#define fbLog_info fbLog_internal(FbCompositor::LOG_LEVEL_INFO, "[Info] ")
#define fbLog_debug fbLog_internal(FbCompositor::LOG_LEVEL_DEBUG, "[Debug] ")

#define fbLog_internal(minLevel, levelName) if (FbCompositor::Logger::loggingLevel() >= (minLevel)) std::cerr << (levelName)


#endif  // FBCOMPOSITOR_LOGGING_HH
