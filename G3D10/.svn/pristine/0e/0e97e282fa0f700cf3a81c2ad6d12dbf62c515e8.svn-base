/**
  \file G3D-base.lib/source/Color1.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/Color1.h"
#include "G3D-base/Color1unorm8.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/Color3.h"

namespace G3D {

const Color1& Color1::one() {
    static const Color1 x(1.0f);
    return x;
}


const Color1& Color1::zero() {
    const static Color1 x(0.0f);
    return x;
}

const Color1& Color1::nan() {
    static const Color1 x(fnan());
    return x;
}


Color1::Color1(BinaryInput& bi) {
    deserialize(bi);
}


Color3 Color1::rgb() const {
    return Color3(value, value, value);
}


void Color1::deserialize(BinaryInput& bi) {
    value = bi.readFloat32();
}


void Color1::serialize(BinaryOutput& bo) const {
    bo.writeFloat32(value);
}


Color1::Color1(const class Color1unorm8& other) {
    value = other.value;
}

} // namespace G3D

