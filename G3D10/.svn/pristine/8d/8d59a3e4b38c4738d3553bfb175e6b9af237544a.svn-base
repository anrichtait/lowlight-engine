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
#define NOMINMAX
#define _USE_MATH_DEFINES
#define _NVISA
#define NVVM_ADDRESS_SPACE_0_GENERIC
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#define INTERNAL_DEVELOPER
#define DEVELOP 1
#define _VARIADIC_MAX 10
#define CUDA_64_BIT_DEVICE_CODE

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
//#include <cuda_fp16.h>

#define SAMPLE(id, x, y, layer, lod) rtTex2DLod<float4>(id, x, y, lod)
// OptiX programming model in brief:
//
// The INTERSECT callback is determined by the geometry (we use one: triangle). It only
// has access to variables bound on the geometry by the host, not materials. So it can't
// perform the alpha test directly.
// 
// The CLOSEST-HIT and ANY-HIT callbacks are determined by the material returned from the
// intersect callback and the ray type specified in the generator. They have access to
// the material variables. We use any hit to perform alpha test. For occlusion rays, the closest
// hit does nothing. For regular rays, closest hit performs full material sampling.
//
// For our application, all rays in a wavefront are either occlusion or regular cast; there is
// no mixing of ray types within a call.

using namespace optix;

/** float4 -> float3 */
inline __device__ float3 xyz(float4 v) { return make_float3(v.x, v.y, v.z); }

rtBuffer<float4, 2>         rayOriginBuffer;
rtBuffer<float4, 2>         rayDirectionBuffer;

rtDeclareVariable(rtObject,     root, , );
rtDeclareVariable(uint2,        launch_index, rtLaunchIndex, );
// rtDeclareVariable(uint2,        launch_dim,   rtLaunchDim, );

// OptiX doesn't support bool, so we use ints here
rtDeclareVariable(int,          backfaceCull, , );
rtDeclareVariable(int,          alphaTest, , );
rtDeclareVariable(int,          lod, , );
rtDeclareVariable(unsigned int, visibilityMask, , );
rtDeclareVariable(float,        partialCoverageThreshold, , );

// For occlusion cast
rtBuffer<unsigned char, 2>      hitOutBuffer;  

// For regular cast
rtBuffer<uchar4, 2>             material0OutBuffer;  
rtBuffer<uchar4, 2>             material1OutBuffer;  
rtBuffer<uchar4, 2>             material2OutBuffer;  
rtBuffer<float4, 2>             material3OutBuffer;  

rtBuffer<uint4, 2>              hitLocationOutBuffer;
rtBuffer<float4, 2>             shadingNormalOutBuffer;
rtBuffer<float4, 2>             positionOutBuffer;


rtBuffer<float, 2>              rayConeAnglesBuffer;

// Information passed between the callbacks
struct Hit {
    // The following are computed in intersect:
    //float           t;
    int              primitiveIndex;
    //int             materialIndex;
    float2           barycentrics;

    //
    //unsigned int    backface;

     //These are non-unit until the closest hit:
    //float3          geometricNormal;
    //float3          interpolatedNormal;
    //float3          interpolatedTangent;
    //
    //// The following are computed in closest-hit
    //// for regular casts:
    //
    ///** World-space shading normal, bump value */
    //float4          texN;
    //float4          tex0;
    //float4          tex1;
    //float4          tex2;
    //float4          tex3;
    //float           constant;
    //unsigned int    flags;
};


// Geometry
rtBuffer<float3> vertexPosition;
rtBuffer<float2> vertexTexcoord;
rtBuffer<float3> vertexNormal;
rtBuffer<float4> vertexTangent;
rtBuffer<int3>   geometryIndex;

rtDeclareVariable(int, twoSided, , );


// Variables passed between functions. Not visible on the host
rtDeclareVariable(Hit, hit_attr, attribute hit_attr, ); 
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );


// Triangle intersection with optional backface culling.
// Intersect runs at a time when we do not yet know if this is going to
// be the first hit (or any hit at all!).
// In the triangle API, this is an *attributes* program, not an intersection program.
RT_PROGRAM void intersect() {
    hit_attr.primitiveIndex = rtGetPrimitiveIndex();
    hit_attr.barycentrics = rtGetTriangleBarycentrics();
}


// Materials
//
// These are material properties that are automatically bound by OptiX
// when the material's corresponding any_hit/closest_hit shader is invoked

//rtTextureSampler<float4, 2> texNSampler;
//rtTextureSampler<float4, 2> tex0Sampler;
//rtTextureSampler<float4, 2> tex1Sampler;
//rtTextureSampler<float4, 2> tex2Sampler;
//rtTextureSampler<float4, 2> tex3Sampler;

