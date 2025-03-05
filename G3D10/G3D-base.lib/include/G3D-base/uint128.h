/**
  \file G3D-base.lib/include/G3D-base/uint128.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_UINT128_H
#define G3D_UINT128_H

#include "G3D-base/g3dmath.h"

namespace G3D {

/** Limited functionality 128-bit unsigned integer. This is primarily to support FNV hashing and other
    cryptography applications.  See the GMP library for high-precision C++ math support. */
class uint128 {
public:

    G3D::uint64 hi;
    G3D::uint64 lo;

    uint128(const uint64& lo);

    uint128(const uint64& hi, const uint64& lo);
    
    uint128& operator+=(const uint128& x);
    
    uint128& operator*=(const uint128& x);
    
    uint128& operator^=(const uint128& x);
    
    uint128& operator&=(const uint128& x);
    
    uint128& operator|=(const uint128& x);
    
    bool operator==(const uint128& x);
    
    uint128& operator>>=(const int x);

    uint128& operator<<=(const int x);
    
    uint128 operator&(const uint128& x);
};
}

#endif
