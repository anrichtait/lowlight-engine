/**
  \file G3D-base.lib/source/BumpMapPreprocess.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/BumpMapPreprocess.h"
#include "G3D-base/Any.h"
#include "G3D-base/stringutils.h"

namespace G3D {

BumpMapPreprocess::BumpMapPreprocess(const Any& any) {
    *this = BumpMapPreprocess();
    AnyTableReader r(any);
    r.getIfPresent("lowPassFilter", lowPassFilter);
    r.getIfPresent("zExtentPixels", zExtentPixels);
    r.getIfPresent("scaleZByNz", scaleZByNz);
    r.getIfPresent("mode", mode);
    r.verifyDone();
}


Any BumpMapPreprocess::toAny() const {
    Any any(Any::TABLE, "BumpMapPreprocess");
    BumpMapPreprocess defaults;
    if (lowPassFilter != defaults.lowPassFilter) { any["lowPassFilter"] = lowPassFilter; }
    if (zExtentPixels != defaults.zExtentPixels) { any["zExtentPixels"] = zExtentPixels; }
    if (scaleZByNz != defaults.scaleZByNz) { any["scaleZByNz"] = scaleZByNz; }
    if (mode != defaults.mode) { any["mode"] = mode; }
    return any;
}

}
