/*
Copyright 2019, NVIDIA

2-Clause BSD License (https://opensource.org/licenses/BSD-2-Clause)

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define WAVE_NO_LINK

//CUDA include
//#include<cuda_runtime.h>

#include "../include/wave/wave.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <ctime>
#include <iostream>
#include <limits>
#include <cmath>
#include <algorithm>
#include <string>

#ifndef NOMINMAX
#   define NOMINMAX
#endif
#ifndef _USE_MATH_DEFINES
#   define _USE_MATH_DEFINES
#endif

#define _NVISA
#define NVVM_ADDRESS_SPACE_0_GENERIC
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#define INTERNAL_DEVELOPER
#define DEVELOP 1
#define _VARIADIC_MAX 10
#define CUDA_64_BIT_DEVICE_CODE


#ifndef BEGIN_PROFILER_EVENT
// These are present only for G3D debugging of wave, and are not defined normally
#define BEGIN_PROFILER_EVENT(ignore) 
#define END_PROFILER_EVENT() 
#endif

#if 1
#if defined(_WIN32)
/*
 * GLEW does not include <windows.h> to avoid name space pollution.
 * GL needs GLAPI and GLAPIENTRY, GLU needs APIENTRY, CALLBACK, and wchar_t
 * defined properly.
 */
/* <windef.h> and <gl.h>*/
#ifdef APIENTRY
#  ifndef GLAPIENTRY
#    define GLAPIENTRY APIENTRY
#  endif
#  ifndef GLEWAPIENTRY
#    define GLEWAPIENTRY APIENTRY
#  endif
#else
#define GLEW_APIENTRY_DEFINED
#  if defined(__MINGW32__) || defined(__CYGWIN__) || (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
#    define APIENTRY __stdcall
#    ifndef GLAPIENTRY
#      define GLAPIENTRY __stdcall
#    endif
#    ifndef GLEWAPIENTRY
#      define GLEWAPIENTRY __stdcall
#    endif
#  else
#    define APIENTRY
#  endif
#endif
#ifndef GLAPI
#  if defined(__MINGW32__) || defined(__CYGWIN__)
#    define GLAPI extern
#  endif
#endif
/* <winnt.h> */
#ifndef CALLBACK
#   define GLEW_CALLBACK_DEFINED
#   if defined(__MINGW32__) || defined(__CYGWIN__)
#    define CALLBACK __attribute__ ((__stdcall__))
#   elif (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#    define CALLBACK __stdcall
#   else
#    define CALLBACK
#   endif
#endif

/* <wingdi.h> and <winnt.h> */
#ifndef WINGDIAPI
#define GLEW_WINGDIAPI_DEFINED
#define WINGDIAPI __declspec(dllimport)
#endif
/* <ctype.h> */
#if (defined(_MSC_VER) || defined(__BORLANDC__)) && !defined(_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#  define _WCHAR_T_DEFINED
#endif
/* <stddef.h> */
#if !defined(_W64)
#  if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && defined(_MSC_VER) && _MSC_VER >= 1300
#    define _W64 __w64
#  else
#    define _W64
#  endif
#endif
#if !defined(_PTRDIFF_T_DEFINED) && !defined(_PTRDIFF_T_) && !defined(__MINGW64__)
#  ifdef _WIN64
typedef __int64 ptrdiff_t;
#  else
typedef _W64 int ptrdiff_t;
#  endif
#  define _PTRDIFF_T_DEFINED
#  define _PTRDIFF_T_
#endif

#ifndef GLAPI
#  if defined(__MINGW32__) || defined(__CYGWIN__)
#    define GLAPI extern
#  else
#    define GLAPI WINGDIAPI
#  endif
#endif

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef __int64 GLsizeiptr;

/*
GLAPI void __stdcall glBindTexture(GLenum target, GLuint texture);
GLAPI void __stdcall glGetIntegerv(GLenum pname, GLint *params);
GLAPI void __stdcall glPixelStorei(GLenum pname, GLint param);
GLAPI void __stdcall glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
GLAPI void __stdcall glBindBuffer(GLenum target, GLuint buffer);
GLAPI void __stdcall glGenBuffers(GLsizei n, GLuint* buffers);
GLAPI void __stdcall glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
WINGDIAPI GLenum APIENTRY glGetError(void);
*/

/*
 * GLEW_STATIC is defined for static library.
 * GLEW_BUILD  is defined for building the DLL library.
 */

#ifdef GLEW_STATIC
#  define GLEWAPI extern
#else
#  ifdef GLEW_BUILD
#    define GLEWAPI extern __declspec(dllexport)
#  else
#    define GLEWAPI extern __declspec(dllimport)
#  endif
#endif

#else /* _UNIX */

/*
 * Needed for ptrdiff_t in turn needed by VBO.  This is defined by ISO
 * C.  On my system, this amounts to _3 lines_ of included code, all of
 * them pretty much harmless.  If you know of a way of detecting 32 vs
 * 64 _targets_ at compile time you are free to replace this with
 * something that's portable.  For now, _this_ is the portable solution.
 * (mem, 2004-01-04)
 */

#include <stddef.h>

/* SGI MIPSPro doesn't like stdint.h in C++ mode          */
/* ID: 3376260 Solaris 9 has inttypes.h, but not stdint.h */

#if (defined(__sgi) || defined(__sun)) && !defined(__GNUC__)
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#define GLEW_APIENTRY_DEFINED
#define APIENTRY

/*
 * GLEW_STATIC is defined for static library.
 */

#ifdef GLEW_STATIC
#  define GLEWAPI extern
#else
#  if defined(__GNUC__) && __GNUC__>=4
#   define GLEWAPI extern __attribute__ ((visibility("default")))
#  elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#   define GLEWAPI extern __global
#  else
#   define GLEWAPI extern
#  endif
#endif

/* <glu.h> */
#ifndef GLAPI
#define GLAPI extern
#endif

#endif /* WIN32 or UNIX */

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifndef GLEWAPIENTRY
#define GLEWAPIENTRY
#endif

#define GLEW_VAR_EXPORT GLEWAPI
#define GLEW_FUN_EXPORT GLEWAPI

#define GL_NONE 0
#define GL_NO_ERROR 0
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_RGBA 0x1908
#define GL_RED 0x1903
#define GL_FLOAT 0x1406
#define GL_HALF_FLOAT 0x140B
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE_2D 0x0DE1
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_STREAM_COPY 0x88E2
#define GL_RGBA_INTEGER 0x8D99
#define GL_UNSIGNED_INT 0x1405
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#define GL_RGBA8 0x8058
#define GL_RGBA16F 0x881A
#define GL_RGBA32F 0x8814

