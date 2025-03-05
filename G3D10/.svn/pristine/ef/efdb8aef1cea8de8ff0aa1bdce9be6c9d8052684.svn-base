/**
  \file G3D-base.lib/include/G3D-base/Stopwatch.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_Stopwatch_h

#include "G3D-base/platform.h"
#include "G3D-base/chrono.h"
#include "G3D-base/G3DGameUnits.h"
#include "G3D-base/G3DString.h"


namespace G3D {

/**
    \brief Provides accurate measurement of sequences of durations.

    Example: For profiling sequences of code
    <pre>
        ContinousStopwatch sw("My Stopwatch", true);
        slowOperation();
        sw.printElapsedTime("slowOperation");
        kdTree.balance();
        sw.printElapsedTime("Balance tree");
    </pre>

    \sa Stopwatch
*/
class ContinuousStopwatch {
private:
    using clock = std::chrono::steady_clock;

    String              m_name;
    bool                m_enabled;
    clock::time_point   m_startTime;
    clock::time_point   m_previousTime;
    String              m_previousMarker;

public:
    ContinuousStopwatch(const String& name = "Stopwatch", bool enabled = false);

    /** Set whether printElapsedTime output is enabled. */
    void setEnabled(bool enabled) { 
        m_enabled = enabled;
    }

    bool enabled() const { 
        return m_enabled;
    }

    /** Get elapsed time since previous marker and start of sequence */
    const RealTime getElapsedTime();

    /** Prints the elapsed time since previous marker and start of sequence */
    void printElapsedTime(const String& marker = String());

    /** Restarts the stopwatch with a new starting time */
    void restart();
};

/**
    \brief Provides accurate measurement of durations.

    Example 1: For profiling code
    <pre>
        Stopwatch sw;
        sw.tick();
        ...timed code...
        sw.tock();

        screenPrintf("%f\n", sw.elapsedTime());
    </pre>

    Example 2: For profiling sequences of code
    <pre>
        Stopwatch sw;
        sw.setEnabled(true);
        slowOperation();
        sw.printElapsedTime("slowOperation");
        kdTree.balance();
        sw.printElapsedTime("Balance tree");
    </pre>

    \sa ContinuousStopwatch
 */
class Stopwatch {
private:
    using clock = std::chrono::steady_clock;

    String                      m_name;
    bool                        m_isAfterTick;

    clock::time_point           m_tick;

    clock::duration             m_duration;
    clock::duration             m_ewmaDuration;

    ContinuousStopwatch         m_continuous;

public:

    Stopwatch(const String& name = "Stopwatch");

    /** Call at the beginning of the period that you want timed. */
    void tick();

    /** Call at the end of the period that you want timed. */
    void tock();

    /** Amount of time between the most recent tick and tock calls.  
        0 if tick has never been called. */
    RealTime elapsedTime() const {
        return elapsedDuration<std::chrono::duration<double>>().count();
    }

    /** Time-smoothed value that is stable to the nearest 1%.
        This is useful if you are displaying elapsed time in real-time 
        and want a stable number.*/
    RealTime smoothElapsedTime() const {
        return smoothElapsedDuration<std::chrono::duration<double>>().count();
    }

    /** Provides the elapsed time as a std::chrono::duration.
        Use this if you want resolution smaller than seconds before rounding. */
    template<class Duration = chrono::nanoseconds>
    Duration elapsedDuration() const {
        return std::chrono::duration_cast<Duration>(m_duration);
    }

    /** Provides the smooth elapsed time as a std::chrono::duration.
        Use this if you want resolution smaller than seconds before rounding. */
    template<class Duration = chrono::nanoseconds>
    Duration smoothElapsedDuration() const {
        return std::chrono::duration_cast<Duration>(m_ewmaDuration);
    }

    /** /deprecated Use ContinuousStopwatch */
    [[deprecated]] void setEnabled(bool e) {
        m_continuous.setEnabled(e);
    }

    /** /deprecated Use ContinuousStopwatch */
    [[deprecated]] bool enabled() const {
        return m_continuous.enabled();
    }

    /** /deprecated Use ContinuousStopwatch */
    [[deprecated]] void reset() {
        m_continuous.restart();
    }

    /** /deprcated Use ContinuousStopwatch */
    [[deprecated]] void printElapsedTime(const String& s = String()) {
        m_continuous.printElapsedTime(s);
    }

};

} // namespace G3D

