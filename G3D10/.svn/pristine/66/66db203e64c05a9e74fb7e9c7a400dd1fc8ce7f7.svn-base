/**
  \file G3D-base.lib/source/Vector4int8.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/Vector4int8.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/Vector4.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/G3DString.h"

namespace G3D {
	
    // TODO: ddgi restore
    Vector4uint8::Vector4uint8(const Vector4& source) {
        x = iClamp(iRound(source.x), 0, 255);
        y = iClamp(iRound(source.y), 0, 255);
        z = iClamp(iRound(source.z), 0, 255);
        w = iClamp(iRound(source.w), 0, 255);
    }

Vector4int8::Vector4int8(const Vector4& source) {
    x = iClamp(iRound(source.x), -128, 127);
    y = iClamp(iRound(source.y), -128, 127);
    z = iClamp(iRound(source.z), -128, 127);
    w = iClamp(iRound(source.w), -128, 127);
}

Vector4int8::Vector4int8(const Vector3& source, int8 w) : w(w) {
    x = iClamp(iRound(source.x), -128, 127);
    y = iClamp(iRound(source.y), -128, 127);
    z = iClamp(iRound(source.z), -128, 127);
}

Vector4int8::Vector4int8(class BinaryInput& b) {
    deserialize(b);
}

void Vector4int8::serialize(class BinaryOutput& b) const {
    // Intentionally write individual bytes to avoid endian issues
    b.writeInt8(x);
    b.writeInt8(y);
    b.writeInt8(z);
    b.writeInt8(w);
}

void Vector4int8::deserialize(class BinaryInput& b) {
    x = b.readInt8();
    y = b.readInt8();
    z = b.readInt8();
    w = b.readInt8();
}

} // namespace G3D