/*

#define GLEW_GET_FUN(x) x
typedef void (GLAPIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
PFNGLDELETEBUFFERSPROC __glewDeleteBuffers = NULL;
#define glDeleteBuffers GLEW_GET_FUN(__glewDeleteBuffers)

GLAPI void GLAPIENTRY glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
GLAPI void GLAPIENTRY glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
*/
#endif

#ifndef NOMINMAX
#   define NOMINMAX
#endif
#include <cuda_gl_interop.h>

#include <optix_world.h>
#pragma warning(disable : 4996)

static inline void checkForOpenGLError() {
#   ifdef _DEBUG
        const GLenum e = glGetError();
        assert(e == GL_NO_ERROR);
#   endif
}

static inline void checkCudaErrors(cudaError_t status) {
    //assert(status == 0);
    if (status != 0)
    {
        printf("Cuda error: %d\n", status);
        exit(1);
    }
}

namespace wave {

class _BVH {
protected:
    static const int                m_castEntryPoint = 0;
    static const int                m_occlusionCastEntryPoint = 1;

    static const int                m_castRayType = 0;
    static const int                m_occlusionCastRayType = 1;
    
    mutable optix::Context          m_context;

    // To be really clever, we might use a templated alias declaration to derive a shared pointer
    // to a general optix obj, not just a buffer.
    using SharedPtrOptixBuffer = std::shared_ptr<optix::Buffer>;
    struct VertexBufferCacheElement {
        SharedPtrOptixBuffer position;
        SharedPtrOptixBuffer normal;
        SharedPtrOptixBuffer tangent;
        SharedPtrOptixBuffer texCoord;
    };
    using VertexBufferCache = std::unordered_map<size_t, std::weak_ptr<VertexBufferCacheElement>>;
    VertexBufferCache                   m_vertexBufferCache;
    // We need to maintain this for the vertex cache becuase elements of the Geometry Cache
    // can't verify that the thing they're deleting is stale.
    bool                                m_maybeVertexCacheStale = false;

    struct GeometryCacheElement {
        int                                             transformID;
        optix::GeometryInstance                         optixInstance;
        std::shared_ptr<VertexBufferCacheElement>       vertexData;
    };

    using GeometryCache = std::unordered_map<GeometryIndex, GeometryCacheElement>;
    GeometryCache                       m_geometryCache;

    using TransformCache = std::unordered_map<int, optix::Transform>;
    TransformCache                  m_transformCache;


    optix::Group                    m_root;

    optix::Program                  m_intersectProgram;
    optix::Program                  m_rayGeneratorProgram;
    optix::Program                  m_missProgram;
    optix::Program                  m_exceptionProgram;

    optix::Program                  m_occlusionClosestHitProgram;
    optix::Program                  m_occlusionAnyHitProgram;
    optix::Program                  m_occlusionCastProgram;

    optix::Program                  m_closestHitProgram;
    optix::Program                  m_anyHitProgram;
    optix::Program                  m_castProgram;

    using MaterialCache = std::unordered_map<MaterialIndex, optix::Material>;
    MaterialCache                   m_materialCache;

    static const int                NUM_OUT_BUFFERS = 9;

    struct GraphicsResource {
        struct cudaGraphicsResource* resource;
        void* ptr = nullptr;
    };

    using GraphicsResourceCache = std::unordered_map<GLint, GraphicsResource>;

    mutable GraphicsResourceCache m_glBufferCache;

    /** Load a small text file from disk into memory allocated with operator new. */
    static char* loadFile(const char* filename);

    static bool optixSupportsTexture(GLint texture2D);

    using TextureCache = std::unordered_map<GLint, optix::TextureSampler>;
    // TODO: clean this up.
    mutable TextureCache                  m_glTextureCache;
    optix::TextureSampler createOptiXTexture(GLint texture2D);

    void setRayBuffers(int width, int height, GLint rayOriginBuffer2D, GLint rayDirectionBuffer2D) const;

    void copyGLPixels(const GLint texture2D, optix::TextureSampler& sampler) const;

    GraphicsResource registerGLBuffer(GLint pbo, bool readOnly=false) const;
    void* getMappedPointer(GraphicsResource& g) const;

    /** Allocates the optix::Buffers the first frame. Thereafter, sets their
        sizes and pointers from the GL ID parameters. */
    void setOutputBuffers(
        int             width,
        int             height,
        GLint           material0OutPBO,
        GLint           material1OutPBO,
        GLint           material2OutPBO,
        GLint           material3OutPBO,
        GLint           hitLocationOutPBO,
        GLint           shadingNormalOutPBO,
        GLint           positionOutPBO,
        GLint           geometricNormalOutPBO, 
        GLint           hitPBO) const;

    void setInputBuffers(int width, int height, GLint rayConeAnglesPBO) const;

    void printCudaMemoryUsage(const std::string& message) const;

    // For testing wave.lib only.
    static void timingCallback(int verbosityLevel, const char* tag, const char* message, void* userData) {
        std::cout << "Verbosity: " + verbosityLevel << std::endl;
        std::cout << "Tag: " << tag << std::endl;
        std::cout << message << std::endl;
    }

public:


    _BVH();

    /** May throw exceptions */
    void init();

    ~_BVH();

    MaterialIndex createMaterial
       (bool            hasAlpha,
        const GLint     normalBumpTexture2D,
        const float*    scale_n,
        const float*    bias_n,
        const GLint     texture2D_0,
        const float*    scale_0,
        const float*    bias_0,
        const GLint     texture2D_1,
        const float*    scale_1,
        const float*    bias_1,
        const GLint     texture2D_2,
        const float*    scale_2,
        const float*    bias_2,
        const GLint     texture2D_3,
        const float*    scale_3,
        const float*    bias_3,
        const GLfloat   materialConstant,
        GLubyte         flags);

    void deleteMaterial(MaterialIndex index);

    void reset();

    GeometryIndex createGeometry
       (const float*              position, 
        const float*              normal,
        const float*              texCoord,
        const float*              tangentAndFacing,
        int                       numVertices,
        const int*                index,
        const MaterialIndex       materialIndex,
        int                       numTris,
        const float               matrix[16],
        bool                      twoSided,
        int                       transformID,
		unsigned int			  visibilityMask);

    void deleteGeometry(GeometryIndex handle);

