/**
  \file G3D-base.lib/include/G3D-base/Vector4int32.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_Vector4int32_h
#define G3D_Vector4int32_h

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/HashTrait.h"
#include "G3D-base/Crypto.h"
#include "G3D-base/Vector2int32.h"
#include "G3D-base/Vector3int32.h"

namespace G3D {

    class Any;

/**
 \ Vector4int32
 A Vector4 that packs its fields into int32s. Corresponds to OpenGL's ivec4
 */
G3D_BEGIN_PACKED_CLASS(4)
Vector4int32 {
private:

public:
    G3D::int32              x;
    G3D::int32              y;
    G3D::int32              z;
    G3D::int32              w;

    Vector4int32() : x(0), y(0), z(0), w(0) {}
    Vector4int32(int _x, int _y, int _z, int _w) : x(_x), y(_y), z(_z), w(_w) {}
    Vector4int32(const Any& any);
    Any toAny() const;

    /** Rounds to the nearest int */
    explicit Vector4int32(const class Vector4& v);

    static Vector4int32 truncate(const class Vector4& v);

    bool nonZero() const {
        return (x != 0) || (y != 0) || (z != 0) || (w != 0);
    }

    inline G3D::int32& operator[] (int i) {
        debugAssert(i <= 3);
        return ((G3D::int32*)this)[i];
    }

    inline const G3D::int32& operator[] (int i) const {
        debugAssert(i <= 3);
        return ((G3D::int32*)this)[i];
    }

    inline Vector4int32 operator+(const Vector4int32& other) const {
        return Vector4int32(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    inline Vector4int32 operator-(const Vector4int32& other) const {
        return Vector4int32(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    inline Vector4int32 operator*(const Vector4int32& other) const {
        return Vector4int32(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    inline Vector4int32 operator*(const int s) const {
        return Vector4int32(x * s, y * s, z * s, w * s);
    }

    /** Integer division */
    inline Vector4int32 operator/(const Vector4int32& other) const {
        return Vector4int32(x / other.x, y / other.y, z / other.z, w / other.w);
    }

    /** Integer division */
    inline Vector4int32 operator/(const int s) const {
        return Vector4int32(x / s, y / s, z / s, w/s);
    }

    Vector4int32& operator+=(const Vector4int32& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vector4int32& operator-=(const Vector4int32& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    Vector4int32& operator*=(const Vector4int32& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    bool operator== (const Vector4int32& rkVector) const {
        return ( x == rkVector.x && y == rkVector.y && z == rkVector.z && w == rkVector.w );
    }

    bool operator!= (const Vector4int32& rkVector) const {
        return ( x != rkVector.x || y != rkVector.y || z != rkVector.z || w != rkVector.w);
    }

    Vector4int32 max(const Vector4int32& v) const {
        return Vector4int32(G3D::max(x, v.x), G3D::max(y, v.y), G3D::max(z, v.z), G3D::max(w, v.w));
    }

    Vector4int32 min(const Vector4int32& v) const {
        return Vector4int32(G3D::min(x, v.x), G3D::min(y, v.y), G3D::min(z, v.z), G3D::min(w, v.w));
    }

    Vector4int32 operator-() const {
        return Vector4int32(-x, -y, -z, -w);
    }

    String toString() const;

    Vector4int32 operator<<(int i) const {
        return Vector4int32(x << i, y << i, z << i, w << i);
    }

    Vector4int32 operator>>(int i) const {
        return Vector4int32(x >> i, y >> i, z >> i, w >> i);
    }

    Vector4int32 operator>>(const Vector4int32& v) const {
        return Vector4int32(x >> v.x, y >> v.y, z >> v.z, w >> v.w);
    }

    Vector4int32 operator<<(const Vector4int32& v) const {
        return Vector4int32(x << v.x, y << v.y, z << v.z, w << v.w);
    }

    Vector4int32 operator&(int32 i) const {
        return Vector4int32(x & i, y & i, z & i, w & i);
    }

    size_t hashCode() const {
        return G3D::superFastHash(this, sizeof(*this));
    }

    // 2-char swizzles

    Vector2int32 xx() const;
    Vector2int32 yx() const;
    Vector2int32 zx() const;
    Vector2int32 xy() const;
    Vector2int32 yy() const;
    Vector2int32 zy() const;
    Vector2int32 xz() const;
    Vector2int32 yz() const;
    Vector2int32 zz() const;
}
G3D_END_PACKED_CLASS(4)

typedef Vector4int32 Point4int32;

Vector4int32 iFloor(const Vector4&);

} // namespace G3D

template <> struct HashTrait<G3D::Vector4int32> {
    static size_t hashCode(const G3D::Vector4int32& key) {
      return key.hashCode();
        
        //return G3D::Crypto::crc32(&key, sizeof(key));
        /*
        // Mask for the top bit of a uint32
        const G3D::uint32 top = (1UL << 31);
        // Mask for the bottom 10 bits of a uint32
        const G3D::uint32 bot = 0x000003FF;
        return static_cast<size_t>(((key.x & top) | ((key.y & top) >> 1) | ((key.z & top) >> 2)) | 
                                   (((key.x & bot) << 19) ^ ((key.y & bot) << 10) ^ (key.z & bot)));
        */
    }
};

#endif
