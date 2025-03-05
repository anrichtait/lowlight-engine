#version 450
#extension GL_ARB_separate_shader_objects : enable


void main() {
    // Draw a screen filling quad.
    gl_Position = vec4(vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2) * 2.0f + -1.0f, 0.0f, 1.0f);
}