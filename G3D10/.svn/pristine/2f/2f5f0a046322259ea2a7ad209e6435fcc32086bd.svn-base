/**
  \file G3D-base.lib/include/G3D-base/BumpMapPreprocess.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_base_BumpMapPreprocess_h

#include "G3D-base/platform.h"
#include "G3D-base/enumclass.h"

namespace G3D {
class Any;

/** 
Not in the BumpMap class to avoid a circular dependency between Texture and BumpMap.
G3D::Image::computeNormalMap().
*/
class BumpMapPreprocess {
public:
     G3D_DECLARE_ENUM_CLASS(Mode,
        /** Use the format as-is. */
        NONE,

        /** Convert the input bump map to a normal-bump map. */
        BUMP_TO_NORMAL_AND_BUMP,

        /** Load as a normal map and leave unchanged */
        NORMAL_TO_NORMAL,

        /** Convert the input normal map to a normal-bump map. Slow */
        NORMAL_TO_NORMAL_AND_BUMP,

        /** Use as-is if this appears to be a normal map, otherwise, convert the bump map to a normal map. */
        AUTODETECT_TO_NORMAL,

        /** Use as-is if this appears to be a normal-bump map, otherwise convert to a normal-bump map.
            Slow if the input is a normal map. */
        AUTODETECT_TO_NORMAL_AND_BUMP,

        /** Use as-is as a normal or normal-bump map. If the input appears to
            be a bump-only map, convert to a bump-normal map. This is the default for 
            Texture::Settings::Preprocess::normalMap()
            */
        AUTODETECT_TO_AUTODETECT);

    Mode            mode = Mode::NONE;

    /** If true, the elevations are box filtered after computing normals
     and before uploading, which produces better results for parallax offset mapping
     Defaults to false. */
    bool            lowPassFilter = false;

    /** Height of the maximum ("white") value, in _pixels_, for the purpose of computing normals.
        A value of 1 means that a white pixel next to a black pixel produces a 45-degree ramp.

        A value of 255 means that a 255 x 255 bump image with a full black-to-white gradient
        will produce a 45-degree ramp (this also results in "cubic" voxels from a bump map).
        A negative value means to set zExtentPixels to \code -zExtentPixels * max(width, height) \endcode,
        so that it scales with the width of the texture.

        The default is 4.0f.
     */
    float           zExtentPixels = 4.0f;

    /** After computing normals, scale the height by |N.z|, a trick that reduces texture swim in steep areas for parallax offset 
      mapping. Defaults to false.*/
    bool            scaleZByNz = false;
    
    BumpMapPreprocess() {}

    BumpMapPreprocess(const Any& any);

    Any toAny() const;

    bool operator==(const BumpMapPreprocess& other) const {
        return 
            (lowPassFilter == other.lowPassFilter) &&
            (zExtentPixels == other.zExtentPixels) &&
            (scaleZByNz == other.scaleZByNz) &&
            (mode == other.mode);
    }
};

}

