/**
  \file G3D-gfx.lib/source/Profiler.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/stringutils.h"
#include "G3D-base/Log.h"
#include "G3D-gfx/Profiler.h"
#include "G3D-gfx/glcalls.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-gfx/glheaders.h"
#include "G3D-gfx/RenderDevice.h"

namespace G3D {
    
thread_local shared_ptr<Profiler::ThreadInfo>*  Profiler::s_threadInfo = nullptr;
thread_local int                                Profiler::s_level = 0;

Array< shared_ptr<Profiler::ThreadInfo> >       Profiler::s_threadInfoArray;
std::mutex                                      Profiler::s_profilerMutex;
uint64                                          Profiler::s_frameNum = 0;
bool                                            Profiler::s_enabled = false;
bool                                            Profiler::s_timeShaderLaunches = true;

void Profiler::set_LAUNCH_SHADER_timingEnabled(bool enabled) {
    s_timeShaderLaunches = enabled;
}

   
bool Profiler::LAUNCH_SHADER_timingEnabled() {
    return s_timeShaderLaunches;
}


GLuint Profiler::ThreadInfo::getQueryLocationObject(int eventIndex, QueryLocation location) {
    const int NUM_QUERY_OBJECTS_INCREMENT = NUM_QUERY_LOCATIONS * 50;
    if (nextQueryObjectIndex == queryObjects.size()) {
        queryObjects.resize(queryObjects.size() + NUM_QUERY_OBJECTS_INCREMENT);
            
        glGenQueries(NUM_QUERY_OBJECTS_INCREMENT, queryObjects.getCArray() + nextQueryObjectIndex);
        debugAssertGLOk();
    }

    const bool isNewEvent = (eventIndex * NUM_QUERY_LOCATIONS) == nextQueryObjectIndex;
    if (isNewEvent) {
        GLuint newObject = queryObjects[nextQueryObjectIndex + location];
        nextQueryObjectIndex += NUM_QUERY_LOCATIONS;
        return newObject;
    } else {
        return queryObjects[eventIndex * NUM_QUERY_LOCATIONS + location];
    }
}


Profiler::ThreadInfo::~ThreadInfo() {
    glDeleteQueries(queryObjects.size(), queryObjects.getCArray());
    debugAssertGLOk();
    queryObjects.clear();
}


void Profiler::ThreadInfo::beginEvent(const String& name, const String& file, int line, const String& hint) {
    Event event;
    event.m_hash = HashTrait<String>::hashCode(name) ^ HashTrait<String>::hashCode(file) ^ size_t(line) ^ HashTrait<String>::hashCode(hint);

    if (ancestorStack.length() == 0) {
        event.m_parentIndex = -1;
    } else {
        event.m_parentIndex = ancestorStack.last();
        if (eventTree[event.m_parentIndex].m_numChildren == 0) {
            ++eventTree[event.m_parentIndex].m_numChildren;

            Event dummy;
            dummy.m_name = "other";
            dummy.m_file = eventTree[event.m_parentIndex].m_file;
            dummy.m_line = eventTree[event.m_parentIndex].m_line;
            dummy.m_level = s_level;
            dummy.m_hash = eventTree[event.m_parentIndex].hash() ^ HashTrait<String>::hashCode(dummy.m_name);
            dummy.m_numChildren = -1; // indicates dummy event
            dummy.m_parentIndex = ancestorStack.last();

            // push dummy query objects
            getQueryLocationObject(eventTree.length(), QUERY_LOCATION_START);
            getQueryLocationObject(eventTree.length(), QUERY_LOCATION_END);

            eventTree.append(dummy);
        }
        ++eventTree[event.m_parentIndex].m_numChildren;
        event.m_hash = eventTree[event.m_parentIndex].m_hash ^ event.m_hash;
        const Event& prev = eventTree.last();
        if (prev.m_name == name && prev.m_file == file && prev.m_line == line && prev.m_hint == hint) {
            event.m_hash = prev.m_hash + 1;
        }
    }
    ancestorStack.push(eventTree.length());

    if (RenderDevice::current) {
        // Set start location marker query object 
        glQueryCounter(getQueryLocationObject(eventTree.length(), QUERY_LOCATION_START), GL_TIMESTAMP);
        debugAssertGLOk();
    }

    // Take a CPU sample
    event.m_name = name;
    event.m_file = file;
    event.m_line = line;
    event.m_hint = hint;
    event.m_cpuStart = System::time();
    event.m_level = s_level;
    ++s_level;
    eventTree.append(event);
}


void Profiler::ThreadInfo::endEvent() {
    const int eventIndex = ancestorStack.pop();

    Event& event(eventTree[eventIndex]);

    if (RenderDevice::current) {
        // Set end location marker query object
        glQueryCounter(getQueryLocationObject(eventIndex, QUERY_LOCATION_END), GL_TIMESTAMP);
        debugAssertGLOk();
    }

    // Take a CPU sample
    event.m_cpuEnd = System::time();
    --s_level;
    debugAssertM(s_level >= 0, "More END_PROFILER_EVENT than BEGIN_PROFILER_EVENT calls!");
}

//////////////////////////////////////////////////////////////////////////

int Profiler::calculateUnaccountedTime(Array<Event>& eventTree, const int index, RealTime& cpuTime, RealTime& gpuTime) {
    const Event& event = eventTree[index];
    
    if (event.m_numChildren == 0) {
        cpuTime = event.cpuDuration();
        gpuTime = event.gfxDuration();
        return index + 1;    
    }        
    
    int childIndex = index + 2;
    RealTime totalChildCpu = 0;
    RealTime totalChildGfx = 0;
    for (int child = 1; child < event.m_numChildren; ++child) {
        RealTime childCpu = 0;
        RealTime childGfx = 0;
        childIndex = calculateUnaccountedTime(eventTree, childIndex, childCpu, childGfx);
        totalChildCpu += childCpu;
        totalChildGfx += childGfx;
    }

    Event& dummy = eventTree[index + 1];
    dummy.m_cpuStart = 0;
    dummy.m_cpuEnd = event.cpuDuration() - totalChildCpu;
    dummy.m_gfxStart = 0;
    dummy.m_gfxEnd = event.gfxDuration() - totalChildGfx;

    cpuTime = event.cpuDuration();
    gpuTime = event.gfxDuration();
    return childIndex;
}


void Profiler::threadShutdownHook() {
    std::lock_guard<std::mutex> guard(s_profilerMutex);
    const int i = s_threadInfoArray.findIndex(*s_threadInfo);
    alwaysAssertM(i != -1, "Could not find thread info during thread destruction for Profiler");
    s_threadInfoArray.remove(i);

    delete s_threadInfo;
    s_threadInfo = nullptr;
}


void Profiler::beginEvent(const String& name, const String& file, int line, const String& hint) {
    if (! s_enabled) { return; }

    if (isNull(s_threadInfo)) {
        // First time that this thread invoked beginEvent--intialize it
        s_threadInfo = new shared_ptr<ThreadInfo>(new ThreadInfo());
        std::lock_guard<std::mutex> guard(s_profilerMutex);
        s_threadInfoArray.append(*s_threadInfo);
    }

    (*s_threadInfo)->beginEvent(name, file, line, hint);
}


void Profiler::endEvent() {
    if (! s_enabled) { return; }
    (*s_threadInfo)->endEvent();
}


void Profiler::setEnabled(bool e) {
    s_enabled = e;
}


void Profiler::nextFrame() {
    if (! s_enabled) { return; }

    std::lock_guard<std::mutex> guard(s_profilerMutex);
    debugAssertGLOk();
    debugAssertM(s_level == 0, "More BEGIN_PROFILER_EVENT than END_PROFILER_EVENT calls!");

    // For each thread
    for (int t = 0; t < s_threadInfoArray.length(); ++t) {
        const shared_ptr<ThreadInfo>& info = s_threadInfoArray[t];

        for (int e = 0; e < info->eventTree.length(); ++e) {
            Event& event = info->eventTree[e];
            GLuint startQueryObject = info->getQueryLocationObject(e, ThreadInfo::QUERY_LOCATION_START);
            GLuint endQueryObject = info->getQueryLocationObject(e, ThreadInfo::QUERY_LOCATION_END);
            if (! event.isDummy()) {
#               ifdef G3D_OSX
                    // Attempt to get OS X profiling working:

                    // Wait for the result if necessary
                    const int maxCount = 1000000;
                    int count;
                    GLint available = GL_FALSE;
                    glGetQueryObjectiv(startQueryObject, GL_QUERY_RESULT_AVAILABLE, &available); 
                    for (count = 0; (available != GL_TRUE) && (count < maxCount); ++count) {
                        glGetQueryObjectiv(startQueryObject, GL_QUERY_RESULT_AVAILABLE, &available);
                    }
                
                    if (count > maxCount) {
                        debugPrintf("Timed out on GL_QUERY_RESULT_AVAILABLE\n");
                    }
#               endif

                // Take the GFX times now
                // Get the time, in nanoseconds
                GLuint64EXT ns = 0;
                glGetQueryObjectui64v(startQueryObject, GL_QUERY_RESULT, &ns);
                debugAssertGLOk();
                event.m_gfxStart = ns * 1e-9;

                glGetQueryObjectui64v(endQueryObject, GL_QUERY_RESULT, &ns);
                // If you get an OpenGL error here, it is probably because there are mismatched BEGIN_PROFILER_EVENT...END_PROFILER_EVENT calls.
                debugAssertGLOk();
                event.m_gfxEnd = ns * 1e-9;
            } // if GFX
        } // e

        RealTime a, b;
        for (int e = 0; e < info->eventTree.length(); e = calculateUnaccountedTime(info->eventTree, e, a, b));

        // Reset query object array
        info->nextQueryObjectIndex = 0;

        // Swap pointers (to avoid a massive array copy)
        Array<Event>::swap(info->previousEventTree, info->eventTree);

        // Clear (but retain the underlying storage to avoid allocation next frame)
        info->eventTree.fastClear();
    } // t

    ++s_frameNum;
}

void Profiler::getEvents(Array<const Array<Event>*>& eventTrees) {
    eventTrees.fastClear();
    std::lock_guard<std::mutex> guard(s_profilerMutex);

    for (int t = 0; t < s_threadInfoArray.length(); ++t) {
        eventTrees.append(&s_threadInfoArray[t]->previousEventTree);
    }
}


void Profiler::getEventTime(const String& eventName, RealTime& cpuTime, RealTime& gfxTime) {
    cpuTime = 0;
    gfxTime = 0;
    std::lock_guard<std::mutex> guard(s_profilerMutex);
    for (int t = 0; t < s_threadInfoArray.length(); ++t) {
        for (const Event& e : s_threadInfoArray[t]->previousEventTree) {
            if (e.name() == eventName) {
                cpuTime += e.cpuDuration();
                gfxTime += e.gfxDuration();
            }
        }
    }
}


} // namespace G3D
