/**
  \file G3D-gfx.lib/include/G3D-gfx/DXCaps.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_gfx_DXCaps_h

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"


namespace G3D {

/** 
    Provides very basic DirectX detection and information support
*/
class DXCaps {

public:
    /**
        Returns 0 if not installed otherwise returns the major and minor number
        in the form (major * 100) + minor.  eg. 900 is 9.0 and 901 is 9.1
    */
    static uint32 version();

    /**
        Returns the amount of video memory detected by Direct3D in bytes. 
        This may not be the true amount of physical RAM; some graphics cards
        over or under report.
    */
    static uint64 videoMemorySize();
};

} // namespace G3D