    void setTransform
       (GeometryIndex   geometryHandle,
        const float     geometryToWorldRowMajorMatrix[16]);

    void setTimingCallback(TimingCallback* callback, int verbosityLevel);

    bool occlusionCast
       (GLint           rayOriginTexture2D,
        GLint           rayDirectionTexture2D,
        int             width,
        int             height,
        GLint           hitPBO,
        bool            alphaTest = true,
        bool            partialCoverageThresholdZero = false,
        bool            backfaceCull = true,
        unsigned int    visibilityMask = 0xFF) const;

     bool cast
       (GLint           rayOriginTexture2D,
        GLint           rayDirectionTexture2D,
        int             width,
        int             height,
        GLint           material0OutPBO,
        GLint           material1OutPBO,
        GLint           material2OutPBO,
        GLint           material3OutPBO,
        GLint           hitLocationOutPBO,
        GLint           shadingNormalOutPBO,
        GLint           positionOutPBO,
        GLint           geometricNormalOutPBO,
        bool            alphaTest = true,
        bool            backfaceCull = true,
        int             materialLod = 0,
        GLint           rayConeAnglesPBO = GL_NONE,
		unsigned int	visibilityMask = 0xFF) const;

     void updateTexture(GLuint texture, GLint pbo, int mipLevel, int width, int height, int baseFormat, int dataFormat) const;

     // Map all resources unmapped by a hook last frame.
     void mapUnmappedResources(const GLint* ids, const int num) const {
         for (int i = 0; i < num; ++i) {
             // Do not create the entry here!
             if (m_glBufferCache.find(ids[i]) != m_glBufferCache.end()) {
                 getMappedPointer(m_glBufferCache[ids[i]]);
             }
         }
     }

     void unmapCachedBuffer(const GLint glBufID) const;
     void unregisterCachedBuffer(const GLint glBufID) const;
};


_BVH::_BVH() {
}


void _BVH::init() {

    unsigned int numCudaDevices = 0;
    const RTresult result = rtDeviceGetDeviceCount(&numCudaDevices);

    if ((numCudaDevices == 0) || (result != RT_SUCCESS)) {
        throw "No CUDA devices";
    }

    try {
        m_context = optix::Context::create();
    } catch (...) {
        throw "Could not create OptiX context";
    }

    if (! m_context) {
        throw "No context";
    }

    {
        const std::vector<int>& enabled_devices = m_context->getEnabledDevices();
        if (enabled_devices.size() == 0) {
            throw "No OptiX-enabled devices";
        }
        // Set only a single device for the context
        m_context->setDevices(enabled_devices.begin(), ++enabled_devices.begin());
        int m_cuda_device_ordinal = -1;
        int m_optix_device_ordinal = enabled_devices[0];
        m_context->getDeviceAttribute(m_optix_device_ordinal, RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL, sizeof(int), &m_cuda_device_ordinal);
    }

    // Set the wrapper group for multiple transforms
    m_root = m_context->createGroup();
    m_root->setAcceleration(m_context->createAcceleration("Trbvh"));

    m_context["root"]->set(m_root);

    // normal cast + occlusion cast
    m_context->setRayTypeCount(2);
    m_context->setEntryPointCount(2);

    // Reasonable smallest working size parameter.
    // https://forums.developer.nvidia.com/t/how-to-understand-and-set-the-stack-size/67715
    m_context->setStackSize(1024);
    
    char* ptx = loadFile(
#       ifdef _DEBUG
            "wave-gpu-debug.ptx"
#       else 
            "wave-gpu.ptx"
#       endif
    );

    //Debugging, uncomment to use printf from inside wave-gpu.cu
    //m_context->setPrintEnabled(1);
    //m_context->setPrintBufferSize(4096);

    m_intersectProgram              = m_context->createProgramFromPTXString(ptx, "intersect");
    m_exceptionProgram              = m_context->createProgramFromPTXString(ptx, "exception");

    m_castProgram                   = m_context->createProgramFromPTXString(ptx, "cast");
    m_anyHitProgram                 = m_context->createProgramFromPTXString(ptx, "anyHit");
    m_closestHitProgram             = m_context->createProgramFromPTXString(ptx, "closestHit");
    m_missProgram                   = m_context->createProgramFromPTXString(ptx, "miss");

    m_occlusionCastProgram          = m_context->createProgramFromPTXString(ptx, "occlusionCast");
    m_occlusionAnyHitProgram        = m_anyHitProgram;
    m_occlusionClosestHitProgram    = m_context->createProgramFromPTXString(ptx, "occlusionClosestHit");
    delete[] ptx; ptx = nullptr;

    m_context->setRayGenerationProgram(m_castEntryPoint, m_castProgram);
    m_context->setRayGenerationProgram(m_occlusionCastEntryPoint, m_occlusionCastProgram);
    m_context->setMissProgram(m_castEntryPoint, m_missProgram);
}


_BVH::~_BVH() {
    if (m_context) {
        m_context->destroy();
    }
}

void _BVH::printCudaMemoryUsage(const std::string& message) const {
    size_t free, total;
    cudaMemGetInfo(&free, &total);

    std::string memoryUse = "\nFree: " + std::to_string(free) + ", Total: " + std::to_string(total) + "\n";
    printf((message + memoryUse).c_str(), free, total);
}

void _BVH::setInputBuffers(int width, int height, GLint rayConeAnglesPBO) const {
    static bool first = true;
    static optix::Buffer rayConeAnglesBuffer;
    if (first) {
        first = false;
        optix::Buffer& b = m_context->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_COPY_ON_DIRTY);
        b->setFormat(RT_FORMAT_FLOAT);
        rayConeAnglesBuffer = b;
    }

    if (rayConeAnglesPBO != GL_NONE) {
        GraphicsResource& g = m_glBufferCache[rayConeAnglesPBO];
        // As with setRayBuffers(), only null if newly created.
        if (g.ptr == nullptr) {
            g = registerGLBuffer(rayConeAnglesPBO, true);
        }
        rayConeAnglesBuffer->setDevicePointer(0, getMappedPointer(g));
    }
    m_context["rayConeAnglesBuffer"]->set(rayConeAnglesBuffer);

    rayConeAnglesBuffer->setSize(width, height);
}

