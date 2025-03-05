/**
  \file G3D-gfx.lib/include/G3D-gfx/Profiler.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_Profiler_h
#define G3D_Profiler_h

#include "G3D-base/platform.h"
#include "G3D-base/Array.h"
#include "G3D-base/G3DGameUnits.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/Table.h"
#include <mutex>

typedef int GLint;
typedef unsigned int GLuint;
#ifndef GL_NONE
#   define GL_NONE (0)
#   define DEFINED_GL_NONE 1
#endif

namespace G3D {

/** 
    \brief Measures execution time of CPU and GPU events across multiple threads.

    

 */
class Profiler {
public:

    /**
      May have child Events.
     */
    class Event {
    private:
        friend class Profiler;

        String          m_name;
        String          m_file;
        String          m_hint;
        int             m_line;
        /** a unique identifier that is the events parent hash plus the hash of its hint and the hash of the its shader file and line number */
        size_t          m_hash;

        /** Relative to an arbitrary baseline */
        RealTime        m_gfxStart;
        RealTime        m_gfxEnd;

        /** Unix time */
        RealTime        m_cpuStart;
        RealTime        m_cpuEnd;

        int             m_numChildren;
        int             m_parentIndex;

        int             m_level;

    public:

        /** For the root's parent */
        enum { NONE = -1 };

        Event() : m_line(0), m_hash(0), m_gfxStart(nan()), m_gfxEnd(nan()), m_cpuStart(nan()), m_cpuEnd(nan()), m_numChildren(0),
            m_parentIndex(NONE), m_level(0) {
        }

        /** Tree level, 0 == root.  This information can be inferred from the tree structure but
            is easiest to directly query. */
        int level() const {
            return m_level;
        }

        /** Number of child events.  Descendents are expanded in depth-first order. */
        int numChildren() const {
            return m_numChildren == -1 ? 0 : m_numChildren;
        }

        /** Index in the event tree of this node's parent, NONE if this is the root. */
        int parentIndex() const {
            return m_parentIndex;
        }

        /** Whether or not event is a generated dummy event */
        int isDummy() const {
            return m_numChildren == -1;
        }

        /** The name provided for this event when it began. For auto-generated shader events from LAUNCH_SHADER,
            this will be the name of the shader.

            Note that event names are not necessarily unique. The location of an event within the tree is the
            only unique identification.
            */
        const String& name() const {
            return m_name;
        }

        /** The name of the C++ file in which the event began. */
        const String& file() const {
            return m_file;
        }

        const String& hint() const {
            return m_hint;
        }

        const size_t hash() const {
            return m_hash;
        }

        /** The line number in file() at which the event began. */
        int line() const {
            return m_line;
        }

        /** Unix time at which Profiler::beginEvent() was called to create this event. Primarily useful for ordering events on a timeline.
            \see cpuDuration, gfxDuration, endTime */
        RealTime startTime() const {
            return m_cpuStart;
        }

        /** Unix time at which Profiler::endEvent() was called to create this event. */
        RealTime endTime() const {
            return m_cpuEnd;
        }

        /** Time elapsed between when the GPU began processing this task and when it completed it,
            including the time consumed by its children. The GPU may have been idle for some of that
            time if it was blocked on the CPU or the event began before significant GPU calls were
            actually issued by the program.*/
        RealTime gfxDuration() const {
            return m_gfxEnd - m_gfxStart;
        }

        /** Time elapsed between when the CPU began processing this task and when it completed it,
            including the time consumed by its children. */
        RealTime cpuDuration() const {
            return m_cpuEnd - m_cpuStart;
        }

        bool operator==(const String& name) const {
            return m_name == name;
        }

        bool operator!=(const String& name) const {
            return m_name != name;
        }
    };

private:
    
    /** Per-thread profiling information */
    class ThreadInfo {
    public:
        /** GPU query objects available for use.*/
        Array<GLuint>                       queryObjects;
        int                                 nextQueryObjectIndex;

        /** Full tree of all events for the current frame on the current thread */
        Array<Event>                        eventTree;

        /** Indices of the ancestors of the current event, in m_taskTree */
        Array<int>                          ancestorStack;

        /** Full tree of events for the previous frame */
        Array<Event>                        previousEventTree;

        void beginEvent(const String& name, const String& file, int line, const String& hint = "");

        void endEvent();

        enum QueryLocation {
            QUERY_LOCATION_START,
            QUERY_LOCATION_END,
            NUM_QUERY_LOCATIONS
        };

