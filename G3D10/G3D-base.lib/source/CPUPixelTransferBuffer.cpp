/**
  \file G3D-base.lib/source/CPUPixelTransferBuffer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/CPUPixelTransferBuffer.h"
#include "G3D-base/ImageFormat.h"

namespace G3D {

CPUPixelTransferBuffer::CPUPixelTransferBuffer(const ImageFormat* format, int width, int height, int depth, int rowAlignment) 
    : PixelTransferBuffer(format, width, height, depth, rowAlignment)
    , m_buffer(nullptr) {

}


shared_ptr<CPUPixelTransferBuffer> CPUPixelTransferBuffer::create(int width, int height, const ImageFormat* format, const shared_ptr<MemoryManager>& memoryManager, int depth, int rowAlignment) {
    const shared_ptr<CPUPixelTransferBuffer>& imageBuffer = createShared<CPUPixelTransferBuffer>(format, width, height, depth, rowAlignment);

    // Allocate buffer with memory manager, this reference now owns the buffer
    imageBuffer->allocateBuffer(memoryManager);

    return imageBuffer;
}


shared_ptr<CPUPixelTransferBuffer> CPUPixelTransferBuffer::fromData
   (int                            width, 
    int                            height, 
    const ImageFormat*             format, 
    void*                          data,
    int                            depth, 
    int                            rowAlignment) {

    debugAssert(notNull(data));
    debugAssert(width > 0 && height > 0 && depth > 0);
    const shared_ptr<CPUPixelTransferBuffer>& imageBuffer = createShared<CPUPixelTransferBuffer>(format, width, height, depth, rowAlignment);
    imageBuffer->m_buffer = data;  
    return imageBuffer;
}


CPUPixelTransferBuffer::~CPUPixelTransferBuffer() {
    debugAssertM(isNull(m_mappedPointer), "Missing call to CPUPixelTransferBuffer::unmap()");

    if (notNull(m_buffer) && notNull(m_memoryManager)) {
        // The CPUPixelTransferBuffer owns this memory and should free its
        freeBuffer();
    }
}


void CPUPixelTransferBuffer::allocateBuffer(const shared_ptr<MemoryManager>& memoryManager) {
    debugAssert(isNull(m_memoryManager));
    debugAssert(isNull(m_buffer));

    // set memory manager
    m_memoryManager = memoryManager;

    // allocate buffer
    m_rowStride = m_width * (m_format->cpuBitsPerPixel / 8);
    m_rowStride = (m_rowStride + (m_rowAlignment - 1)) & (~ (m_rowAlignment - 1));

    const size_t bufferSize = m_depth * m_height * m_rowStride;
    m_buffer = m_memoryManager->alloc(bufferSize);
}

    
void CPUPixelTransferBuffer::freeBuffer() {
    debugAssertM(isNull(m_mappedPointer), "Invoked CPUPixelTransferBuffer::freeBuffer while mapped");
    debugAssert(m_memoryManager);
    debugAssert(m_buffer);

    m_memoryManager->free(m_buffer);
    m_buffer = nullptr;
}


void* CPUPixelTransferBuffer::mapReadWrite() {
    debugAssertM(isNull(m_mappedPointer), "Duplicate calls to CPUPixelTransferBuffer::mapX()");
    m_mappedPointer = m_buffer;
    return m_mappedPointer;
}


void* CPUPixelTransferBuffer::mapWrite() {
    debugAssertM(isNull(m_mappedPointer), "Duplicate calls to CPUPixelTransferBuffer::mapX()");
    m_mappedPointer = m_buffer;
    return m_mappedPointer;
}


const void* CPUPixelTransferBuffer::mapRead() const {
    debugAssertM(isNull(m_mappedPointer), "Duplicate calls to CPUPixelTransferBuffer::mapX()");
    m_mappedPointer = m_buffer;
    return m_mappedPointer;
}


void CPUPixelTransferBuffer::unmap() const {
    debugAssertM(notNull(m_mappedPointer), "Duplicate calls to CPUPixelTransferBuffer::unmap()");
    m_mappedPointer = nullptr;
}


void CPUPixelTransferBuffer::setData(const void* data) { 
    debugAssertM(isNull(m_mappedPointer), "Illegal to invoke setData() while mapped");
    System::memcpy(m_buffer, data, size());
}


void CPUPixelTransferBuffer::getData(void* data) const {
    debugAssertM(isNull(m_mappedPointer), "Illegal to invoke getData() while mapped");
    System::memcpy(data, m_buffer, size());
}

} // namespace G3D