void _BVH::setOutputBuffers(
    int             width,
    int             height,
    GLint           material0OutPBO,
    GLint           material1OutPBO,
    GLint           material2OutPBO,
    GLint           material3OutPBO,
    GLint           hitLocationOutPBO,
    GLint           shadingNormalOutPBO,
    GLint           positionOutPBO,
    GLint           geometricNormalOutPBO,
    GLint           hitPBO)const {
    static const char* bufName[9] = { 
        "material0OutBuffer", 
        "material1OutBuffer", 
        "material2OutBuffer",
        "material3OutBuffer", 
        "hitLocationOutBuffer",
        "shadingNormalOutBuffer",
        "positionOutBuffer",
        "geometricNormalOutBuffer",
        "hitOutBuffer" };

    static bool first = true;
    static optix::Buffer outBuffers[NUM_OUT_BUFFERS];


    GLint bufIDs[NUM_OUT_BUFFERS] = {
               material0OutPBO, // Lambertian
               material1OutPBO, // Glossy
               material2OutPBO, // Transmissive
               material3OutPBO, // Emissive
               hitLocationOutPBO,
               shadingNormalOutPBO,
               positionOutPBO,
               geometricNormalOutPBO,
               hitPBO
    };

    // On the first frame, create the output buffers.
    if (first) {
        first = false;
        for (int i = 0; i < NUM_OUT_BUFFERS; ++i) {
            optix::Buffer& b = m_context->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_COPY_ON_DIRTY);
            switch (i) {
            case 0:
            case 1:
            case 2:
                b->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
                break;
            case 4:
                b->setFormat(RT_FORMAT_UNSIGNED_INT4);
                break;
            case 8:
                b->setFormat(RT_FORMAT_UNSIGNED_BYTE);
                break;
            default:
                b->setFormat(RT_FORMAT_FLOAT4);
            }
            outBuffers[i] = b;
        }
    }
    
    for (int i = 0; i < NUM_OUT_BUFFERS; ++i) {
        if (bufIDs[i] != GL_NONE) {
            GraphicsResource& g = m_glBufferCache[bufIDs[i]];
            // As with setRayBuffers(), only null if newly created.
            if (g.ptr == nullptr) {
                g = registerGLBuffer(bufIDs[i]);
            }
            outBuffers[i]->setDevicePointer(0, getMappedPointer(g));
        }
        m_context[bufName[i]]->set(outBuffers[i]);

		outBuffers[i]->setSize(width, height);
    }
}


char* _BVH::loadFile(const char* filename) {
    // Abstracted to allow alternative loading schemes in the future, such as embedding in the resource
    // section of the executable itself.

    std::FILE* file = std::fopen(filename, "rb");
    if (! file) {
        // Could not load from CWD, check PATH
        char* PATH = getenv("PATH");
        const char* DELIMITERS = 
#       ifdef _MSC_VER
            ";";
#       else
            ":";
#       endif
        const char SEPARATOR =
#       ifdef _MSC_VER
            '\\';
#       else
            '/';
#       endif

        // For each directory in PATH
        for (char* dir = strtok(PATH, DELIMITERS); dir && ! file; dir = strtok(nullptr, DELIMITERS)) {
            const std::string& s = (std::string(dir) + SEPARATOR) + filename;
            file = std::fopen(s.c_str(), "rb");
        }

    }
    assert(file);
    std::fseek(file, 0, SEEK_END);
    const size_t size = std::ftell(file);
    char* buffer = new char[size + 1];
    std::rewind(file);
    std::fseek(file, 0, SEEK_SET);

    std::fread(buffer, 1, size, file);
    buffer[size] = '\0';
    std::fclose(file);
    file = nullptr;

    return buffer;
}


bool _BVH::optixSupportsTexture(GLint texture2D) {
    // http://docs.nvidia.com/gameworks/content/gameworkslibrary/optix/optixapireference/group___open_g_l.html
    // http://docs.nvidia.com/gameworks/content/gameworkslibrary/optix/optixapireference/optix__gl__interop_8h.html#a04e1ab43df38124e9902ea98238bb1b4

    // TODO
    //const ImageFormat* fmt = tex->format();
    //return (fmt->numComponents != 3) && (fmt->colorSpace == ImageFormat::ColorSpace::COLOR_SPACE_RGB);
    return true;
}


optix::TextureSampler _BVH::createOptiXTexture(GLint texture2D) {
    // For now these have to be strong pointers instead of a weak cache
    // because we don't yet have auto-destructors for optix samplers.

    optix::TextureSampler& optixTexture = m_glTextureCache[texture2D];

    if (!optixTexture) {
        // Created new RT texture
        try {

            // Old GL interop way
            // TODO: Map through an optional flag to avoid copy.
            optixTexture = m_context->createTextureSamplerFromGLImage(texture2D, RT_TARGET_GL_TEXTURE_2D);
            //optixTexture->unregisterGLTexture();
            //printf("MIP level count: %u\n", optixTexture->getBuffer()->getMipLevelCount());
            //const int mipLevels = 1 + (int)floor(log2f(float(fmax(glwidth, glheight))));
            //optixTexture->setMipLevelCount();
            //optixTexture = m_context->createTextureSampler();
            //copyGLPixels(texture2D, optixTexture);
        } catch (const optix::Exception& e) {
            throw e.getErrorString();
        }
    }

    return optixTexture;
}


MaterialIndex _BVH::createMaterial
    (const bool      hasAlpha,
     const GLint     texture2D_n,
     const float*    scale_n,
     const float*    bias_n,
     const GLint     texture2D_0,
     const float*    scale_0,
     const float*    bias_0,
     const GLint     texture2D_1,
     const float*    scale_1,
     const float*    bias_1,
     const GLint     texture2D_2,
     const float*    scale_2,
     const float*    bias_2,
     const GLint     texture2D_3,
     const float*    scale_3,
     const float*    bias_3,
     const GLfloat   materialConstant,
     GLubyte         flags) {

    static MaterialIndex firstOpenIndex = -1;
    ++firstOpenIndex;

	// Creates the element.
    optix::Material& mat = m_materialCache[firstOpenIndex];
    mat = m_context->createMaterial();

    mat->setClosestHitProgram(m_castRayType, m_closestHitProgram);
    mat->setClosestHitProgram(m_occlusionCastRayType, m_occlusionClosestHitProgram);

    // Only bind anyHit programs if we need alpha testing
    // Transmissive can then be handled by closest hit shader as normal.
    if (hasAlpha) {
        mat->setAnyHitProgram(m_castRayType, m_anyHitProgram);
    }

    mat->setAnyHitProgram(m_occlusionCastRayType, m_occlusionAnyHitProgram);

    mat["texNSampler"]->setInt(createOptiXTexture(texture2D_n)->getId());
    mat["tex0Sampler"]->setInt(createOptiXTexture(texture2D_0)->getId());
    mat["tex1Sampler"]->setInt(createOptiXTexture(texture2D_1)->getId());
    mat["tex2Sampler"]->setInt(createOptiXTexture(texture2D_2)->getId());
    mat["tex3Sampler"]->setInt(createOptiXTexture(texture2D_3)->getId());

    mat["texNScale"]->set4fv(scale_n);
    mat["texNBias"]->set4fv(bias_n);

    mat["tex0Scale"]->set4fv(scale_0);
    mat["tex0Bias"]->set4fv(bias_0);

    mat["tex1Scale"]->set4fv(scale_1);
    mat["tex1Bias"]->set4fv(bias_1);

    mat["tex2Scale"]->set4fv(scale_2);
    mat["tex2Bias"]->set4fv(bias_2);

    mat["tex3Scale"]->set4fv(scale_3);
    mat["tex3Bias"]->set4fv(bias_3);

    mat["flags"]->setInt(flags);
    mat["constant"]->setFloat(materialConstant);

    return firstOpenIndex;
}


