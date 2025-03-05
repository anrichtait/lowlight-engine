/*
Copyright 2018, NVIDIA

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
#pragma once
// Do not include any OptiX, CUDA, or OpenGL headers in this file. That would create a dependency for
// library users on the OptiX SDK!

// Define if necessary to get the old non-conformant behavior
// of VS 2017 before release 15.8
#ifndef _DISABLE_EXTENDED_ALIGNED_STORAGE
#define _DISABLE_EXTENDED_ALIGNED_STORAGE
#endif

#ifndef WAVE_NO_LINK
#   ifdef _MSC_VER
#       ifdef _DEBUG
#          pragma comment(lib, "waved")
#       else
#          pragma comment(lib, "wave")
#       endif

#       pragma comment(lib, "optix.6.5.0.lib")
#   endif
#endif

// If OpenGL has already been included, don't redefine this type.
// Otherwise, provide the definition for the convenience of the caller.
#ifndef GL_ZERO
typedef int GLint;
typedef float GLfloat;
typedef unsigned int GLuint;
typedef unsigned char GLubyte;
#endif

namespace wave {

/** Index into an array of user data maintained by the BVH. Guaranteed to be a 32-bit integer.*/
typedef int MaterialIndex;

/** Index into an array of geometry maintained by the BVH. Guaranteed to be a 32-bit integer.*/
typedef int GeometryIndex;

/** Callback signature for use with rtContextSetUsageReportCallback.
    Copied from OptiX header.*/
using TimingCallback = void(int, const char*, const char*, void*);

class BVH {
private:

    class _BVH*     bvh;

public:

    static const float* identityMatrix();

    BVH();

    /** Returns false if initialization failed */
    bool valid() const;

    virtual ~BVH();


    /** Overwrites the current material at \a index with this, extending the material set as needed.
        The caller is responsible for managing active material indices. They are not allocated by BVH.
        The library is most efficient when materials indices are consecutive, with no gaps.

         You can use whatever material you wish. The textures will be interpreted as follows:

         - If alphaTest is enabled, it tests against material0Texture2D.a
         - The RGB channels of materialNormalBumpTexture2D are interpreted as tangent-space normals and converted 
           to world space shading normals when writing intersection properties. 

        All other textures are sampled and passed on without modification

        The supported material formats are those of CUDA:
        
        - {GL_R, GL_RG, GL_RGBA} x {8, 16, 16F, 32F, 8UI, 16UI, 32UI, 8I, 16I, 32I}
        - Supports sRGB unsigned byte data through rtTextureSamplerReadMode enums RT_TEXTURE_READ_ELEMENT_TYPE_SRGB and RT_TEXTURE_READ_NORMALIZED_FLOAT_SRGB.
 
    */
    MaterialIndex createMaterial
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
        const GLubyte   flags);

    void deleteMaterial(MaterialIndex index);

    /** Delete all materials and geometry */
    void reset();
    
    /** \param position 3*numVertices floats in XYZ
        \param normal XYZ, unit
        \param texcoord UV, unit
        \param tangentAndFacing float4: XYZ = tangent-space X-axis, W = +/- 1
        \param alphaMask numTris OpenGL textures to alpha test against. Set to GL_NONE to make alpha test always pass on a surface
        \param index 3*numTris indices as a triangle list (values used to index into *float3* array, not float).
        \param materialHandle Each triangle's materialHandle. 

    */
    GeometryIndex createGeometry
       (const float*        position, 
        const float*        normal,
        const float*        texCoord,
        const float*        tangentAndFacing,
        int                 numVertices,
        const int*          index,
        const MaterialIndex materialIndex,
        int                 numTris,
        const float         geometryToWorldRowMajorMatrix[16],
        bool                twoSided,
        int                 transformID,
		unsigned int		visibilityMask);

    void deleteGeometry(GeometryIndex index);

    /** Changes the rigid body transformation for a geometric object. */
    void setTransform
       (GeometryIndex   geometryIndex,
        const float     geometryToWorldRowMajorMatrix[16]);


    /** Set the callback function for printing timings in OptiX. 
        using TimingCallback = void(int, const char*, const char*, void*);
        
        verbosity of 0 disables reporting. callback is ignored in this case.
        
        verbosity of 1 enables error messages and important warnings. This 
        verbosity level can be expected to be efficient and have no significant overhead.

        verbosity of 2 additionally enables minor warnings, performance recommendations, 
        and scene statistics at startup or recompilation granularity. This level may 
        have a performance cost.

        verbosity of 3 additionally enables informational messages and per-launch 
        statistics and messages. */
    void setTimingCallback(TimingCallback* callback, int verbosityLevel);

    /**
      \param rayOriginTexture2D RGBA32F texture2D with RGB = world space XYZ origin, A = minimum hit distance
      \param rayDirectionTexture2D RGBA32F texture2D with RGB = world space XYZ direction (unit vector), A = maximum hit distance
      \param width, height Dimensions of the rayOriginTexture2D
      \param hitOutTexture2D An R8 texture that will be updated with zero for misses and nonzero for hits.

      If the ray hits, writes a nonzero value to hitOutTexture2D. If the ray misses, writes zero. All rays must be cast.
      Put a degenerate ray in the input textures for those you don't actually want.

      The rayOriginTexture2D and rayDirectionTexture2D must have the same dimensions.
     */ 
    bool occlusionCast
       (GLint           rayOriginTexture2D,
        GLint           rayDirectionTexture2D,
        int             width,
        int             height,
        GLint           hitOutTexture2D,
        bool            alphaTest                       = true,
        bool            partialCoverageThresholdZero    = false,
        bool            backfaceCull                    = true,
        unsigned int    visibilityMask                  = 0xFF) const;

    /**
    The output texture objects and formats are:
         0. RGBA32F  = material0Texture2D.rgba
         1. RGBA32F  = material1Texture2D.rgba
         2. RGBA32F  = material2Texture2D.rgba
         3. RGBA32F  = material3Texture2D.rgb, materialConstant
         4. RGBA32UI = (triIndex, baryU, baryV, backface)
         5. RGBA32F  = shading normal in RGB [on -1 to 1, no bias and scale]. A = frontface flag. Will always be true for two-sided
            surfaces. The normal is automatically flipped if we hit the backface of a twosided surface.
         6. RGBA32F  = world space hit position, A = hit distance (+infinity on a miss, negative if single-sided backface is hit)
         7. RGBA32F  = geometric normal in RGB [on -1 to 1, no bias or scale]. A = flags

         baryU, baryV are fixed point on a scale of [0, 2^32). To convert to float:
         `float u = float(baryU) / float(0xFFFFFFFFUL)`

        On a miss, float4(0.0f,0.0f,0.0f,1.0f) is written to all textures. Note that a miss is easily tested by checking if length(geometricNormal) == 0.0f.

     \param lod What MIP-map level (of detail) to fetch from. 0 = highest resolution. For indirect rays,
        it can be useful to force a lower detail level to improve cache coherence.

      Alpha test cuts off at 0.5f.
    */
     bool cast
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
        bool            alphaTest = true,
        bool            backfaceCull = true,
        int             materialLod = 0,
        GLint           rayConeAnglesPBO = 0, // GL_NONE, but we don't have that definition in the header.
		unsigned int	visibilityMask = 0xFF) const;


     void unregisterCachedBuffer(const GLint glBufID);
     void unmapCachedBuffer(const GLint glBufID);
};



} // opw
