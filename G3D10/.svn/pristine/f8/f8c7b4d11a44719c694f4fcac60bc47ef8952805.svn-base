#ifndef disneyBDRF_glsl
#define disneyBDRF_glsl

#define PI pi

// The following BRDF is based on https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf

float SchlickFresnel(float u) {
    // pow(m,5)
    float m = clamp(1.0 - u, 0.0, 1.0);
    return square(square(m)) * m;
}

float GTR1(float NdotH, float a) {
    if (a >= 1.0) { return 1.0 / PI; }
    float a2 = square(a);
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    return (a2 - 1.0) / (PI * log(a2) * t);
}

float GTR2(float NdotH, float a) {
    float a2 = square(a);
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    return a2 / (PI * t * t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay) {
    return 1.0 / (PI * ax * ay * square(square(HdotX / ax) + square(HdotY / ay) + NdotH * NdotH));
}

float SmithG_GGX(float NdotV, float alphaG) {
    float a = alphaG * alphaG;
    float b = NdotV * NdotV;
    return 1.0 / (NdotV + sqrt(a + b - a * b));
}

float SmithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay) {
    return 1.0 / (NdotV + sqrt(square(VdotX * ax) + square(VdotY * ay) + square(NdotV)));
}

// See http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
// and https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
// for documentation of material parameters. Unlike their reference code, our baseColor is in linear
// space (not gamma encoded)
//
// L is the unit vector to the light source (omega_in) in world space
// N is the unit normal in world space
// V is the vector to the eye (omega_out) in world space
// X and Y are the tangent directions in world space
vec3 evaluateDisneyBRDF
(vec3    baseColor,
    float   metallic,
    float   subsurface,
    float   specular,
    float   roughness,
    float   specularTint,
    float   anisotropic,
    float   sheen,
    float   sheenTint,
    float   clearcoat,
    float   clearcoatGloss,
    vec3    L,
    vec3    V,
    vec3    N,
    vec3    X,
    vec3    Y) {

    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    if (NdotL < 0 || NdotV < 0) return vec3(0);

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);
    float LdotH = dot(L, H);

    float luminance = dot(baseColor, vec3(0.3, 0.6, 0.1));

    // normalize luminance to isolate hue and saturation components
    vec3 Ctint = (luminance > 0.0) ? baseColor / luminance : vec3(1.0);
    vec3 Cspec0 = mix(specular * 0.08 * mix(vec3(1.0), Ctint, specularTint), baseColor, metallic);
    vec3 Csheen = mix(vec3(1.0), Ctint, sheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2.0 * LdotH * LdotH * roughness;
    float Fd = mix(1, Fd90, FL) * mix(1, Fd90, FV);

    // Based on Hanrahan-Krueger BRDF approximation of isotropic BSSRDF
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    float Fss90 = LdotH * LdotH * roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - 0.5) + 0.5);

    // Specular
    float aspect = sqrt(1.0 - anisotropic * 0.9);
    float ax = max(0.001, square(roughness) / aspect);
    float ay = max(0.001, square(roughness) * aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1.0), FH);
    float Gs = SmithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay) *
        SmithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);

    // sheen
    vec3 Fsheen = FH * sheen * Csheen;

    // clearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(NdotH, mix(0.1, 0.001, clearcoatGloss));
    float Fr = mix(0.04, 1.0, FH);
    float Gr = SmithG_GGX(NdotL, 0.25) * SmithG_GGX(NdotV, 0.25);

    return ((1.0 / PI) * mix(Fd, ss, subsurface) * baseColor + Fsheen) * (1.0 - metallic) +
        Gs * Fs * Ds + 0.25 * clearcoat * Gr * Fr * Dr;
}

#undef PI

#endif
