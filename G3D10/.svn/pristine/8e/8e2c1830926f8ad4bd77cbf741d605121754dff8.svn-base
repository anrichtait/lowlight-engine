/**
  \file G3D-app.lib/source/Component.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/Component.h"

namespace G3D {

const ImageFormat* ImageUtils::to8(const ImageFormat* f) {
    switch (f->code) {
    case ImageFormat::CODE_L32F:
        return ImageFormat::L8();

    case ImageFormat::CODE_R32F:
        return ImageFormat::R8();

    case ImageFormat::CODE_RGB32F:
        return ImageFormat::RGB8();

    case ImageFormat::CODE_RGBA32F:
        return ImageFormat::RGBA8();

    default:
        alwaysAssertM(false, "Unsupported format");
        return f;
    }
}

}
