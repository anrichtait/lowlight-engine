/**
  \file G3D-base.lib/source/Vector3int16.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Vector3int16.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/format.h"

namespace G3D {

Vector3int16::Vector3int16(const class Vector3& v) {
    x = (int16)iFloor(v.x + 0.5);
    y = (int16)iFloor(v.y + 0.5);
    z = (int16)iFloor(v.z + 0.5);
}


Vector3int16::Vector3int16(class BinaryInput& bi) {
    deserialize(bi);
}


void Vector3int16::serialize(class BinaryOutput& bo) const {
    bo.writeInt16(x);
    bo.writeInt16(y);
    bo.writeInt16(z);
}


void Vector3int16::deserialize(class BinaryInput& bi) {
    x = bi.readInt16();
    y = bi.readInt16();
    z = bi.readInt16();
}

String Vector3int16::toString() const {
    return G3D::format("(%d, %d, %d)", x, y, z);
}

    
Vector3int16 Vector3int16::floor(const Vector3& v) {
    return Vector3int16(iFloor(v.x), iFloor(v.y), iFloor(v.z));
}


Vector3int16 Vector3int16::ceil(const Vector3& v) {
    return Vector3int16(iCeil(v.x), iCeil(v.y), iCeil(v.z));
}

}
