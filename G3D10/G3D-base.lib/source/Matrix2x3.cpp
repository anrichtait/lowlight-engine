/**
  \file G3D-base.lib/source/Matrix2x3.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/Matrix2x3.h"
#include "G3D-base/System.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/Any.h"
#include "G3D-base/Vector2.h"
#include "G3D-base/Vector3.h"


namespace G3D {
    
Matrix2x3::Matrix2x3(const float* init) {
    System::memcpy(elt, init, sizeof(float) * 6);
}
    

Matrix2x3::Matrix2x3(const Any& any) {
    any.verifyName("Matrix3");
    any.verifyType(Any::ARRAY);

    any.verifySize(6);

    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 3; ++c) {
            elt[r][c] = any[r * 3 + c];
        }
    }
}


/** Matrix2x3::zero() */
Matrix2x3::Matrix2x3() {
    System::memset(elt, 0, sizeof(float) * 6);
}

bool Matrix2x3::operator!=(const Matrix2x3& other) const {
    return ! (*this == other);
}


bool Matrix2x3::operator==(const Matrix2x3& other) const {

    // If the bit patterns are identical, they must be
    // the same matrix.  They might still differ due 
    // to floating point issues such as -0 == 0.
    if (memcmp(this, &other, sizeof(Matrix2x3)) == 0) {
        return true;
    } 

    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (elt[r][c] != other.elt[r][c]) {
                return false;
            }
        }
    }

    return true;
}


String Matrix2x3::toString() const {
    return G3D::format("[%g, %g, %g; %g, %g %g]", 
            elt[0][0], elt[0][1], elt[0][2],
            elt[1][0], elt[1][1], elt[1][2]);
}


Vector2 Matrix2x3::operator*(const Vector3& v) const {
    return Vector2(
        elt[0][0] * v.x + elt[0][1] * v.y + elt[0][2] * v.z,
        elt[1][0] * v.x + elt[1][1] * v.y + elt[1][2] * v.z);

}


} // namespace


