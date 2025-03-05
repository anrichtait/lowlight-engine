#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 invTransModel;
    mat4 view;
    mat4 proj;
    mat4 modelViewProj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec3 color;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 position;
layout(location = 4) out vec3 tangent;
layout(location = 5) out flat float tangentYSign;

void main() {
    vec4 postProj = ubo.modelViewProj * vec4(inPosition.xyz, 1.0);
    gl_Position = postProj;
    color = inColor;
    texCoord = inTexCoord;
    normal = normalize(vec3(ubo.invTransModel * vec4(inNormal, 0)));
    tangent = normalize(mat3x3(ubo.model) * inTangent.xyz);
    position = vec3(ubo.model * vec4(inPosition.xyz, 1.0));
    tangentYSign = inTangent.w;
}