/**
  \file G3D-base.lib/source/unorm16.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-base/Any.h"
#include "G3D-base/unorm16.h"

namespace G3D {
unorm16::unorm16(const class Any& a) {
    *this = unorm16(a.number());
}

Any unorm16::toAny() const {
    return Any((float)*this);
}

}
