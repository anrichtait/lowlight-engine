#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable

struct hitPayload {
    vec4 lambertian;
    vec4 glossy;
    vec4 emissive;
    vec4 shadingNormal;
    vec4 position;
};

layout(location = 0) rayPayloadInNV hitPayload hit;

void main()
{
    const vec4 missVal = vec4(0.0f,0.0f,0.0f, -1.0f);
    hit.emissive = missVal;
    hit.shadingNormal = missVal;
    // TODO: real infinity
    hit.position = vec4(0.0f,0.0f,0.0f, 1.0f / 0.0f);
}