/**
  \file G3D-base.lib/source/Color1unorm8.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-base/unorm8.h"
#include "G3D-base/Color1unorm8.h"
#include "G3D-base/Color1.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"

namespace G3D {

Color1unorm8::Color1unorm8(const class Color1& c) : value(c.value) {
}


Color1unorm8::Color1unorm8(class BinaryInput& bi) {
    deserialize(bi);
}


void Color1unorm8::serialize(class BinaryOutput& bo) const {
    bo.writeUInt8(value.bits());
}


void Color1unorm8::deserialize(class BinaryInput& bi) {
    value = unorm8::fromBits(bi.readUInt8());
}


}
