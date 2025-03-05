/**
  \file G3D-base.lib/source/ParseError.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include <inttypes.h>
#include "G3D-base/ParseError.h"
#include "G3D-base/format.h"

namespace G3D {

String ParseError::formatFileInfo() const {
    String s;

    if (line != UNKNOWN) {
        if (character != UNKNOWN) {
            return format("%s:%d(%d): ", filename.c_str(), line, character);
        } else {
            return format("%s:%d: ", filename.c_str(), line);
        }
    } else if (byte != UNKNOWN) {
        return format("%s:(%" PRId64 "): ", filename.c_str(), byte);
    } else if (filename.empty()) {
        return "";
    } else {
        return filename + ": ";
    }
}

}
