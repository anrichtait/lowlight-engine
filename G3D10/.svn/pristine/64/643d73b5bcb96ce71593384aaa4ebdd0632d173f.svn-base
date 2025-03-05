/**
  \file G3D-base.lib/source/Stopwatch.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/Stopwatch.h"
#include "G3D-base/System.h"

namespace G3D {

ContinuousStopwatch::ContinuousStopwatch(const String& name, bool enabled) 
    : m_name(name)
    , m_enabled(enabled) {
    restart();
}

const RealTime ContinuousStopwatch::getElapsedTime()
{
    const clock::time_point now = clock::now();
    const RealTime totalElapsedTime = std::chrono::duration_cast<chrono::seconds>(now - m_startTime).count();
    return totalElapsedTime;
}

void ContinuousStopwatch::printElapsedTime(const String& marker) {
    const clock::time_point now = clock::now();
    const RealTime lastElapsedTime = std::chrono::duration_cast<chrono::seconds>(now - m_previousTime).count();
    const RealTime totalElapsedTime = std::chrono::duration_cast<chrono::seconds>(now - m_startTime).count();
    if (m_enabled) {
        debugPrintf("%s: %10s - %8fs since %s (%fs since start)\n",
            m_name.c_str(),
            marker.c_str(),
            lastElapsedTime,
            m_previousMarker.c_str(),
            totalElapsedTime);
    }
    m_previousTime = now;
    m_previousMarker = marker;
}

void ContinuousStopwatch::restart() {
    m_previousTime = m_startTime = clock::now();
    m_previousMarker = "start";
}



Stopwatch::Stopwatch(const String& name)
    : m_name(name)
    , m_isAfterTick(false)
    , m_continuous(name, false) {

}

void Stopwatch::tick() {
    // ensure we assert in release builds when this is most used
    alwaysAssertM(!m_isAfterTick, "Stopwatch::tick() called twice in a row.");
    m_isAfterTick = true;

    m_tick = clock::now();
}

void Stopwatch::tock() {
    clock::time_point now = clock::now();
    m_duration = now - m_tick;

    const auto maxDuration = max(m_duration.count(), m_ewmaDuration.count());
    const auto minDuration = min(m_duration.count(), m_ewmaDuration.count());
    if ((maxDuration - minDuration) > (m_ewmaDuration.count() / 2)) {
        // if the new duration is more than 50% different than current ewma duration
        // weight heavily towards the new duration
        m_ewmaDuration = (m_duration * 50) / 100 + (m_ewmaDuration * 50) / 100;
    } else {
        // normal weight
        m_ewmaDuration = (m_duration * 5) / 100 + (m_ewmaDuration * 95) / 100;
    }

    // ensure we assert in release builds when this is most used
    alwaysAssertM(m_isAfterTick, "Stopwatch::tock() called before Stopwatch::tick().");
    m_isAfterTick = false;
}

} // namespace G3D