void _BVH::deleteMaterial(MaterialIndex index) {
    m_materialCache[index]->destroy();
    m_materialCache.erase(index);

}


// TODO: This method clears the data structures, but does not leave the tree
// in a clear state. We don't call it now, but some day...
void _BVH::reset() {
    m_materialCache.clear();
    m_context["root"] = nullptr;
    m_geometryCache.clear();
}


void _BVH::deleteGeometry(GeometryIndex handle) {
    // If this handle is not cached, fail immediately.
    assert(m_geometryCache.find(handle) != m_geometryCache.end());

    GeometryCacheElement& instance = m_geometryCache[handle];
    optix::Transform& t = m_transformCache[instance.transformID];

    if (!t) {
        return;
    }

    const int groupChildCount = m_root->getChildCount();

    //Ensure that the transform has a child to delete
    optix::GeometryGroup& group = t->getChild<optix::GeometryGroup>();
    if (group) {

        group->removeChild(instance.optixInstance);
        instance.optixInstance->getGeometryTriangles()->destroy();
        instance.optixInstance->destroy();

        //Need to mark this dirty, or the old geometry is not fully deleted!
        group->getAcceleration()->markDirty();

        if (group->getChildCount() == 0) {

            group->destroy();
            m_root->removeChild(t);
            t->destroy();

            m_transformCache.erase(instance.transformID);
        }
    }

    m_geometryCache.erase(handle);
    m_maybeVertexCacheStale = true;

    m_root->getAcceleration()->markDirty();
}


void _BVH::setTransform
   (GeometryIndex   geometryHandle,
    const float     geometryToWorldRowMajorMatrix[16]) {
    //optix::Transform t = m_root->getChild<optix::Transform>(m_geometryInstanceCache[geometryHandle].transformIdx);
    m_transformCache[m_geometryCache[geometryHandle].transformID]->setMatrix(false, geometryToWorldRowMajorMatrix, nullptr);
    m_root->getAcceleration()->markDirty();
}

void _BVH::setTimingCallback(TimingCallback* callback, int verbosityLevel) {
    m_context->setUsageReportCallback(callback, verbosityLevel, nullptr);
}


size_t pointerHash(size_t ptr) {
    return ptr ^ (ptr >> 32);
}
size_t vertexDataHash(size_t positionPtr, size_t normalPtr, size_t tangentPtr, size_t texCoordPtr) {
    return pointerHash(positionPtr) ^ pointerHash(normalPtr) ^ pointerHash(tangentPtr) ^ pointerHash(texCoordPtr);
}