        /** Retrieves query objects for event. Allocates objects if new event. */
        GLuint getQueryLocationObject(int eventIndex, QueryLocation location);

        ~ThreadInfo();
    };

    /** Information about the current thread. Initialized by beginEvent */
    static thread_local shared_ptr<ThreadInfo>* s_threadInfo;

    static thread_local int                     s_level;

    /** Stores information about all threads for the current frame */
    static Array<shared_ptr<ThreadInfo> >   s_threadInfoArray;

    static std::mutex                       s_profilerMutex;

    /** Whether to make profile events in every LAUNCH_SHADER call. Default is true. */
    static bool                             s_timeShaderLaunches;

    /** Updated on every call to nextFrame() to ensure that events from different frames are never mixed. */
    static uint64                           s_frameNum;

    static bool                             s_enabled;

    static int calculateUnaccountedTime(Array<Event>& eventTree, const int index, RealTime& cpuTime, RealTime& gpuTime);

    /** Prevent allocation using this private constructor */
    Profiler() {}

public:

    /** Do not call directly if using Thread. Registered with Thread to deallocate the ThreadInfo for a thread.
        Must be explicitly invoked if you use a different thread API. */
    static void threadShutdownHook();

    /** Notify the profiler to latch the current event tree. 
        Events are always presented one frame late so that 
        that information is static and independent of when
        the caller requests it within the frame.
    
        Invoking nextFrame may stall the GPU and CPU by blocking in
        the method, causing your net frame time to appear to increase.
        This is (correctly) not reflected in the values returned by event timers.

        GApp calls this automatically.  Note that this may cause OpenGL errors and race conditions
        in programs that use multiple GL contexts if there are any outstanding
        events on any thread at the time that it is invoked. It is the programmer's responsibility to ensure that that 
        does not happen.
    */
    static void nextFrame();

    /** When disabled no profiling occurs (i.e., beginCPU and beginGFX
        do nothing).  Since profiling can affect performance
        (nextFrame() may block), top framerate should be measured with
        profiling disabled. */
    static bool enabled() {
        return s_enabled;
    }

    /** \copydoc enabled() */
    static void setEnabled(bool e);

    /** Calls to beginEvent may be nested on a single thread. Events on different
        threads are tracked independently.*/
    static void beginEvent(const String& name, const String& file, int line, const String& hint = "");
    
    /** Ends the most recent pending event on the current thread. */
    static void endEvent();

    /** Return all events from the previous frame, one array per thread. The underlying
        arrays will be mutated when nextFrame() is invoked.
        
        The result has the form: <code>const Profiler::Event& e = (*(eventTrees[threadIndex]))[eventIndex]</code>
        The events are stored as the depth-first traversal of the event tree.
        See the Profiler::Event documentation for information about identifying
        the roots and edges within each tree. 
    */
    static void getEvents(Array<const Array<Event>*>& eventTrees);

    /** Set whether to make profile events in every LAUNCH_SHADER call. 
        Useful for when you only want to time a small amount of thing, or just the aggregate of many launches. */
    static void set_LAUNCH_SHADER_timingEnabled(bool enabled);

    /** Returns the sum of all time spent in events with this name (which may be zero,
        if they do not exist) across all threads. */
    static void getEventTime(const String& eventName, RealTime& cpuTime, RealTime& gfxTime);

    /** Whether to make profile events in every LAUNCH_SHADER call. Default is true. */
    static bool LAUNCH_SHADER_timingEnabled();
};

} // namespace G3D 

/** \def BEGIN_PROFILER_EVENT 

   Defines the beginning of a profilable event.  Example:
    \code
   BEGIN_PROFILER_EVENT("MotionBlur");
   ...
   END_PROFILER_EVENT("MotionBlur");
   \endcode

   The event name must be a compile-time constant char* or String.

   \sa END_PROFILER_EVENT, Profiler, Profiler::beginEvent
 */

#define BEGIN_PROFILER_EVENT_WITH_HINT(eventName, hint) { static const String& __profilerEventName = (eventName); Profiler::beginEvent(__profilerEventName, __FILE__, __LINE__, hint); }
#define BEGIN_PROFILER_EVENT(eventName) { BEGIN_PROFILER_EVENT_WITH_HINT(eventName, "") }
/** \def END_PROFILER_EVENT 
    \sa BEGIN_PROFILER_EVENT, Profiler, Profiler::endEvent
    */
#define END_PROFILER_EVENT() Profiler::endEvent()

#ifdef DEFINED_GL_NONE
#   undef GL_NONE
#   undef DEFINED_GL_NONE
#endif

#endif
