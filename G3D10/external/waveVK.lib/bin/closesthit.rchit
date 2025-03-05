#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable

struct hitPayload {
    vec4 lambertian;
    vec4 glossy;
    vec4 emissive;
    vec4 shadingNormal;
    vec4 position;
};

struct Material {
    int texNIndex;
    int tex0Index;
    int tex1Index;
    int tex2Index;
    int tex3Index;

    vec4 scaleN;
    vec4 biasN;

    vec4 scale0;
    vec4 bias0;

    vec4 scale1;
    vec4 bias1;

    vec4 scale2;
    vec4 bias2;

    vec4 scale3;
    vec4 bias3;
};

layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;

struct Vertex
{
    vec3 pos;
    vec3 nrm;
    vec4 tan;
    vec2 texCoord;
    int matIndex;
};

layout(binding = 3, set = 0) buffer Vertices { Vertex v[]; }
vertices[];
layout(binding = 4, set = 0) buffer Indices { uint i[]; }
indices[];

layout(binding = 7, set = 0) buffer TransformsInverseTranspose {
    mat4 i[];    
}
transformInverseTranspose;

layout(binding = 13, set = 0) uniform sampler2D[] textureSamplers;

layout(binding = 14, set = 0) buffer Materials {
    Material m[];
}
materials;
//layout(binding = 5, set = 0) buffer MatColorBufferObject { vec4[] m; }
//materials;

layout(location = 0) rayPayloadInNV hitPayload hit;
layout(location = 2) rayPayloadNV bool isShadowed;

hitAttributeNV vec3 attribs;

// Initial implementation of bump-mapped normals. It assumes simple bump-mapping without parallax (occlusion) mapping.
// Code is modified from BumpMap.glsl in G3D. Implementation stalled because the compiler doesn't recognize dFdx and 
// dFdy.

//void bumpMapBlinn78(in sampler2D normalBumpMap, in vec2 texCoord, in vec3 tan_X, in vec3 tan_Y, in vec3 tan_Z, out vec3 wsN, out vec2 offsetTexCoord) {
//
//    offsetTexCoord = texCoord;
//
//    // Bias the MIP-map lookup to avoid specular aliasing
//    float biasFactor = 1.75;
//
//    // Take the normal map values back to (-1, 1) range to compute a tangent space normal
//    vec3 tsN = textureGrad(normalBumpMap, offsetTexCoord, dFdx(offsetTexCoord) * biasFactor, dFdy(offsetTexCoord) * biasFactor).xyz * 2.0 + vec3(-1.0);
//
//    // As we approach the full-res MIP map, let the rawNormal length be 1.0.  This avoids having
//    // bilinear interpolation create short normals under the magnification filter
//    rawNormalLength = lerp(1.0, length(tsN), min(1.0, 0.0f));
//        //computeSampleRate(offsetTexCoord * biasFactor, textureSize(normalBumpMap, 0))));
//
//    // note that the columns might be slightly non-orthogonal due to interpolation
//    mat3 tangentToWorld = mat3(tan_X, tan_Y, tan_Z);
//
//    // Take the normal to world space
//    wsN = (tangentToWorld * tsN);
//
//    // For very tiny geometry the tangent space can become degenerate and produce a bad normal
//    wsN = normalize((dot(wsN, wsN) > 0.001) ? wsN : tan_Z);
//}

void main()
{
    ivec3 ind = ivec3(indices[gl_InstanceID].i[3 * gl_PrimitiveID], indices[gl_InstanceID].i[3 * gl_PrimitiveID + 1],
        indices[gl_InstanceID].i[3 * gl_PrimitiveID + 2]);

    Vertex v0 = vertices[gl_InstanceID].v[ind.x];
    Vertex v1 = vertices[gl_InstanceID].v[ind.y];
    Vertex v2 = vertices[gl_InstanceID].v[ind.z];

    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    vec3 normal = normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);
    vec2 texcoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;
    vec4 tangent = v0.tan * barycentrics.x + v1.tan * barycentrics.y + v2.tan * barycentrics.z;

    // Transforming the normal to world space
    //normal = (normalize(vec3(transformInverseTranspose.i[gl_InstanceID] * vec4(normal, 0.0))) + vec3(1,1,1)) * 0.5f;

    // TODO: restore
    //float maxBary = max(barycentrics.x, max(barycentrics.y, barycentrics.z));
    //vec3 vertexColor;

    //if (maxBary == barycentrics.x) {
    //    vertexColor = vec3(1, 0, 0);
    //}
    //else if (maxBary == barycentrics.y) {
    //    vertexColor = vec3(0, 1, 0);
    //}
    //else if (maxBary == barycentrics.z) {
    //    vertexColor = vec3(0, 0, 1);
    //}


    hit.lambertian = texture(textureSamplers[materials.m[gl_InstanceID].tex0Index - 7], texcoord) * vec4(materials.m[gl_InstanceID].scale0.xyz, 1.0f) + vec4(materials.m[gl_InstanceID].bias0.xyz, 1.0f);
    //vec4((normalize(vec3(gl_ObjectToWorldNV * vec4(normal, 0.0))) + vec3(1,1,1)) * 0.5f, 1.0f);


    // Bump mapping parameters

    //vec4 osNormal = vec4(normalize(texture(textureSamplers[materials.m[gl_InstanceID].texNIndex - 7], texcoord)), 0.0f);

    //vec3 tan_Z = (gl_ObjectToWorldNV * osNormal).xyz;
    //vec3 tan_X = mat3(gl_ObjectToWorldNV) * tangent,xyz;
    //vec3 tan_Y = cross(tan_Z, tan_X) * tangent.w;

    //vec3 wsNormal;
    //vec2 offsetTexCoord;

    //bumpMapBlinn78(textureSamplers[materials.m[gl_InstancID].texNIndex - 7], texCoord, tan_X, tan_Y, tan_Z, wsN, offsetTexCoord);


    //hit.glossy = texture(textureSamplers[materials.m[gl_InstanceID].tex1Index - 7], texcoord);// * materials.m[gl_InstanceID].scale2 + materials.m[gl_InstanceID].bias2;
    //hit.emissive = texture(textureSamplers[materials.m[gl_InstanceID].tex3Index - 7], texcoord);// * materials.m[gl_InstanceID].scale3 + materials.m[gl_InstanceID].bias3;
    //hit.shadingNormal = normalize(texture(textureSamplers[materials.m[gl_InstanceID].texNIndex - 7], texcoord)); 
    //    //vec4(normalize(vec3(gl_ObjectToWorldNV * vec4(normal, 0.0))), 0.0f);
    //hit.position = vec4(gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV, gl_HitTNV);

    hit.glossy = texture(textureSamplers[materials.m[gl_InstanceID].tex1Index - 7], texcoord)   * materials.m[gl_InstanceID].scale1 + materials.m[gl_InstanceID].bias1;
    hit.emissive = vec4((texture(textureSamplers[materials.m[gl_InstanceID].tex3Index - 7], texcoord) * materials.m[gl_InstanceID].scale3 + materials.m[gl_InstanceID].bias3).xyz, 1.0f);

  hit.shadingNormal = vec4(normalize(vec3(gl_ObjectToWorldNV * vec4(normal, 0.0))), 0.0f); 
      //texture(textureSamplers[materials.m[gl_InstanceID].texNIndex - 7], texcoord) * materials.m[gl_InstanceID].scaleN + materials.m[gl_InstanceID].biasN;
    
  hit.position = vec4(gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV, gl_HitTNV);

  //hitValue = c; 

}
