/**
  \file G3D-base.lib/source/MemoryManager.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/MemoryManager.h"
#include "G3D-base/System.h"

namespace G3D {

MemoryManager::MemoryManager() {}


void* MemoryManager::alloc(size_t s) {
    return System::malloc(s);
}


void MemoryManager::free(void* ptr) {
    System::free(ptr);
}


bool MemoryManager::isThreadsafe() const {
    return true;
}


shared_ptr<MemoryManager> MemoryManager::create() {
    static shared_ptr<MemoryManager> m(new MemoryManager());
    return m;
}


///////////////////////////////////////////////////

AlignedMemoryManager::AlignedMemoryManager() {}


void* AlignedMemoryManager::alloc(size_t s) {
    return System::alignedMalloc(s, 16);
}


void AlignedMemoryManager::free(void* ptr) {
    System::alignedFree(ptr);
}


bool AlignedMemoryManager::isThreadsafe() const {
    return true;
}


shared_ptr<AlignedMemoryManager> AlignedMemoryManager::create() {
    static shared_ptr<AlignedMemoryManager> m = createShared<AlignedMemoryManager>();
    return m;
}


///////////////////////////////////////////////////

CRTMemoryManager::CRTMemoryManager() {}


void* CRTMemoryManager::alloc(size_t s) {
    return ::malloc(s);
}


void CRTMemoryManager::free(void* ptr) {
    return ::free(ptr);
}


bool CRTMemoryManager::isThreadsafe() const {
    return true;
}


shared_ptr<CRTMemoryManager> CRTMemoryManager::create() {
    static shared_ptr<CRTMemoryManager> m(new CRTMemoryManager());
    return m;
}
}
