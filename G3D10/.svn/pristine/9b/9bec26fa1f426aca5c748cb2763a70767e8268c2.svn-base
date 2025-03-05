/**
  \file G3D-base.lib/source/Vector2uint16.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Vector2uint16.h"
#include "G3D-base/Vector2.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/Any.h"
#include "G3D-base/Vector2int32.h"

namespace G3D {

Vector2uint16::Vector2uint16(const class Vector2int32& v) : x(v.x), y(v.y) {}

Vector2uint16::Vector2uint16(const Any& any) {
    any.verifyName("Vector2uint16", "Point2int16");
    any.verifyType(Any::TABLE, Any::ARRAY);
    any.verifySize(2);

    if (any.type() == Any::ARRAY) {
        x = any[0];
        y = any[1];
    } else {
        // Table
        x = any["x"];
        y = any["y"];
    }
}


Vector2uint16& Vector2uint16::operator=(const Any& a) {
    *this = Vector2uint16(a);
    return *this;
}


Any Vector2uint16::toAny() const {
    Any any(Any::ARRAY, "Vector2uint16");
    any.append(int(x), int(y));
    return any;
}


Vector2uint16::Vector2uint16(const class Vector2& v) {
    x = (uint16)iFloor(v.x + 0.5);
    y = (uint16)iFloor(v.y + 0.5);
}


Vector2uint16::Vector2uint16(class BinaryInput& bi) {
    deserialize(bi);
}


void Vector2uint16::serialize(class BinaryOutput& bo) const {
    bo.writeUInt16(x);
    bo.writeUInt16(y);
}


void Vector2uint16::deserialize(class BinaryInput& bi) {
    x = bi.readUInt16();
    y = bi.readUInt16();
}


Vector2uint16 Vector2uint16::clamp(const Vector2uint16& lo, const Vector2uint16& hi) {
    return Vector2uint16(iClamp(x, lo.x, hi.x), iClamp(y, lo.y, hi.y));
}


}
