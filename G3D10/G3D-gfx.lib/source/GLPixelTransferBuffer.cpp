/**
  \file G3D-gfx.lib/source/GLPixelTransferBuffer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-gfx/Milestone.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-gfx/glcalls.h"

namespace G3D {

static Array<GLuint> s_freeList;

GLPixelTransferBuffer::GLPixelTransferBuffer(const ImageFormat* format, int width, int height, int depth, const void* data, uint32 glUsageHint) :
    PixelTransferBuffer(format, width, height, depth, 0), m_glBufferID(GL_NONE), m_glUsageHint(glUsageHint) {
   
    // Allocate the PBO
    if (s_freeList.size() == 0) {
        glGenBuffers(1, &s_freeList.next());
    }
    m_glBufferID = s_freeList.pop();

    // Allocate the data, specify the most generic usage pattern.  When reading back from
    // a texture, we do not have to preallocate in this way.
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glBufferID);
    glBufferData(GL_PIXEL_PACK_BUFFER, size(), data, glUsageHint);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);
}


shared_ptr<GLPixelTransferBuffer> GLPixelTransferBuffer::create
(int                            width, 
 int                            height, 
 const ImageFormat*             format, 
 const void*                    data,
 int                            depth,
 uint32                         glUsageHint) {
    if ((GLCaps::api() == OSWindow::Settings::API_OPENGL_ES) || (GLCaps::api() == OSWindow::Settings::API_OPENGL_EGL)) {
        // Needed for Raspberry Pi
        // https://www.raspberrypi.org/forums/viewtopic.php?t=136093
        glUsageHint = GL_DYNAMIC_DRAW;
    }
    return createShared<GLPixelTransferBuffer>(format, width, height, depth, data, glUsageHint);
}


GLPixelTransferBuffer::~GLPixelTransferBuffer() {
    runReallocateHooks();
    // Free the OpenGL PBO
    s_freeList.push(m_glBufferID);
    m_glBufferID = GL_NONE;
}


void GLPixelTransferBuffer::resize(int newWidth, int newHeight, int newDepth) {
    alwaysAssertM(m_mappedPointer == nullptr, "Cannot resize while mapped");
    if (m_width == newWidth && m_height == newHeight && m_depth == newDepth) {
        // Nothing to do
        return;
    }
    runReallocateHooks();   
    m_width = newWidth;
    m_height = newHeight;
    m_depth = newDepth;
    m_rowStride = iCeil(m_width * m_format->cpuBitsPerPixel / 8.0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glBufferID);
    glBufferData(GL_PIXEL_PACK_BUFFER, size(), nullptr, m_glUsageHint);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);
}


void GLPixelTransferBuffer::deleteAllBuffers() {
    if (s_freeList.size() > 0) {
        glDeleteBuffers(s_freeList.size(), s_freeList.getCArray());
    }
    s_freeList.clear();
}


void GLPixelTransferBuffer::bindWrite() {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glBufferID);
    m_milestone = Milestone::create("Bind GLPixelTransferBuffer");
}


void GLPixelTransferBuffer::unbindRead() {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, GL_NONE);
    m_milestone = Milestone::create("Unbind GLPixelTransferBuffer");
}


void GLPixelTransferBuffer::bindRead() {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glBufferID);
    m_milestone = Milestone::create("Bind GLPixelTransferBuffer");
}


void GLPixelTransferBuffer::unbindWrite() {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);
    m_milestone = Milestone::create("Unbind GLPixelTransferBuffer");
}

void GLPixelTransferBuffer::bindAsShaderStorageBuffer(int bindpoint) {
#   ifdef G3D_OSX
        alwaysAssertM(false, "GLPixelTransferBuffer::bindAsShaderStorageBuffer is not supported on macOS."
            " Use a pixel shader instead of a compute pass and write to a Texture,"
            " then convert to a GLPixelTransferBuffer.");
#   endif
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glBufferID());
    debugAssertGLOk();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindpoint, glBufferID());
    debugAssertGLOk();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_ZERO);
    debugAssertGLOk();
}


void GLPixelTransferBuffer::map(GLenum access) const {
    debugAssertGLOk();
    runMapHooks();
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glBufferID);
    debugAssertGLOk();
    if ((GLCaps::api() == OSWindow::Settings::API_OPENGL_ES) || (GLCaps::api() == OSWindow::Settings::API_OPENGL_EGL)) {
        m_mappedPointer = glMapBufferOES(GL_PIXEL_PACK_BUFFER, access);
    } else {
        m_mappedPointer = glMapBuffer(GL_PIXEL_PACK_BUFFER, access);
    }
    debugAssertGLOk();
    debugAssert(m_mappedPointer);
}


void* GLPixelTransferBuffer::mapReadWrite() {
    debugAssertM(isNull(m_mappedPointer), "Duplicate calls to PixelTransferBuffer::mapX()");

    map(GL_READ_WRITE);
    return m_mappedPointer;
}


void* GLPixelTransferBuffer::mapWrite() {
    debugAssertM(isNull(m_mappedPointer), "Duplicate calls to PixelTransferBuffer::mapX()");

    map(GL_WRITE_ONLY);
    return m_mappedPointer;
}


const void* GLPixelTransferBuffer::mapRead() const {
    debugAssertM(isNull(m_mappedPointer), "Duplicate calls to PixelTransferBuffer::mapX()");
    map(GL_READ_ONLY);
    debugAssertM(notNull(m_mappedPointer), "Pointer was null after mapRead");
    return m_mappedPointer;
}


void GLPixelTransferBuffer::unmap() const {
    debugAssertM(notNull(m_mappedPointer), "Duplicate calls to PixelTransferBuffer::unmap()");

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glBufferID);
    if ((GLCaps::api() == OSWindow::Settings::API_OPENGL_ES) || (GLCaps::api() == OSWindow::Settings::API_OPENGL_EGL)) {
        glUnmapBufferOES(GL_PIXEL_PACK_BUFFER);
    } else {
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);
    m_mappedPointer = nullptr;
}


bool GLPixelTransferBuffer::readyToMap() const {
    return isNull(m_milestone) || m_milestone->completed();
}


void GLPixelTransferBuffer::setData(const void* data) { 
    debugAssertM(isNull(m_mappedPointer), "Illegal to invoke setData() while mapped");
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glBufferID());
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, size(), data);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, GL_NONE);
}


void GLPixelTransferBuffer::getData(void* data) const {
    debugAssertM(isNull(m_mappedPointer), "Illegal to invoke getData() while mapped");

    glBindBuffer(GL_PIXEL_PACK_BUFFER, glBufferID());
    glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, size(), data);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);
}


void GLPixelTransferBuffer::copy
   (const shared_ptr<GLPixelTransferBuffer>&   src, 
    const shared_ptr<GLPixelTransferBuffer>&   dst, 
    int                                        srcSizePixels, 
    int                                        srcStartPixel, 
    int                                        dstStartPixel) {

    debugAssert(notNull(src) && notNull(dst));
    alwaysAssertM(src->format() == dst->format(), "Different formats");

    if (srcSizePixels == -1) {
        srcSizePixels = src->pixelCount();
    }
    
    glBindBuffer(GL_COPY_READ_BUFFER, src->m_glBufferID);
    glBindBuffer(GL_COPY_WRITE_BUFFER, dst->m_glBufferID);
    const size_t bytesPerPixel = iCeil(src->format()->cpuBitsPerPixel / 8.0f);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcStartPixel * bytesPerPixel, dstStartPixel * bytesPerPixel, srcSizePixels * bytesPerPixel);
    glBindBuffer(GL_COPY_READ_BUFFER, GL_NONE);
    glBindBuffer(GL_COPY_WRITE_BUFFER, GL_NONE);
}


uint64 GLPixelTransferBuffer::getGPUAddress(GLenum access) const{

    debugAssertM(GLCaps::supports("GL_NV_shader_buffer_load") && GLCaps::supports("GL_NV_shader_buffer_store"), "GLPixelTransferBuffer::getGPUAddress requires a GPU with NVIDIA buffer extensions");

    if (!glIsNamedBufferResidentNV(glBufferID())) {
        glMakeNamedBufferResidentNV(glBufferID(), access);
    }

    uint64 gpuAddress = 0;
    glGetNamedBufferParameterui64vNV(glBufferID(), GL_BUFFER_GPU_ADDRESS_NV, &gpuAddress);

    return gpuAddress;
}

} // G3D