GeometryIndex _BVH::createGeometry
   (const float*            position, 
    const float*            normal,
    const float*            texcoord,
    const float*            tangentAndFacing,
    int                     numVertices,
    const int*              index,
    const MaterialIndex     materialIndex,
    int                     numTris,
    const float             matrix[16],
    bool                    twoSided,
    int                     transformID,
	unsigned int			visibilityMask) {

    static GeometryIndex firstOpenIndex = -1;
    ++firstOpenIndex;

    // Fail immediately if this index is already in the cache.
    assert(m_geometryCache.find(firstOpenIndex) == m_geometryCache.end());

    GeometryCacheElement& instance = m_geometryCache[firstOpenIndex];

    // Indices. For now, create these buffers every time.
    optix::Buffer indexBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, numTris);
    memcpy((optix::int3*)indexBuffer->map(), index, 3 * sizeof(int) * numTris);
    indexBuffer->unmap();

    size_t vertexDataKey = vertexDataHash((size_t)position, (size_t)normal, (size_t)tangentAndFacing, (size_t)texcoord);
    // Capture weak pointer by reference so we can assign it later.
    std::weak_ptr<VertexBufferCacheElement>& v = m_vertexBufferCache[vertexDataKey];

    // This is the shared_pointer through which we will access the current vertex data
    // whether it was just constructed or cached.
    std::shared_ptr<VertexBufferCacheElement> currentCacheElement;
 
    if (v.expired()) { // Our weak pointer was just created.
        // Custom deleter. optix::Buffer uses "shallow" reference counting:
        // it only deletes the C++ object (optix::BufferObj). The custom deleter
        // ensures that the buffer is also removed from the context.
        auto optixBufferDeleter = [](optix::Buffer* bufferPtr) {
            (*bufferPtr)->destroy();
            delete bufferPtr;
        };

        // The weak pointer was just constructed, so create the vertex buffers.
        VertexBufferCacheElement* e = new VertexBufferCacheElement;
        ///// Begin Create Vertex Buffer Data in OptiX

        // With our current scheme, we are mapping optix::Buffer 1:1 optix::BufferObj
        // The optix::BufferObj will have a referenc count of 1 for its whole life
        // until the final shared pointer is destroyed.
        optix::Buffer* positionBuffer = new optix::Buffer;
        *positionBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numVertices);
        memcpy((optix::float3*)(*positionBuffer)->map(), position, sizeof(float) * 3 * numVertices);
        (*positionBuffer)->unmap();
        //Copy construction, can't call make_shared to make the pointer in place.
        e->position = SharedPtrOptixBuffer(positionBuffer, optixBufferDeleter);

        optix::Buffer* vertexBuffer = new optix::Buffer;
        *vertexBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numVertices);
        memcpy((optix::float3*)(*vertexBuffer)->map(), normal, sizeof(float) * 3 * numVertices);
        (*vertexBuffer)->unmap();
        e->normal = SharedPtrOptixBuffer(vertexBuffer, optixBufferDeleter);


        optix::Buffer* texCoordBuffer = new optix::Buffer;
        *texCoordBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, numVertices);
        memcpy((optix::float2*)(*texCoordBuffer)->map(), texcoord, sizeof(float) * 2 * numVertices);
        (*texCoordBuffer)->unmap();
        e->texCoord = SharedPtrOptixBuffer(texCoordBuffer, optixBufferDeleter);

        optix::Buffer* tangentBuffer = new optix::Buffer;
        (*tangentBuffer) = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, numVertices);
        memcpy((optix::float4*)(*tangentBuffer)->map(), tangentAndFacing, sizeof(float) * 4 * numVertices);
        (*tangentBuffer)->unmap();
        e->tangent = SharedPtrOptixBuffer(tangentBuffer, optixBufferDeleter);

        //// End Create Vertex Buffer Data In OptiX.

        // This is a pointer to a cache element, not an optix buffer, so we dont need any custom deleter
        // Some redundancy here...
        currentCacheElement = std::make_shared<VertexBufferCacheElement>(*e);
        instance.vertexData = currentCacheElement;
        v = currentCacheElement;
    }
    else {
        // Guaranteed not to be null, becuase we just set it above.
        currentCacheElement = v.lock();
    }

    // Triangle API
    optix::GeometryTriangles geom_tri = m_context->createGeometryTriangles();
    geom_tri->setPrimitiveCount(numTris);
    geom_tri->setTriangleIndices(indexBuffer, RT_FORMAT_UNSIGNED_INT3);
    geom_tri->setVertices((unsigned)numVertices, *(currentCacheElement->position), RT_FORMAT_FLOAT3);
    geom_tri->setBuildFlags(RTgeometrybuildflags(0));
    geom_tri->setAttributeProgram(m_intersectProgram);
    geom_tri->setMaterialCount(1);

    optix::GeometryInstance geometry_instance = m_context->createGeometryInstance();
    geometry_instance->setGeometryTriangles(geom_tri);

    // Set the material for this surface
    geometry_instance->setMaterialCount(1);
    geometry_instance->setMaterial(0, m_materialCache[materialIndex]);

    geometry_instance["twoSided"]->setInt(twoSided);
    geometry_instance["geometryIndex"]->setBuffer(indexBuffer);
    //geometry_instance["geometryMaterialIndex"]->setBuffer(materialIndexBuffer);

    // Vertex Data, set from the current cache element.
    geometry_instance["vertexPosition"]->setBuffer(*(currentCacheElement->position));
    geometry_instance["vertexNormal"]->setBuffer(*(currentCacheElement->normal));
    geometry_instance["vertexTexcoord"]->setBuffer(*(currentCacheElement->texCoord));
    geometry_instance["vertexTangent"]->setBuffer(*(currentCacheElement->tangent));

    optix::Transform& t = m_transformCache[transformID];
    
    // TODO: this may not correctly detect when the transform is nulled out in deleteGeometry.
    // If it doesn't, need to explicitly delete this object from the transform cache.
    if (!t) {
        optix::GeometryGroup g = m_context->createGeometryGroup();
        g->setAcceleration(m_context->createAcceleration("Trbvh"));

        t = m_context->createTransform();
        t->setMatrix(false, matrix, nullptr);
        t->setChild(g);

        m_root->addChild(t);
    }

    optix::GeometryGroup& g = t->getChild<optix::GeometryGroup>();
    g->addChild(geometry_instance);

	// Only last 8 bits are used.
	g->setVisibilityMask(visibilityMask);

    g->getAcceleration()->markDirty();

    m_root->getAcceleration()->markDirty();

    instance.transformID = transformID;
    instance.optixInstance = geometry_instance;

    return firstOpenIndex;
}

void _BVH::copyGLPixels(const GLint texture2D, optix::TextureSampler& sampler) const {

    // We assume that the texture sampler has already been created.
    assert(sampler);
    // Ensure that all textures are in memory and can be copied to to OPtix.
    glBindTexture(GL_TEXTURE_2D, texture2D);

    GLint glwidth, glheight, glformat;
    //Grab the parameters of this texture for optix.
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &glwidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &glheight);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &glformat);
    // Assume float4 as default
    int bytesPerPixel = 16;
    RTformat optixFormat = RT_FORMAT_FLOAT4;
    GLint readDataType = GL_FLOAT;
    switch (glformat) {
    case GL_RGBA8: 
        bytesPerPixel = 4;
        optixFormat = RT_FORMAT_BYTE4;
        readDataType = GL_BYTE;
        break;
    case GL_RGBA16F:
        bytesPerPixel = 8;
        optixFormat = RT_FORMAT_HALF4;
        readDataType = GL_HALF_FLOAT;
    case GL_RGBA32F: break;
    default:
        // Hack to suppress warning about too many arguments to "assert".
        assert(false && "Unrecognized texture format");
    }

    const int mipLevels = 1 + (int)floor(log2f(float(fmax(glwidth, glheight))));


    //optix::Buffer glPixels = m_context->create2DLayeredBuffer(RT_BUFFER_INPUT, optixFormat, glwidth, glheight, 1, mipLevels);// optixFormat, glwidth, glheight);
    optix::Buffer glPixels = m_context->createMipmappedBuffer(RT_BUFFER_INPUT, optixFormat, glwidth, glheight, mipLevels);// optixFormat, glwidth, glheight);

    for (int i = 0; i < mipLevels; ++i) {
        void* optixData = glPixels->map(i, RT_BUFFER_MAP_WRITE);
        glGetTexImage(GL_TEXTURE_2D, i, GL_RGBA, readDataType, optixData);
        glPixels->unmap(i);
    }
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    sampler->setBuffer(glPixels);
    //sampler = m_context->createTextureSampler();
    // Copied from optix sample
    //sampler->setWrapMode(0, RT_WRAP_REPEAT);
    //sampler->setWrapMode(1, RT_WRAP_REPEAT); // don't sample beyond the poles of the environment sphere
    //sampler->setWrapMode(2, RT_WRAP_REPEAT);
    //sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
    //sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
    //sampler->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);
    //sampler->setMaxAnisotropy(1.0f);
    //sampler->setArraySize(1);
    //sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
    //sampler->setMaxAnisotropy(1.0f);
    //sampler->setBuffer(glPixels);
}

