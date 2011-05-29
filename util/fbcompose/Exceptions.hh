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

#include <exception>
#include <string>


namespace FbCompositor {

    //--- BASE EXCEPTION -------------------------------------------------------

    /**
     * Base class for all exceptions in the project's namespace.
     */
    class CompositorException : public std::exception {
    public:
        /** The constructor. */
        CompositorException(std::string errorMessage) throw() :
            m_errorMessage(errorMessage) {}

        /** Destructor.  */
        virtual ~CompositorException() throw() {}

        /** \returns The main error message.  */
        virtual const char *what() const throw() { return m_errorMessage.c_str(); }

    private:
        /** The exception's main message. */
        std::string m_errorMessage;
    };


    //--- DERIVED EXCEPTIONS ---------------------------------------------------

    /**
     * This exception is thrown whenever an error occurs while processing the
     * compositor's configuration data.
     */
    class ConfigException : public CompositorException {
    public:
        /** Public constructor. */
        ConfigException(std::string errorMessage) throw() :
            CompositorException(errorMessage) {}

        /** Destructor. */
        virtual ~ConfigException() throw() {}
    };


    /**
     * This exception is thrown whenever the an out of bounds index is supplied
     * to a function.
     */
    class IndexOutOfBoundsException : public CompositorException {
    public:
        /** Public constructor. */
        IndexOutOfBoundsException(std::string errorMessage) throw() :
            CompositorException(errorMessage) {}

        /** Destructor. */
        virtual ~IndexOutOfBoundsException() throw() {}
    };

}

#endif  // FBCOMPOSITOR_EXCEPTIONS_HH
