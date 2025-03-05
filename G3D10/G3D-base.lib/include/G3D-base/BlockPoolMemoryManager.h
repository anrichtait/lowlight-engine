/**
  \file G3D-base.lib/include/G3D-base/BlockPoolMemoryManager.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#ifndef G3D_BlockPoolMemoryManager_h
#define G3D_BlockPoolMemoryManager_h

#include "G3D-base/MemoryManager.h"
#include "G3D-base/Set.h"
#include <mutex>

namespace G3D {

/** 
    \brief A MemoryManager that allocates fixed-size objects and
    maintains a freelist that never shrinks.  Useful for sharing
    work buffers among threads.
 */
class BlockPoolMemoryManager : public ReferenceCountedObject {
protected:
    
    const size_t         m_blockSize;
    Set<uint32*>         m_allBlocks;
    Array<uint32*>       m_freeList;
    mutable std::mutex   m_mutex;

    BlockPoolMemoryManager(size_t s) : m_blockSize(s) {}

public:
    
    size_t blockSize() const {
        return m_blockSize;
    }

    virtual ~BlockPoolMemoryManager() {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_freeList.invokeDeleteOnAllElements();
    }
    
    /** Return a pointer to \a s bytes of memory that are unused by
        the rest of the program.  The contents of the memory are
        undefined */
    virtual void* alloc(size_t s) {
        alwaysAssertM(s == m_blockSize, "BlockPoolMemoryManager can only allocate fixed-size blocks");
        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_freeList.size() == 0) {
            m_freeList.push(new uint32[iCeil(double(s) / sizeof(uint32))]);
            m_allBlocks.insert(m_freeList.last());
        }
        void* ptr = m_freeList.pop();
        debugAssert(ptr == nullptr || isValidHeapPointer(ptr));
        return ptr;
    }

    /** Returns the number of blocks currently sitting in the free list.*/
    int freeListNumBlocks() const {
        std::lock_guard<std::mutex> guard(m_mutex);
        int s = m_freeList.size();
        return s;
    }

    /** Total number of blocks ever allocated at once. */
    int peakNumBlocks() const {
        return m_allBlocks.size();
    }

    /** Invoke to declare that this memory will no longer be used by
        the program.  The memory manager is not required to actually
        reuse or release this memory. */
    virtual void free(void* _ptr) {
        if (_ptr != nullptr) {
            uint32* ptr = (uint32*)_ptr; 
            std::lock_guard<std::mutex> guard(m_mutex);
            debugAssertM(m_allBlocks.contains(ptr),
                         "Tried to BlockPoolMemoryManager::free a pointer not allocated by this memory manager.");
            debugAssertM(! m_freeList.contains(ptr), "Double free");
            m_freeList.push(ptr);
        }
    }

    /** Returns true if this memory manager is threadsafe (i.e., alloc
        and free can be called asychronously) */
    virtual bool isThreadsafe() const {
        return true;
    }

    /** Creates a new instance. Each instance is allowed to have its own block size. */
    static shared_ptr<BlockPoolMemoryManager> create(size_t blockSize) {
        return shared_ptr<BlockPoolMemoryManager>(new BlockPoolMemoryManager(blockSize));
    }
};

} // G3D

#endif
