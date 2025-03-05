/**
  \file G3D-base.lib/source/Vector4uint16.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Vector4uint16.h"
#include "G3D-base/Vector4.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/format.h"

namespace G3D {

Vector4uint16::Vector4uint16(const class Vector4& v) {
    x = (uint16)iFloor(v.x + 0.5f);
    y = (uint16)iFloor(v.y + 0.5f);
    z = (uint16)iFloor(v.z + 0.5f);
    w = (uint16)iFloor(v.w + 0.5f);
}


Vector4uint16::Vector4uint16(class BinaryInput& bi) {
    deserialize(bi);
}


void Vector4uint16::serialize(class BinaryOutput& bo) const {
    bo.writeUInt16(x);
    bo.writeUInt16(y);
    bo.writeUInt16(z);
    bo.writeUInt16(w);
}


void Vector4uint16::deserialize(class BinaryInput& bi) {
    x = bi.readUInt16();
    y = bi.readUInt16();
    z = bi.readUInt16();
    w = bi.readUInt16();
}


String Vector4uint16::toString() const {
    return G3D::format("(%d, %d, %d, %d)", x, y, z, w);
}

}
