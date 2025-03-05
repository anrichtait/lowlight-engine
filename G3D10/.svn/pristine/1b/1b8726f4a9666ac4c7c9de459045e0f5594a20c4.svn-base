/**
  \file G3D-base.lib/source/BinaryFormat.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/BinaryFormat.h"

namespace G3D {

int32 byteSize(BinaryFormat f) {
    switch (f) {
    case BOOL8_BINFMT:
    case UINT8_BINFMT:
    case INT8_BINFMT:
        return 1;

    case UINT16_BINFMT:
    case INT16_BINFMT:
        return 2;

    case FLOAT16_BINFMT:
        return 2;

    case UINT32_BINFMT:
    case INT32_BINFMT:
    case FLOAT32_BINFMT:
        return 4;

    case FLOAT64_BINFMT:
    case UINT64_BINFMT:
    case INT64_BINFMT:
        return 8;

    case INT128_BINFMT:
    case UINT128_BINFMT:
        return 16;

    case VECTOR2_BINFMT:
        return 2 * 4;

    case VECTOR2INT16_BINFMT:
        return 2 * 2;

    case VECTOR3_BINFMT:
        return 3 * 4;

    case VECTOR3INT16_BINFMT:
        return 3 * 2;

    case VECTOR4_BINFMT:
        return 4 * 4;

    case VECTOR4INT16_BINFMT:
        return 4 * 4;

    case VECTOR4UINT16_BINFMT:
        return 4 * 4;

    case COLOR3_BINFMT:
        return 3 * 4;

    case COLOR3UNORM8_BINFMT:
        return 3 * 1;

    case COLOR3INT16_BINFMT:
        return 3 * 2;

    case COLOR4_BINFMT:
        return 4 * 4;

    case COLOR4UNORM8_BINFMT:
        return 4 * 1;

    case COLOR4INT16_BINFMT:
        return 4 * 2;

    default:
        return -1;
    }
}
}
