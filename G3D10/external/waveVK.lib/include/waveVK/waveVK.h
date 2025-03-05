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

// Define if necessary to get the old non-conformant behavior
// of VS 2017 before release 15.8
#ifndef _DISABLE_EXTENDED_ALIGNED_STORAGE
#define _DISABLE_EXTENDED_ALIGNED_STORAGE
#endif

#include <G3D/G3D.h>

namespace wave {


    struct Vertex
    {
        alignas(16) Vector3     pos;
        alignas(16) Vector3     nrm;
        alignas(16) Vector4     tan;
        alignas(16) Vector2     texCoord = Vector2(0.5f, 0.5f);
        alignas(4) int          matID = 0;
    };

class VKBVH {
private:

    class _VKBVH*     _bvh;

public:

    VKBVH();

    virtual ~VKBVH();


	void* glReadyHandle(int index);
	void* glCompleteHandle(int index);
	void* glMemoryHandle(int index);

    int allocateVulkanInteropTexture(const int width, const int height, const int mipLevels, bool buffer, uint64& allocatedMemory);

    void doVulkanRendering(
        int width,
        int height,
        int rayOrigins,
        int rayDirections,
        int lambertian,
        int glossy,
        int emissive,
        int shadingNormal,
        int position);
    void updateCameraFrame(const Matrix4& m);
    void finalizeAccelerationStructure(const std::vector<unsigned int>& raygenBytes,
        const std::vector<unsigned int>& missBytes,
        const std::vector<unsigned int>& closesthitBytes,
        const std::vector<unsigned int>& anyhitBytes,
        const std::vector<unsigned int>& shadowMissBytes);

    /** \param position 3*numVertices floats in XYZ
    \param normal XYZ, unit
    \param texcoord UV, unit
    \param tangentAndFacing float4: XYZ = tangent-space X-axis, W = +/- 1
    \param alphaMask numTris OpenGL textures to alpha test against. Set to GL_NONE to make alpha test always pass on a surface
    \param index 3*numTris indices as a triangle list (values used to index into *float3* array, not float).
    \param materialHandle Each triangle's materialHandle.
    */
    int createGeometry
       (const std::vector<Vertex>&           vertices,
        int                                  numVertices,
        const std::vector<int>&              index,
        const int                            materialIndex,
        int                                  numIndices,
        const float                          geometryToWorldRowMajorMatrix[16],
        bool                                 twoSided,
        int                                  transformID);

    int createMaterial(
        const bool   hasAlpha,
        const int    textureNIndex,
        const float* scale_n,
        const float* bias_n,
        const int    texture0Index,
        const float* scale_0,
        const float* bias_0,
        const int    texture1Index,
        const float* scale_1,
        const float* bias_1,
        const int    texture2Index,
        const float* scale_2,
        const float* bias_2,
        const int    texture3Index,
        const float* scale_3,
        const float* bias_3,
        const float  materialConstant,
        uint8         flags);

    void setTransform(int geometryIndex, const float geometryToWorldRowMajorMatrix[16]);
};

} // opw
