/** Exceptions.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_EXCEPTIONS_HH
#define FBCOMPOSITOR_EXCEPTIONS_HH

#include "FbTk/FbString.hh"

#include <exception>


namespace FbCompositor {

    //--- BASE EXCEPTION -------------------------------------------------------

    /**
     * Base class for all exceptions in the project's namespace.
     */
    class CompositorException : public std::exception {
    public:
        /** The constructor. */
        CompositorException(FbTk::FbString errorMessage) :
            m_errorMessage(errorMessage) {}

        /** Destructor. */
        virtual ~CompositorException() throw() {}

        /** \returns The main error message.  */
        virtual const char *what() const throw() { return m_errorMessage.c_str(); }

    private:
        /** The exception's main message. */
        FbTk::FbString m_errorMessage;
    };


    //--- DERIVED EXCEPTIONS ---------------------------------------------------

    /**
     * This exception is thrown whenever an error condition is encountered in
     * initialization of compositor's components.
     */
    class InitException : public CompositorException {
    public:
        /** Public constructor. */
        InitException(FbTk::FbString errorMessage) :
            CompositorException(errorMessage) {}
    };

    /**
     * This exception is thrown whenever an error condition is encountered when
     * the compositor has been initialized and is running.
     */
    class RuntimeException : public CompositorException {
    public:
        /** Public constructor. */
        RuntimeException(FbTk::FbString errorMessage) :
            CompositorException(errorMessage) {}
    };


    //--- MORE CONCRETE EXCEPTION TYPES ----------------------------------------

    /**
     * This exception is thrown whenever an error condition is encountered
     * when processing the configuration data.
     */
    class ConfigException : public InitException {
    public:
        /** Public constructor. */
        ConfigException(FbTk::FbString errorMessage) :
            InitException(errorMessage) {}
    };

    /**
     * This exception is thrown whenever an error condition is encountered
     * while initializing (loading, creating instances etc) plugins.
     */
    class PluginException : public InitException {
    public:
        /** Public constructor. */
        PluginException(FbTk::FbString errorMessage) :
            InitException(errorMessage) {}
    };


    /**
     * This exception is thrown whenever an error occurs while attempting to
     * obtain current system time.
     */
    class TimeException : public RuntimeException {
    public:
        /** Public constructor. */
        TimeException(FbTk::FbString errorMessage) :
            RuntimeException(errorMessage) {}
    };

    /**
     * This exception is thrown whenever an error occurs while manipulating the
     * windows in response to X's events.
     */
    class WindowException : public RuntimeException {
    public:
        /** Public constructor. */
        WindowException(FbTk::FbString errorMessage) :
            RuntimeException(errorMessage) {}
    };
}

#endif  // FBCOMPOSITOR_EXCEPTIONS_HH
