/**
  \file G3D-base.lib/include/G3D-base/SplineExtrapolationMode.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_SplineExtrapolationMode_h

#include "G3D-base/platform.h"
#include "G3D-base/enumclass.h"

namespace G3D {

/** 
  Describes the behavior of G3D::Spline, etc. when accessing a
  time outside of the control point range.
  \sa SplineInterpolationMode, Spline, PhysicsFrameSpline
*/
G3D_DECLARE_ENUM_CLASS(SplineExtrapolationMode, CYCLIC, LINEAR, CLAMP);


/** Describes the interpolation behavior of G3D::Spline 
    \sa SplineExtrapolationMode, Spline, PhysicsFrameSpline */ 
G3D_DECLARE_ENUM_CLASS(SplineInterpolationMode, LINEAR, CUBIC);

} // namespace G3D
