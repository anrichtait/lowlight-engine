// G3D GLSL implementation of the shadertoy.com API

void mainImage(out vec4 fragColor, in vec2 fragCoord);

uniform int iFrame;
uniform float iTime;
uniform float iTimeDelta;
const float iSampleRate = 44100.0;
ivec2 iResolution = ivec2(g3d_FragCoordExtent);

out vec4 _fragColor;
void main() {
    mainImage(_fragColor, ivec2(gl_FragCoord.xy));
    _fragColor.a = 1.0;
}