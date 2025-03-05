/**
  \file G3D-base.lib/include/G3D-base/Matrix2x3.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#define G3D_Matrix2x3_h

#ifdef _MSC_VER
// Disable conditional expression is constant, which occurs incorrectly on inlined functions
#   pragma warning (push)
#   pragma warning( disable : 4127 )
#endif

#include "G3D-base/platform.h"
#include "G3D-base/DoNotInitialize.h"
#include "G3D-base/debugAssert.h"

namespace G3D {

class Any;

/**
  A 2x3 matrix.  Do not subclass.  Data is initialized to 0 when default constructed.

  \sa G3D::CoordinateFrame, G3D::Matrix3, G3D::Matrix4, G3D::Matrix3x4, G3D::Quat
 */
class Matrix2x3 {
private:

    float elt[2][3];

    /**
      Computes the determinant of the 3x3 matrix that lacks excludeRow
      and excludeCol. 
    */

    // Hidden operators
    bool operator<(const Matrix2x3&) const;
    bool operator>(const Matrix2x3&) const;
    bool operator<=(const Matrix2x3&) const;
    bool operator>=(const Matrix2x3&) const;

public:

    Matrix2x3
       (float r1c1, float r1c2, float r1c3,
        float r2c1, float r2c2, float r2c3) {
        elt[0][0] = r1c1; elt[0][1] = r1c2; elt[0][2] = r1c3;
        elt[1][0] = r2c1; elt[1][1] = r2c2; elt[1][2] = r2c3;
    }


    /** Must be in one of the following forms:
        - Matrix3(#, #, # .... #)
    */
    Matrix2x3(const Any& any);

    Matrix2x3(DoNotInitialize dni) {}

    /**
     init should be <B>row major</B>.
     */
    Matrix2x3(const float* init);
    
    
    /** Matrix2x3::zero() */
    Matrix2x3();

    static const Matrix2x3& zero() {
        static const Matrix2x3 M;
        return M;
    }
        
    inline float* operator[](int r) {
        debugAssert(r >= 0);
        debugAssert(r < 2);
        return (float*)&elt[r];
    }

    inline const float* operator[](int r) const {
        debugAssert(r >= 0);
        debugAssert(r < 2);
        return (const float*)&elt[r];
    } 

    /** Returns a row-major pointer. */
    inline operator float* () {
        return (float*)&elt[0][0];
    }

    inline operator const float* () const {
        return (const float*)&elt[0][0];
    }

    bool operator!=(const Matrix2x3& other) const;
    bool operator==(const Matrix2x3& other) const;

    class Vector2 operator*(const class Vector3& v) const;

    String toString() const;
};

}

#ifdef _MSC_VER
#   pragma warning (pop)
#endif
