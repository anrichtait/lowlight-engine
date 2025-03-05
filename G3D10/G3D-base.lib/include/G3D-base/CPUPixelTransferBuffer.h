/**
  \file G3D-base.lib/include/G3D-base/CPUPixelTransferBuffer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_CPUPixelTransferBuffer_h

#include "G3D-base/platform.h"
#include "G3D-base/PixelTransferBuffer.h"

namespace G3D {

/** A PixelTransferBuffer in main memory.
    \sa GLPixelTransferBuffer, Image */
class CPUPixelTransferBuffer : public PixelTransferBuffer {
protected:

    shared_ptr<MemoryManager>   m_memoryManager;
    void*                       m_buffer;

    void allocateBuffer(const shared_ptr<MemoryManager>& memoryManager);
    void freeBuffer();

    CPUPixelTransferBuffer(const ImageFormat* format, int width, int height, int depth, int rowAlignment);

public:

    /** Creates a buffer backed by a CPU array of uninitialized contents. */
    static shared_ptr<CPUPixelTransferBuffer> create
        (int                            width, 
         int                            height, 
         const ImageFormat*             format, 
         const shared_ptr<MemoryManager>&memoryManager   = MemoryManager::create(), 
         int                            depth           = 1, 
         int                            rowAlignment    = 1);

    /** Creates a buffer backed by a CPU array of existing data that will be
        managed by the caller.  It is the caller's responsibility to ensure
        that data remains allocated while the buffer
        is in use. */
    static shared_ptr<CPUPixelTransferBuffer> fromData
        (int                            width, 
         int                            height, 
         const ImageFormat*             format, 
         void*                          data,
         int                            depth           = 1, 
         int                            rowAlignment    = 1);

    virtual ~CPUPixelTransferBuffer();

    /** Returns pointer to raw pixel data */
    void* buffer()                      { return m_buffer; }

    /** Returns pointer to raw pixel data */
    const void* buffer() const          { return m_buffer; }

    virtual bool ownsMemory() const override {
        return notNull(m_memoryManager);
    }

    /** Return row to raw pixel data at start of row \a y of depth \a d. */
    void* row(int y, int d = 0) {
        debugAssert(y < m_height && d < m_depth);
        return static_cast<uint8*>(m_buffer) + rowOffset(y, d); 
    }

    /** Return row to raw pixel data at start of row \a y of depth \a d.
    */
    const void* row(int y, int d = 0) const {
        debugAssert(y < m_height && d < m_depth);
        return const_cast<CPUPixelTransferBuffer*>(this)->row(y, d); 
    }

    virtual void* mapReadWrite() override;

    virtual void* mapWrite() override;

    virtual const void* mapRead() const override;

    virtual void unmap() const override;

    virtual bool readyToMap() const override {
        return true;
    }

    virtual bool requiresGPUContext() const override {
        return false;
    }

    /** Overwrite the current contents with \a data.  Cannot call while mapped. */
    virtual void setData(const void* data) override;

    /** Read back the current contents to \a data.  Cannot call while mapped. */
    virtual void getData(void* data) const override;
};

} // namespace G3D