_BVH::GraphicsResource _BVH::registerGLBuffer(GLint pbo, bool readOnly) const {

    GraphicsResource g;
    checkCudaErrors(cudaGraphicsGLRegisterBuffer(&g.resource, pbo, readOnly ? cudaGraphicsMapFlagsReadOnly : cudaGraphicsMapFlagsWriteDiscard));
    getMappedPointer(g);
    return g;
}

void* _BVH::getMappedPointer(GraphicsResource& g) const {

    if (g.ptr == nullptr) {
        size_t bytes;
        void* bufferData;
        checkCudaErrors(cudaGraphicsMapResources(1, &g.resource));
        checkCudaErrors(cudaGraphicsResourceGetMappedPointer(&bufferData, &bytes, g.resource));

        g.ptr = bufferData;
    }

    return g.ptr;
}
 
void _BVH::setRayBuffers(int width, int height, GLint rayOriginBuffer2D, GLint rayDirectionBuffer2D) const {

    // I suspect the correct way to do this what we did for textures: cache and map every 
    // new thing you see, and then register an uncache hook that runs when the object is destroyed in the main
    // program. Programmer should assume that those buffers are always mapped in optix unless
    // they are explicitly destroyed.

    // These buffers only need to be created once with set formats.
    //static optix::Buffer originBuffer = m_context->createBufferFromGLBO(RT_BUFFER_INPUT, rayOriginBuffer2D);
    static optix::Buffer originBuffer = m_context->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_COPY_ON_DIRTY, RT_FORMAT_FLOAT4);
    //static optix::Buffer directionBuffer = m_context->createBufferFromGLBO(RT_BUFFER_INPUT, rayDirectionBuffer2D);
    static optix::Buffer directionBuffer = m_context->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_COPY_ON_DIRTY, RT_FORMAT_FLOAT4);
    
    static const bool READ_ONLY = true;

    GraphicsResource& originResource = m_glBufferCache[rayOriginBuffer2D];
    GraphicsResource& directionResource = m_glBufferCache[rayDirectionBuffer2D];
    try {
        // This will only be null if it hasn't been created because
        // mapUnmappedResources() is called before this always.
        if (originResource.ptr == nullptr) {
            // Register and map the buffer
            m_glBufferCache[rayOriginBuffer2D] = registerGLBuffer(rayOriginBuffer2D, READ_ONLY);
            m_context["rayOriginBuffer"]->set(originBuffer);
        }
        if (directionResource.ptr == nullptr) {
            // Register and map the buffer
            m_glBufferCache[rayDirectionBuffer2D] = registerGLBuffer(rayDirectionBuffer2D, READ_ONLY);
            m_context["rayDirectionBuffer"]->set(directionBuffer);
        }
    }
    catch (const optix::Exception& e) {
        (void)e;
        assert(false);// , e.getErrorString().c_str());
    }

    originBuffer->setDevicePointer(0, getMappedPointer(m_glBufferCache[rayOriginBuffer2D]));
    directionBuffer->setDevicePointer(0, getMappedPointer(m_glBufferCache[rayDirectionBuffer2D]));

	// Always need to resize buffers to match launch.
	// Because the buffer has a device pointer to a mapped
	// buffer, this call just sets metadata and doesn't
	// trigger a resize.
	originBuffer->setSize(width, height);
	directionBuffer->setSize(width, height);
}


bool _BVH::occlusionCast
   (GLint           rayOriginTexture2D,
    GLint           rayDirectionTexture2D,
    int             width,
    int             height,
    GLint           hitOutTexture2D,
    bool            alphaTest,
    bool            partialCoverageThresholdZero,
    bool            backfaceCull,
    unsigned int    visibilityMask) const {

    GLint ids[3] = { rayOriginTexture2D, rayDirectionTexture2D, hitOutTexture2D };
    mapUnmappedResources(ids, 3);

    setOutputBuffers(
        width,
        height,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        hitOutTexture2D);
    setRayBuffers(width, height, rayOriginTexture2D, rayDirectionTexture2D);

    m_context["backfaceTest"]->setInt(backfaceCull);
    m_context["alphaTest"]->setInt(alphaTest);    
    m_context["lod"]->setInt(0);
    m_context["partialCoverageThreshold"]->setFloat(partialCoverageThresholdZero ? 0.0f : 0.5f);

    m_context["visibilityMask"]->setUint(visibilityMask);


    try {
        m_context->launch(m_occlusionCastEntryPoint, width, height);
        return true;
    }
    catch (const optix::Exception & e) {
        // Do not throw an exception here.
        std::cerr << e.getErrorString() << "\n";
        return false;
    }
}


bool _BVH::cast
   (GLint           rayOriginTexture2D,
    GLint           rayDirectionTexture2D,
    int             width,
    int             height,
    GLint           material0OutTexture2D,
    GLint           material1OutTexture2D,
    GLint           material2OutTexture2D,
    GLint           material3OutTexture2D,
    GLint           hitLocationOutTexture2D,
    GLint           shadingNormalOutTexture2D,
    GLint           positionOutTexture2D,
    GLint           geometricNormalOutTexture2D,
    bool            alphaTest,
    bool            backfaceCull,
    int             materialLod,
    GLint           rayConeAnglesPBO,
	unsigned int	visibilityMask) const {
		BEGIN_PROFILER_EVENT("Pre-Launch");
		GLint ids[10] = {
			rayOriginTexture2D,
			rayDirectionTexture2D,
			material0OutTexture2D,
			material1OutTexture2D,
			material2OutTexture2D,
			material3OutTexture2D,
			hitLocationOutTexture2D,
			shadingNormalOutTexture2D,
			positionOutTexture2D,
			geometricNormalOutTexture2D
		};
		mapUnmappedResources(ids, 10);


		setOutputBuffers(
			width,
			height,
			material0OutTexture2D,
			material1OutTexture2D,
			material2OutTexture2D,
			material3OutTexture2D,
			hitLocationOutTexture2D,
			shadingNormalOutTexture2D,
			positionOutTexture2D,
			geometricNormalOutTexture2D,
			geometricNormalOutTexture2D // Ignored.
		);

        // TODO: find correct abstraction for setting this buffer.
        setInputBuffers(width, height, rayConeAnglesPBO);

		m_context["backfaceCull"]->setInt(backfaceCull);
		m_context["alphaTest"]->setInt(alphaTest);

		m_context["lod"]->setInt(materialLod);

		m_context["visibilityMask"]->setUint(visibilityMask);

		m_context["partialCoverageThreshold"]->setFloat(0.5f);
		setRayBuffers(width, height, rayOriginTexture2D, rayDirectionTexture2D);

		checkForOpenGLError();
		END_PROFILER_EVENT();

        try {
            m_context->launch(m_castEntryPoint, width, height);
            return true;
        }
        catch (const optix::Exception & e) {
            // Do not throw an exception here.
            std::cerr << e.getErrorString() << "\n";
            return false;
        }
}

