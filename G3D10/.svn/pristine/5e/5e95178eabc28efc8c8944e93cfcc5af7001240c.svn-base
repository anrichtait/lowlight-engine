/**
  \file G3D-base.lib/include/G3D-base/Vector3uint8.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once

#include "G3D-base/platform.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/format.h"
#include "G3D-base/HashTrait.h"

namespace G3D {

/**
 \class Vector2uint32 
 A Vector2 that packs its fields into uint8.
 */
G3D_BEGIN_PACKED_CLASS(1)
Vector3uint8 {
private:
    // Hidden operators
    bool operator<(const Vector3uint8&) const;
    bool operator>(const Vector3uint8&) const;
    bool operator<=(const Vector3uint8&) const;
    bool operator>=(const Vector3uint8&) const;

public:
    G3D::uint8              x;
    G3D::uint8              y;
    G3D::uint8              z;

    Vector3uint8() : x(0), y(0) {}
    Vector3uint8(G3D::uint8 _x, G3D::uint8 _y, G3D::uint8 _z) : x(_x), y(_y), z(_z) {}


    inline G3D::uint8& operator[] (int i) {
        debugAssert(((unsigned int)i) <= 2);
        return ((G3D::uint8*)this)[i];
    }

    inline const G3D::uint8& operator[] (int i) const {
        debugAssert(((unsigned int)i) <= 2);
        return ((G3D::uint8*)this)[i];
    }

    inline Vector3uint8 operator+(const Vector3uint8& other) const {
        return Vector3uint8(x + other.x, y + other.y, z + other.z);
    }

    inline Vector3uint8 operator-(const Vector3uint8& other) const {
        return Vector3uint8(x - other.x, y - other.y, z - other.z);
    }

    inline Vector3uint8 operator*(const Vector3uint8& other) const {
        return Vector3uint8(x * other.x, y * other.y, z * other.z);
    }

    inline Vector3uint8 operator*(const int s) const {
        return Vector3uint8(x * s, y * s, z * s);
    }

    inline Vector3uint8& operator+=(const Vector3uint8& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    /** Shifts x, y, and z */
    inline Vector3uint8 operator>>(const int s) const {
        return Vector3uint8(x >> s, y >> s, z >> s);
    }

    /** Shifts x, y, and z */
    inline Vector3uint8 operator<<(const int s) const {
        return Vector3uint8(x << s, y << s, z << s);
    }

    inline Vector3uint8& operator-=(const Vector3uint8& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    inline Vector3uint8& operator*=(const Vector3uint8& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    inline bool operator==(const Vector3uint8& other) const {
        return (x == other.x) && (y == other.y) && (z == other.z);
    }

    inline bool operator!= (const Vector3uint8& other) const {
        return !(*this == other);
    }

    Vector3uint8 max(const Vector3uint8& v) const {
        return Vector3uint8(G3D::max(x, v.x), G3D::max(y, v.y), G3D::max(z, v.z));
    }

    Vector3uint8 min(const Vector3uint8& v) const {
        return Vector3uint8(G3D::min(x, v.x), G3D::min(y, v.y), G3D::min(y, v.z));
    }

}
G3D_END_PACKED_CLASS(1)

typedef Vector3uint8 Point3uint8;

} // namespace G3D

template<> struct HashTrait<G3D::Vector3uint8> {
    static size_t hashCode(const G3D::Vector3uint8& key) { return static_cast<size_t>(size_t(key.x) | (size_t(key.y) << 8) | (size_t(key.z) << 16)); }
};
