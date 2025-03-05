/**
  \file G3D-base.lib/source/Vector2uint32.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Vector2uint32.h"
#include "G3D-base/Vector2.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/Vector2int16.h"
#include "G3D-base/Any.h"

namespace G3D {
    
Vector2uint32 Vector2uint32::parseResolution(const String& s) {
    TextInput ti(TextInput::FROM_STRING, toLower(s));
    const unsigned int width = ti.readInteger();
    ti.readSymbol("x");
    const unsigned int height = ti.readInteger();

    return Vector2uint32(width, height);
}


Vector2uint32::Vector2uint32(const Any& any) {
    any.verifyName("Vector2uint32", "Point2uint32");
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


Vector2uint32::Vector2uint32(const class Vector2int16& v) : x(v.x), y(v.y) {
}

Vector2uint32::Vector2uint32(const class Vector2& v) {
    x = (uint32)iFloor(v.x + 0.5);
    y = (uint32)iFloor(v.y + 0.5);
}


Vector2uint32::Vector2uint32(class BinaryInput& bi) {
    deserialize(bi);
}


void Vector2uint32::serialize(class BinaryOutput& bo) const {
    bo.writeUInt32(x);
    bo.writeUInt32(y);
}


void Vector2uint32::deserialize(class BinaryInput& bi) {
    x = bi.readUInt32();
    y = bi.readUInt32();
}


Vector2uint32 Vector2uint32::clamp(const Vector2uint32& lo, const Vector2uint32& hi) {
    return Vector2uint32(uiClamp(x, lo.x, hi.x), uiClamp(y, lo.y, hi.y));
}


String Vector2uint32::toString() const {
    return G3D::format("(%d, %d)", x, y);
}

}