rtDeclareVariable(int, texNSampler, , );
rtDeclareVariable(int, tex0Sampler, , );
rtDeclareVariable(int, tex1Sampler, , );
rtDeclareVariable(int, tex2Sampler, , );
rtDeclareVariable(int, tex3Sampler, , );

rtDeclareVariable(float4, texNScale, , );
rtDeclareVariable(float4, texNBias, , );

rtDeclareVariable(float4, tex0Scale, , );
rtDeclareVariable(float4, tex0Bias, , );

rtDeclareVariable(float4, tex1Scale, , );
rtDeclareVariable(float4, tex1Bias, , );

rtDeclareVariable(float4, tex2Scale, , );
rtDeclareVariable(float4, tex2Bias, , );

rtDeclareVariable(float4, tex3Scale, , );
rtDeclareVariable(float4, tex3Bias, , );

rtDeclareVariable(int,    flags, , );
rtDeclareVariable(float,  constant, , );

rtDeclareVariable(Hit,    hit_prd, rtPayload, );


static __device__ __inline__ optix::uchar4 make_color(const optix::float4& c)
{
    return optix::make_uchar4(static_cast<unsigned char>(__saturatef(c.x)*255.99f),  /* B */
                              static_cast<unsigned char>(__saturatef(c.y)*255.99f),  /* G */
                              static_cast<unsigned char>(__saturatef(c.z)*255.99f),  /* R */
                              static_cast<unsigned char>(__saturatef(c.w)*255.99f)); /* A */
}

RT_PROGRAM void closestHit() {

    //// Vertex indices
    const int3 v_idx = geometryIndex[hit_attr.primitiveIndex];

    const float3 p0 = vertexPosition[v_idx.x];
    const float3 p1 = vertexPosition[v_idx.y];
    const float3 p2 = vertexPosition[v_idx.z];

    //// Ray-triangle intersection
    //const float3 e0 = p1 - p0;
    //const float3 e1 = p0 - p2;
    //const float3 geometricNormal = cross(e1, e0);

    //const float3 wsP0 = rtTransformPoint(RT_OBJECT_TO_WORLD, p0);
    //const float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));
    //// Compute t
    //const float d = dot(wsGeometricNormal, ray.direction);
    //const float3 e2 = (1.0f / d) * (wsP0 - ray.origin);
    //const float t = dot(wsGeometricNormal, e2);

    //hit_attr.backface = (d > 0.0f) ? 1 : 0;
    const float beta = hit_attr.barycentrics.x;
    const float gamma = hit_attr.barycentrics.y;

    hit_prd = hit_attr;


    const float3 hitPos = rtTransformPoint(RT_OBJECT_TO_WORLD, p1 * beta + p2 * gamma + p0 * (1.0f - beta - gamma));


    // We've encoded the desired MIP level in the ray direction length
    int mipLevel = lod; // 0 if not set
        //(lod == 0) ? length(ray.direction) - 1 : lod;

    float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, vertexNormal[v_idx.y] * beta + vertexNormal[v_idx.z] * gamma + vertexNormal[v_idx.x] * (1.0f - beta - gamma)));

    // Ray cones, empty (set to 0.0f) if not set.
    float angle = rayConeAnglesBuffer[launch_index];
    mipLevel += log(angle * length(hitPos - ray.origin) * (1.0f / max(abs(dot(normal, xyz(rayDirectionBuffer[launch_index]))), 0.000001f)));

    const float frontface = (twoSided || rtIsTriangleHitFrontFace()) ? 1.0f : 0.0f;
    const float flip = (twoSided && ! rtIsTriangleHitFrontFace()) ? 0.0f : 1.0f;


    // Old wave.lib returned hit distance in the w channel as an idiosyncrasy of 
    // the DDGI project. We now instead return 1.0f to conform to the G3D::TriTree API.
    positionOutBuffer[launch_index] = make_float4(hitPos, 1.0f);

    const float2 texcoord = vertexTexcoord[v_idx.y] * beta + vertexTexcoord[v_idx.z] * gamma + vertexTexcoord[v_idx.x] * (1.0f - beta - gamma);

    const float4 texNSample = SAMPLE(texNSampler, texcoord.x, texcoord.y, 0, mipLevel) * texNScale + texNBias;

    material0OutBuffer[launch_index] = make_color(SAMPLE(tex0Sampler, texcoord.x, texcoord.y, 0, mipLevel) * tex0Scale + tex0Bias);
    material1OutBuffer[launch_index] = make_color(SAMPLE(tex1Sampler, texcoord.x, texcoord.y, 0, mipLevel) * tex1Scale + tex1Bias);
    material2OutBuffer[launch_index] = make_color(SAMPLE(tex2Sampler, texcoord.x, texcoord.y, 0, mipLevel) * tex2Scale + tex2Bias);
    material3OutBuffer[launch_index] = make_float4(xyz(SAMPLE(tex3Sampler, texcoord.x, texcoord.y, 0, mipLevel) * tex3Scale + tex3Bias), constant);
    
    // Normal is automatically flipped if we hit a "backface" on a twoSided object. It is *not* flipped if we hit a backface otherwise.
    shadingNormalOutBuffer[launch_index] = make_float4(normal * (2.0f * flip - 1.0f), frontface);
    
     //TODO: Morgan, figure out the coordinate system for this section
     //Transform the texn.xyz value to world space using the tangent frame
    //const float3 tangent2 = normalize(cross(hit_prd.interpolatedTangent, hit_prd.interpolatedNormal));
    //const float3 tsNormal = xyz(hit_prd.texN);
    //hit_prd.texN.x = dot(hit_prd.interpolatedTangent, tsNormal);
    //hit_prd.texN.y = dot(tangent2, tsNormal);
    //hit_prd.texN.z = dot(hit_prd.interpolatedNormal, tsNormal);
}


