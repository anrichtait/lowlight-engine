/**
  \file G3D-base.lib/source/AreaMemoryManager.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/AreaMemoryManager.h"
#include "G3D-base/System.h"

namespace G3D {

AreaMemoryManager::Buffer::Buffer(size_t size) : m_size(size), m_used(0) {
    // Allocate space for a lot of buffers.
    m_first = (uint8*)::malloc(m_size);
}


AreaMemoryManager::Buffer::~Buffer() {
    ::free(m_first);
}


void* AreaMemoryManager::Buffer::alloc(size_t s) {
    if (s + m_used > m_size) {
        return nullptr;
    } else {
        void* old = m_first + m_used;
        m_used += s;
        return old;
    }
}


bool AreaMemoryManager::isThreadsafe() const {
    return false;
}


shared_ptr<AreaMemoryManager> AreaMemoryManager::create(size_t sizeHint) {
    return shared_ptr<AreaMemoryManager>(new AreaMemoryManager(sizeHint));
}


AreaMemoryManager::AreaMemoryManager(size_t sizeHint) : m_sizeHint(sizeHint) {
    debugAssert(sizeHint > 0);
}


AreaMemoryManager::~AreaMemoryManager() {
    deallocateAll();
}


size_t AreaMemoryManager::bytesAllocated() const {
    return m_sizeHint * m_bufferArray.size();
}


void* AreaMemoryManager::alloc(size_t s) {
    void* n = (m_bufferArray.size() > 0) ? m_bufferArray.last()->alloc(s) : nullptr;
    if (n == nullptr) {
        // This buffer is full
        m_bufferArray.append(new Buffer(max(s, m_sizeHint)));
        return m_bufferArray.last()->alloc(s);
    } else {
        return n;
    }
}


void AreaMemoryManager::free(void* x) {
    // Intentionally empty; we block deallocate
}


void AreaMemoryManager::deallocateAll() {
    m_bufferArray.invokeDeleteOnAllElements();
    m_bufferArray.clear();
}

}
