/**
  \file G3D-base.lib/include/G3D-base/Thread.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once

#include "G3D-base/G3DString.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/platform.h"
#include "G3D-base/SpawnBehavior.h"
#include "G3D-base/Vector2int32.h"
#include "G3D-base/Vector3int32.h"
#include <atomic>
#include <functional>
#include <thread>

#ifndef G3D_WINDOWS
#   include <unistd.h> // For usleep
#endif


namespace G3D {

/**
   \brief A mutual exclusion lock that busy-waits when locking.

   On a machine with one (significant) thread per processor core,
   a Spinlock may be substantially faster than a mutex.
 */
class Spinlock {
private:
    std::atomic_flag m_flag;

public:
    Spinlock() {
        m_flag.clear();
    }

    /** Busy waits until the lock is unlocked, then locks it
        exclusively. 
        
        A single thread cannot re-enter
        Spinlock::lock() if already locked.
     */
    void lock() {
        while (m_flag.test_and_set(std::memory_order_acquire)) {
#           ifdef G3D_WINDOWS
                Sleep(0);
#           else
                usleep(0);
#           endif
        }
    }

    void unlock() {
        m_flag.clear(std::memory_order_release);
    }

};


/** 
    \brief Iterates over a 3D region using multiple threads and
    blocks until all threads have completed. Has highest coherence
    per thread in x, and then in blocks of y.
        
    <p> Evaluates \a
    object->\a method(\a x, \a y) for every <code>start.x <= x <
    upTo.x</code> and <code>start.y <= y < upTo.y</code>.
    Iteration is row major, so each thread can expect to see
    successive x values.  </p> 

    \param singleThread If true, force all computation to run on the
    calling thread. Helpful when debugging

    Example:

    \code
    class RayTracer {
    public:
        void trace(const Vector2int32& pixel) {
            ...
        }

        void traceAll() {
            Thread::runConcurrently(Point2int32(0,0), Point2int32(w,h), [this](pixel) { trace(pixel); );
        }
    };
    \endcode
*/
void runConcurrently
   (const Point3int32& start, 
    const Point3int32& stopBefore, 
    const std::function<void (Point3int32)>& callback,
    bool singleThread = false);

void runConcurrently
   (const Point2int32& start,
    const Point2int32& stopBefore, 
    const std::function<void (Point2int32)>& callback,
    bool singleThread = false);

void runConcurrently
   (const int& start, 
    const int& stopBefore, 
    const std::function<void (int)>& callback,
    bool singleThread = false);

void runConcurrently
   (const size_t& start, 
    const size_t& stopBefore, 
    const std::function<void (size_t)>& callback,
    bool singleThread = false);

} // namespace G3D

