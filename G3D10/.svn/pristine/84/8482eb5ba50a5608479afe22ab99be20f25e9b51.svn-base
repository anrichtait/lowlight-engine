/**
  \file G3D-base.lib/include/G3D-base/Color1unorm8.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_Color1unorm8_h
#define G3D_Color1unorm8_h

#include "G3D-base/platform.h"
#include "G3D-base/unorm8.h"

namespace G3D {

/**
 Represents a single-channel color on [0, 1] with G3D::unorm8 precision.
 Equivalent to OpenGL GL_R8, GL_A8, and GL_L8 formats.
 */
G3D_BEGIN_PACKED_CLASS(1)
Color1unorm8 {
private:
    // Hidden operators
    bool operator<(const Color1unorm8&) const;
    bool operator>(const Color1unorm8&) const;
    bool operator<=(const Color1unorm8&) const;
    bool operator>=(const Color1unorm8&) const;

public:

    unorm8       value;

    Color1unorm8() : value(unorm8::fromBits(0)) {}

    explicit Color1unorm8(const unorm8 _v) : value(_v) {}

    explicit Color1unorm8(const class Color1& c);

    explicit Color1unorm8(class BinaryInput& bi);

    void serialize(class BinaryOutput& bo) const;

    void deserialize(class BinaryInput& bi);

    inline bool operator==(const Color1unorm8& other) const {
        return value == other.value;
    }

    inline bool operator!=(const Color1unorm8& other) const {
        return value != other.value;
    }

}
G3D_END_PACKED_CLASS(1)
}
#endif