RT_PROGRAM void occlusionClosestHit() {
    hit_prd = hit_attr;
}


// Used for both occlusion cast and regular cast
RT_PROGRAM void anyHit() {
//// Vertex indices
    const int3 v_idx = geometryIndex[hit_attr.primitiveIndex];

    const float beta = hit_attr.barycentrics.x;
    const float gamma = hit_attr.barycentrics.y;
    const int mipLevel = lod;

    const float2 texcoord = vertexTexcoord[v_idx.y] * beta + vertexTexcoord[v_idx.z] * gamma + vertexTexcoord[v_idx.x] * (1.0f - beta - gamma);
    //printf("anyHit, index %u, %u\nval = %f\n", launch_index.x, launch_index.y, (rtTex2D<float4>(tex0Sampler, hit_attr.texcoord.x, hit_attr.texcoord.y).w * tex0Scale.w + tex0Bias.w));
    float test = SAMPLE(tex0Sampler, texcoord.x,texcoord.y, 0, mipLevel).w * tex0Scale.w + tex0Bias.w;
    if ( alphaTest && ( (test < partialCoverageThreshold) || (test == 0.0f) )) {
        // Failed backface test
        rtIgnoreIntersection();
    }

}

//const float INFINITY = __int_as_float(0x7f800000);

/** Shared by both cast() and occlusionCast() */
__device__ void trace(Hit& hit_prd, int rayType) {
    //printf("trace, index %u, %u\n", launch_index.x, launch_index.y);
    //if (launch_index.x > (uint)32 || launch_index.y > (uint)32) {
    //    printf("Out! %u, %u\n", launch_index.x, launch_index.y);
    //}
    //hit_prd.t           = -1.0f;

    hit_prd.primitiveIndex = -1;

    //const float scale = 3.0f;
    //const float x = scale * (-0.5f + ((float)launch_index.x / 1920.0f));
    //const float y = scale * (-0.5f + ((float)launch_index.y / 1080.0f));
    const float4 direction = rayDirectionBuffer[launch_index];
	// Early out, faster than degenerate rays according to Adam.
	//if (direction.w == -1.0f) {
	//	return;
	//}
    const float4 origin = rayOriginBuffer[launch_index];
        //make_float4(x, y, 0.0f, 0.0f);
    //make_float4(0.0f, 0.0f, -1.0f, 100.0f);//
    //printf("origin, direction = (%f, %f, %f, %f), (%f, %f, %f, %f)\n", origin.x, origin.y, origin.z, origin.w, direction.x, direction.y, direction.z, direction.w);
    //
    
    // If we're not tracing occlusion rays, disable backface culling.
    rtTrace(root, make_Ray(xyz(origin), xyz(direction), rayType, origin.w, direction.w), hit_prd, visibilityMask, (rayType || !backfaceCull) ? RT_RAY_FLAG_NONE : RT_RAY_FLAG_CULL_BACK_FACING_TRIANGLES);
}


/** Ray generation program for general cast. Because it is a generation function, there is no access to the ray struct. Trying to use it will cause a trap error.*/
RT_PROGRAM void cast() {
    Hit hit_prd;
    trace(hit_prd, 0);
}    

 
RT_PROGRAM void occlusionCast() {
    Hit hit_prd;
    trace(hit_prd, 1);

    hitOutBuffer[launch_index] = (hit_prd.primitiveIndex >= 0) ? 255 : 0;
}


// Only used for debugging
RT_PROGRAM void exception() {
    const unsigned int code = rtGetExceptionCode();
}

/** Writes black to the output buffers.*/
RT_PROGRAM void miss() {
    const float4 missVal = make_float4(0.0f, 0.0f, 0.0f, -1.0f);
    material3OutBuffer[launch_index] = missVal;
    shadingNormalOutBuffer[launch_index] = missVal;
    positionOutBuffer[launch_index] = make_float4(0.0f, 0.0f, 0.0f, INFINITY);
}