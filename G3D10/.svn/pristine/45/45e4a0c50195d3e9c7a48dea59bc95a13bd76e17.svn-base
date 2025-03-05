/**
  \file data-files/shader/UniversalSurface/UniversalSurface_vertexOutputs.glsl

  Declares the vertex shader outputs variables.
  
  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef UniversalSurface_vertexOutputs_h
#define UniversalSurface_vertexOutputs_h
#include <compatibility.glsl>
#include <g3dmath.glsl>


// Specify locations explicitly so that the geometry shader is not required 
// to match the vertex shader output names
layout(location=0) out Point2  texCoord;
layout(location=1) out Point3  wsPosition;

/** This is the normal in world space*/
layout(location=2) out Vector3 tan_Z; 

#if (NUM_LIGHTMAP_DIRECTIONS > 0)
    out layout(location=3) vec2 lightMapCoord;
#endif

#ifdef HAS_NORMAL_BUMP_MAP  
    /** Tangent space to world space.
        Note that this will be linearly interpolated across the polygon. */
    out layout(location=4) Vector3 tan_X;
    out layout(location=5) Vector3 tan_Y;

#   if PARALLAXSTEPS > 0
        /** Vector to the eye in tangent space (needed for parallax) */
        out layout(location=6) Vector3 _tsE;
#   endif
#endif

#if defined(GBUFFER_HAS_CS_POSITION_CHANGE) || defined(GBUFFER_HAS_SS_POSITION_CHANGE)
        out layout(location = 7) vec3 csPrevPosition;
#endif

#if HAS_VERTEX_COLOR
    out layout(location=10) Color4 vertexColor;
#endif

out layout(location=11) Point3 csPosition;

#endif