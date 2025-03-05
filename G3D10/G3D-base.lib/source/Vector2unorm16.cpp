/**
  \file G3D-base.lib/source/Vector2unorm16.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Vector2.h"
#include "G3D-base/Vector2unorm16.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/Any.h"

namespace G3D {

Vector2unorm16::Vector2unorm16(const Any& any) {
    any.verifyName("Vector2unorm16", "Point2unorm16");
    any.verifyType(Any::TABLE, Any::ARRAY);
    any.verifySize(2);

    if (any.type() == Any::ARRAY) {
        x = unorm16(any[0]);
        y = unorm16(any[1]);
    } else {
        // Table
        x = unorm16(any["x"]);
        y = unorm16(any["y"]);
    }
}


Vector2unorm16& Vector2unorm16::operator=(const Any& a) {
    *this = Vector2unorm16(a);
    return *this;
}


Any Vector2unorm16::toAny() const {
    Any any(Any::ARRAY, "Vector2unorm16");
    any.append(x, y);
    return any;
}

Vector2unorm16::Vector2unorm16(const class Vector2& v) {
    x = unorm16(v.x);
    y = unorm16(v.y);
}


Vector2unorm16::Vector2unorm16(class BinaryInput& bi) {
    deserialize(bi);
}


void Vector2unorm16::serialize(class BinaryOutput& bo) const {
    bo.writeUInt16(x.bits());
    bo.writeUInt16(y.bits());
}


void Vector2unorm16::deserialize(class BinaryInput& bi) {
    x = unorm16::fromBits(bi.readUInt16());
    y = unorm16::fromBits(bi.readUInt16());
}

}