void _BVH::unmapCachedBuffer(const GLint glBufID) const {
    if (m_glBufferCache.find(glBufID) == m_glBufferCache.end()) {
        return;
    }
    GraphicsResource& g = m_glBufferCache[glBufID];
    if (g.ptr != nullptr) {
        checkCudaErrors(cudaGraphicsUnmapResources(1, &g.resource));
    }
    g.ptr = nullptr;
}

void _BVH::unregisterCachedBuffer(const GLint glBufID) const {
    if (m_glBufferCache.find(glBufID) == m_glBufferCache.end()) {
        return;
    }
    GraphicsResource& g = m_glBufferCache[glBufID];

    // This resource may still be mapped
    unmapCachedBuffer(glBufID);
    // Don't check this call, as it emits a spurious error
    // if called after the GL context is destroyed.
    cudaGraphicsUnregisterResource(g.resource);

    m_glBufferCache.erase(glBufID);
}

//////////////////////////////////////////////////////////////////////////////////
// Trampoline BVH to the private _BVH API

BVH::BVH() : bvh(new _BVH()) {
    try {
        try {
            bvh->init();
        } catch (const optix::Exception& e) {
            throw e.getErrorString();
        }
    } catch (const std::string& s) {
        std::cerr << s << "\n";
        delete bvh;
        bvh = nullptr;
    } catch (const char* s) {
        std::cerr << s << "\n";
        delete bvh;
        bvh = nullptr;
    } catch (...) {
        delete bvh;
        bvh = nullptr;
    }
}


bool BVH::valid() const {
    return bvh != nullptr;
}


BVH::~BVH() {
    delete bvh;
}


GeometryIndex BVH::createGeometry
   (const float*              position, 
    const float*              normal,
    const float*              texCoord,
    const float*              tangentAndFacing,
    int                       numVertices,
    const int*                index,
    const MaterialIndex       materialIndex,
    int                       numTris,
    const float               matrix[16],
    bool                      twoSided,
    int                       transformID,
	unsigned int			  visibilityMask) {
    return bvh->createGeometry
       (position, 
        normal,
        texCoord,
        tangentAndFacing,
        numVertices,
        index,
        materialIndex,
        numTris,
        matrix,
        twoSided, 
        transformID,
		visibilityMask);    
}


void BVH::deleteGeometry(GeometryIndex index) {
    bvh->deleteGeometry(index);
}


void BVH::setTransform
   (GeometryIndex   geometryIndex,
    const float     geometryToWorldRowMajorMatrix[16]) {
    bvh->setTransform(geometryIndex, geometryToWorldRowMajorMatrix);
}

void BVH::setTimingCallback(TimingCallback* callback, int verbosityLevel) {
    bvh->setTimingCallback(callback, verbosityLevel);
}


bool BVH::occlusionCast
   (GLint           rayOriginTexture2D,
    GLint           rayDirectionTexture2D,
    int             width,
    int             height,
    GLint           hitFramebufferObject,
    bool            alphaTest,
    bool            partialConverageThresholdZero,
    bool            backfaceCull,
    unsigned int    visibilityMask) const {

    return bvh->occlusionCast
      (rayOriginTexture2D,
       rayDirectionTexture2D,
       width,
       height,
       hitFramebufferObject,
       alphaTest,
       partialConverageThresholdZero,
       backfaceCull,
        visibilityMask);
}


bool BVH::cast
   (GLint           rayOriginTexture2D,
    GLint           rayDirectionTexture2D,
    int             width,
    int             height,
    GLint           material0OutPBO,
    GLint           material1OutPBO,
    GLint           material2OutPBO,
    GLint           material3OutPBO,
    GLint           hitLocationOutPBO,
    GLint           shadingNormalOutPBO,
    GLint           positionOutPBO,
    GLint           geometricNormalOutPBO,
    bool            alphaTest,
    bool            backfaceCull,
    int             materialLod,
    GLint           rayConeAnglesPBO,
	unsigned int	visibilityMask) const {

    return bvh->cast
       (rayOriginTexture2D,
        rayDirectionTexture2D,
        width,
        height,
        material0OutPBO,
        material1OutPBO,
        material2OutPBO,
        material3OutPBO,
        hitLocationOutPBO,
        shadingNormalOutPBO,
        positionOutPBO,
        geometricNormalOutPBO,
        alphaTest,
        backfaceCull,
        materialLod,
        rayConeAnglesPBO,
		visibilityMask);
}



MaterialIndex BVH::createMaterial
   (const bool      hasAlpha,
    const GLint     normalBumpTexture2D,
    const float*    scale_nb,
    const float*    bias_nb,
    const GLint     texture2D_0,
    const float*    scale_0,
    const float*    bias_0,
    const GLint     texture2D_1,
    const float*    scale_1,
    const float*    bias_1,
    const GLint     texture2D_2,
    const float*    scale_2,
    const float*    bias_2,
    const GLint     texture2D_3,
    const float*    scale_3,
    const float*    bias_3,
    const GLfloat   materialConstant,
    GLubyte         flags) {

    return bvh->createMaterial
       (hasAlpha,
        normalBumpTexture2D,
        scale_nb,
        bias_nb,
        texture2D_0,
        scale_0,
        bias_0,
        texture2D_1,
        scale_1,
        bias_1,
        texture2D_2,
        scale_2,
        bias_2,
        texture2D_3,
        scale_3,
        bias_3,
        materialConstant,
        flags);
}


void BVH::deleteMaterial(MaterialIndex index) {
    bvh->deleteMaterial(index);
}


void BVH::reset() {
    bvh->reset();
}


const float* BVH::identityMatrix() {
    static const float I[16] = {1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1};
    return I;
}

void BVH::unmapCachedBuffer(const GLint glBufID) {
    bvh->unmapCachedBuffer(glBufID);
}

void BVH::unregisterCachedBuffer(const GLint glBufID) {
    bvh->unregisterCachedBuffer(glBufID);
}

} // namespace

