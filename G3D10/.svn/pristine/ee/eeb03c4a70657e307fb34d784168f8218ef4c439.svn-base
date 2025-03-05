/**
  \file G3D-base.lib/include/G3D-base/Vector4int16.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef Vector4int16_h
#define Vector4int16_h

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/HashTrait.h"

namespace G3D {

G3D_BEGIN_PACKED_CLASS(2)
Vector4int16 {
private:
    // Hidden operators
    bool operator<(const Vector4int16&) const;
    bool operator>(const Vector4int16&) const;
    bool operator<=(const Vector4int16&) const;
    bool operator>=(const Vector4int16&) const;

public:

    G3D::int16 x, y, z, w;

    Vector4int16() : x(0), y(0), z(0), w(0) {}
    Vector4int16(G3D::int16 _x, G3D::int16 _y, G3D::int16 _z, G3D::int16 _w) : x(_x), y(_y), z(_z), w(_w) {}
    explicit Vector4int16(const class Vector4& v);
    explicit Vector4int16(class BinaryInput& bi);

    void serialize(class BinaryOutput& bo) const;
    void deserialize(class BinaryInput& bi);

    inline G3D::int16& operator[] (int i) {
        debugAssert(i <= 3);
        return ((G3D::int16*)this)[i];
    }

    inline const G3D::int16& operator[] (int i) const {
        debugAssert(i <= 3);
        return ((G3D::int16*)this)[i];
    }

    inline Vector4int16 operator+(const Vector4int16& other) const {
        return Vector4int16(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    inline Vector4int16 operator-(const Vector4int16& other) const {
        return Vector4int16(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    inline Vector4int16 operator*(const Vector4int16& other) const {
        return Vector4int16(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    inline Vector4int16 operator*(const int s) const {
        return Vector4int16(int16(x * s), int16(y * s), int16(z * s), int16(w * s));
    }

    inline Vector4int16& operator+=(const Vector4int16& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    inline Vector4int16& operator-=(const Vector4int16& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    inline Vector4int16& operator*=(const Vector4int16& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    inline bool operator== (const Vector4int16& rkVector) const {
        return ( x == rkVector.x && y == rkVector.y && z == rkVector.z && w == rkVector.w);
    }

    inline bool operator!= (const Vector4int16& rkVector) const {
        return ( x != rkVector.x || y != rkVector.y || z != rkVector.z || w != rkVector.w);
    }

    String toString() const;

} G3D_END_PACKED_CLASS(2)

} // namespace G3D

template <> struct HashTrait<G3D::Vector4int16> {
    static size_t hashCode(const G3D::Vector4int16& vertex) {
        return G3D::wangHash6432Shift(*(G3D::int64*)&vertex);
    }
};



#endif // Vector4int16_h

