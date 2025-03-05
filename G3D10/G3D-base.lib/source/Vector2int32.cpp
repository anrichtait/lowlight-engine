/**
  \file G3D-base.lib/source/Vector2int32.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Vector2int32.h"
#include "G3D-base/Vector2.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/Vector2int16.h"
#include "G3D-base/Vector2uint16.h"
#include "G3D-base/Any.h"

namespace G3D {

Vector2int32::Vector2int32(const Any& any) {
    any.verifyName("Vector2int32", "Point2int32");
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


Vector2int32 Vector2int32::parseResolution(const String& s) {
    // Doesn't parse correctly
    // int width = 0, height = 0;
    // sscanf(s.c_str(), "%d%*s%d", &width, &height);

    TextInput ti(TextInput::FROM_STRING, toLower(s));
    const int width = ti.readInteger();
    ti.readSymbol("x");
    const int height = ti.readInteger();

    return Vector2int32(width, height);
}


Vector2int32::Vector2int32(const class Vector2int16& v) : x(v.x), y(v.y) {
}

Vector2int32::Vector2int32(const class Vector2uint16& v) : x(v.x), y(v.y) {
}


Vector2int32::Vector2int32(const class Vector2& v) {
    x = (int32)iFloor(v.x + 0.5);
    y = (int32)iFloor(v.y + 0.5);
}


Vector2int32::Vector2int32(class BinaryInput& bi) {
    deserialize(bi);
}


void Vector2int32::serialize(class BinaryOutput& bo) const {
    bo.writeInt32(x);
    bo.writeInt32(y);
}


void Vector2int32::deserialize(class BinaryInput& bi) {
    x = bi.readInt32();
    y = bi.readInt32();
}


Vector2int32 Vector2int32::clamp(const Vector2int32& lo, const Vector2int32& hi) {
    return Vector2int32(iClamp(x, lo.x, hi.x), iClamp(y, lo.y, hi.y));
}


String Vector2int32::toString() const {
    return G3D::format("(%d, %d)", x, y);
}

}
