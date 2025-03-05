/**
  \file G3D-base.lib/source/constants.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/constants.h"
#include "G3D-base/Any.h"
#include "G3D-base/stringutils.h"

namespace G3D {

// TODO: Remove February 2014, used for String benchmark
const char* libraryString = "libraryString";

const char* PrimitiveType::toString(int i, Value& v) {
    static const char* str[] = {"POINTS", "LINES", "LINE_STRIP", "TRIANGLES", "TRIANGLE_FAN", "QUADS", "QUAD_STRIP", nullptr}; 
    static const Value val[] = {POINTS, LINES, LINE_STRIP, TRIANGLES, TRIANGLE_FAN, QUADS, QUAD_STRIP};
    const char* s = str[i];
    if (s) {
        v = val[i];
    }
    return s; 
}

} // G3D
