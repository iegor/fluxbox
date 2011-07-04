/** TickTracker.cc file for the fluxbox compositor. */

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


#include "Logging.hh"
#include "TickTracker.hh"

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

// The accuracy of the timer (1.0 = 1 second).
const double TickTracker::EPSILON = 1e-6;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
TickTracker::TickTracker() throw() {
    m_isRunning = false;
    m_tickSize = 1000000;
    m_ticksPerSecond = 1.0;
}

// Copy constructor.
TickTracker::TickTracker(const TickTracker &other) throw() :
    m_isRunning(other.m_isRunning),
    m_startTime(other.m_startTime),
    m_tickSize(other.m_tickSize),
    m_ticksPerSecond(other.m_ticksPerSecond),
    m_observedTicks(other.m_observedTicks) {
}

// Assignment operator.
TickTracker &TickTracker::operator=(const TickTracker &other) throw() {
    if (this != &other) {
        m_isRunning = other.m_isRunning;
        m_startTime = other.m_startTime;
        m_tickSize = other.m_tickSize;
        m_ticksPerSecond = other.m_ticksPerSecond;
        m_observedTicks = other.m_observedTicks;
    }
    return *this;
}

// Destructor.
TickTracker::~TickTracker() throw() { }


//--- TIMER MANIPULATION -------------------------------------------------------

// Starts the timer.
void TickTracker::start() throw(RuntimeException) {
    if (m_isRunning) {
        fbLog_warn << "Starting a running timer." << std::endl;
    }

    if (gettimeofday(&m_startTime, NULL)) {
        throw TimeException("Cannot obtain the current time.");
    }
    m_observedTicks = 0;
    m_isRunning = true;
}

/** Stops the timer. */
void TickTracker::stop() throw() {
    if (!m_isRunning) {
        fbLog_warn << "Stopping an inactive timer." << std::endl;
    }

    m_isRunning = false;
}


//--- TIMER QUERIES ------------------------------------------------------------

// Returns the new number of elapsed ticks since last call of this function.
int TickTracker::newElapsedTicks() throw(RuntimeException) {
    int totalTicks = totalElapsedTicks();
    int newTicks = totalTicks - m_observedTicks;
    m_observedTicks = totalTicks;

    if (newTicks < 0) {
        return 0;
    } else {
        return newTicks;
    }
}

// Returns the total number of elapsed ticks.
int TickTracker::totalElapsedTicks() throw(RuntimeException) {
    static timeval now;

    if (gettimeofday(&now, NULL)) {
        throw TimeException("Cannot obtain the current time.");
    }

    // TODO: Make sure it reacts to time changes appropriately (especially backwards).
    return tickDifference(now, m_startTime);
}


// Sets the size of a tick.
void TickTracker::setTickSize(int usec) throw(RuntimeException) {
    if (usec < 1) {
        throw RuntimeException("Invalid tick size.");
    }

    m_tickSize = usec;
    m_ticksPerSecond = 1000000.0 / usec;
}


//--- INTERNAL FUNTIONS --------------------------------------------------------

// Returns the difference in time between two timevals.
// Function adapted from http://www.gnu.org/s/libc/manual/html_node/Elapsed-Time.html.
timeval TickTracker::timeDifference(timeval t1, timeval t2) throw() {
    int nsec;

    if (t1.tv_usec < t2.tv_usec) {
        nsec = (t2.tv_usec - t1.tv_usec) / 1000000 + 1;
        t2.tv_usec -= 1000000 * nsec;
        t2.tv_sec += nsec;
    }
    if (t1.tv_usec - t2.tv_usec > 1000000) {
        nsec = (t1.tv_usec - t2.tv_usec) / 1000000;
        t2.tv_usec += 1000000 * nsec;
        t2.tv_sec -= nsec;
    }

    timeval diff;
    diff.tv_sec = t1.tv_sec - t2.tv_sec;
    diff.tv_usec = t1.tv_usec - t2.tv_usec;
    return diff;
}

// Returns the difference between two timevals in ticks.
int TickTracker::tickDifference(const timeval &t1, const timeval &t2) throw() {
    timeval diff = timeDifference(t1, t2);

    double rawDiff = diff.tv_sec * m_ticksPerSecond + (double(diff.tv_usec) / m_tickSize);
    return int(rawDiff + EPSILON);
}
