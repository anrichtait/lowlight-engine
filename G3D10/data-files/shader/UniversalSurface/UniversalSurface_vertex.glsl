/**
  \file data-files/shader/UniversalSurface/UniversalSurface_vertex.glsl

  Declares the input variables and provides a helper function transform() that computes the
  object-to-world space transformation and related vertex attributes.

  This is packaged separately from UniversalSurface_render.vrt to make it easy to compute 
  the object-space positions procedurally in related shaders but still use the material and
  lighting model from UniversalSurface.

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef UniversalSurface_vertex_h
#define UniversalSurface_vertex_h
#include <compatibility.glsl>
#include <g3dmath.glsl>

#expect HAS_VERTEX_COLOR
#expect NUM_LIGHTMAP_DIRECTIONS "0, 1, or 3"

#if __VERSION__ < 330
#   extension GL_ARB_separate_shader_objects : enable
#endif

// Fix some problems on Intel GPUs
//#if (__VERSION__ < 420)
#   define layout(ignore)
//#endif

#ifdef CUSTOMCONSTANT
    uniform vec4    customConstant;
#endif

in vec4 g3d_Vertex;
in vec3 g3d_Normal;
in vec2 g3d_TexCoord0;
in vec2 g3d_TexCoord1;
in vec4 g3d_PackedTangent;

// Fix some problems on AMD Radeon and Intel GPUs [try MacBook Pro 2016 Windows 10]
#if __VERSION__ < 420 || defined(G3D_INTEL)
#   define layout(ignore)
#endif

#include <UniversalSurface/UniversalSurface_vertexOutputs.glsl>
#include <UniversalSurface/UniversalSurface_vertexHelpers.glsl>

void UniversalSurface_transform(in vec4 osVertex, in vec3 osNormal, in vec4 osPackedTangent, in vec2 texCoord0, in vec2 texCoord1, in vec4 vColor) {
    vec3 wsEyePos = g3d_CameraToWorldMatrix[3].xyz;

    wsPosition = (g3d_ObjectToWorldMatrix * osVertex).xyz;
    csPosition = (g3d_ObjectToCameraMatrix * osVertex).xyz;
    
    tan_Z = g3d_ObjectToWorldNormalMatrix * osNormal.xyz;

#   ifdef HAS_NORMAL_BUMP_MAP      
        tan_X = mat3(g3d_ObjectToWorldMatrix) * osPackedTangent.xyz;

        // T and N are guaranteed perpendicular, so B is normalized.  Its facing 
        // direction is stored in the texcoord w component.
        tan_Y = cross(tan_Z, tan_X) * osPackedTangent.w;
#       if PARALLAXSTEPS > 0
            // Compute the tangent space eye vector (ATI does not support the transpose operator)
            mat3 worldToTangent = mat3(tan_X.x, tan_Y.x, tan_Z.x,
                                       tan_X.y, tan_Y.y, tan_Z.y,
                                       tan_X.z, tan_Y.z, tan_Z.z);
            _tsE             = worldToTangent * (wsEyePos - wsPosition);
#       endif  
#   endif

    texCoord = texCoord0;
#   if (NUM_LIGHTMAP_DIRECTIONS > 0)
        lightMapCoord = texCoord1;
#   endif    

#   if HAS_VERTEX_COLOR
        vertexColor = vColor;
#   endif

    // Multiply the matrix on the RIGHT because that gives higher precision (at least on NVIDIA cards)
    // See http://graphics.cs.williams.edu/papers/SAOHPG12/ for discussion
    gl_Position   = osVertex * g3d_ObjectToScreenMatrixTranspose;
}

#endif
