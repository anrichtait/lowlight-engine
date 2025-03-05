/**
  \file G3D-base.lib/source/Rect2D.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/Rect2D.h"
#include "G3D-base/Any.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"

namespace G3D {

const Rect2D& Rect2D::empty() {
    static Rect2D r;
    return r;
}


void Rect2D::serialize(class BinaryOutput& b) const {
    min.serialize(b);
    max.serialize(b);
}

    
void Rect2D::deserialize(class BinaryInput& b) {
    min.deserialize(b);
    max.deserialize(b);
}


/** \param any Must either Rect2D::xywh(#, #, #, #) or Rect2D::xyxy(#, #, #, #)*/
Rect2D::Rect2D(const Any& any) {
    if (any.name() == "Rect2D::empty" || any.name() == "AABox2D::empty") {
        *this = empty();
        return;
    }

    any.verifyName("Rect2D::xyxy", "Rect2D::xywh");
    any.verifyType(Any::ARRAY);
    any.verifySize(4);
    if (any.name() == "Rect2D::xywh") {
        *this = Rect2D::xywh(any[0], any[1], any[2], any[3]);
    } else {
        *this = Rect2D::xyxy(any[0], any[1], any[2], any[3]);
    }
}


/** Converts the Rect2D to an Any. */
Any Rect2D::toAny() const {
    if (isEmpty()) {
        return Any(Any::ARRAY, "Rect2D::empty");
    } else {
        Any any(Any::ARRAY, "Rect2D::xywh");
        any.append(Any(x0()), Any(y0()), Any(width()), Any(height()));
        return any;
    }
}

}
