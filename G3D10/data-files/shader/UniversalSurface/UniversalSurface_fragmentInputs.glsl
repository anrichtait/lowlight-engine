/**
  \file data-files/shader/UniversalSurface/UniversalSurface_fragmentInputs.glsl

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include <compatibility.glsl>
#include <g3dmath.glsl>

#if NUM_LIGHTMAP_DIRECTIONS > 0
layout(location = 3) varying vec2 lightMapCoord;
#endif

/** Texture coordinate */
layout(location = 0) in Point2 texCoord;

layout(location = 1) in Point3 wsPosition;

layout(location = 2) in Vector3 tan_Z;

#if HAS_NORMAL_BUMP_MAP
#   if (PARALLAXSTEPS > 0)
/** Un-normalized (interpolated) tangent space eye vector */
layout(location = 6) in vec3     _tsE;
#   endif    
layout(location = 4)   in vec3       tan_X;
layout(location = 5)   in vec3       tan_Y;
#endif

#if defined(GBUFFER_HAS_CS_POSITION_CHANGE) || defined(GBUFFER_HAS_SS_POSITION_CHANGE)
    layout(location = 7) in Point3 csPrevPosition;
#endif

// Layouts 8 and 9 are for SVO, and come from the geometry shader.
// Fragment shaders that use these declare them explicitly.
//#ifdef SVO_POSITION
//     //These inputs are not included in UniversalSurface_fragmentInputs.glsl
//     //because they come from the geometry shader.
//    layout(location = 8) in vec3          svoPosition;
//    layout(location = 9) flat in  int	triangleAxis;
//#endif


#if HAS_VERTEX_COLOR
    layout(location = 10) in vec4         vertexColor;
#endif

/** Surface position in camera space */
    layout(location = 11) in Point3           csPosition;