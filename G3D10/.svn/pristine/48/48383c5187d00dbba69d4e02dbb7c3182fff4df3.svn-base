/**
  \file G3D-base.lib/source/g3dmath.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/g3dmath.h"
#include <cstdlib>
#include <cstring>
#include "G3D-base/BIN.h"
#include "G3D-base/Random.h"

namespace G3D {

float ignoreFloat;
double ignoreDouble;
int ignoreInt;

float lerpAngle(float a, float b, float t) {
    a = wrap(a, 0, 2.0f * pif());
    b = wrap(b, 0, 2.0f * pif());
    if (b - a > pif()) {
        a += 2.0f * pif();
    }
    return lerp(a, b, t);
}

int roundStochastically(float f) {
    const int   i = iRound(f);
    return i + ((Random::common().uniform() >= (f - float(i))) ? 0 : 1);
}


void hammersleySequence2D(int i, const int N, float& x, float& y) {
    x = float(i) * (1.0f / float(N));

    i = (i << 16u) | (i >> 16u);
    i = ((i & 0x55555555u) << 1u) | ((i & 0xAAAAAAAAu) >> 1u);
    i = ((i & 0x33333333u) << 2u) | ((i & 0xCCCCCCCCu) >> 2u);
    i = ((i & 0x0F0F0F0Fu) << 4u) | ((i & 0xF0F0F0F0u) >> 4u);
    i = ((i & 0x00FF00FFu) << 8u) | ((i & 0xFF00FF00u) >> 8u);
    y = float(0.5 + double(i) * 2.3283064365386963e-10); // / 0x100000000
}


float gaussRandom(float mean, float stdev) {

    // Using Box-Mueller method from http://www.taygeta.com/random/gaussian.html
    // Modified to specify standard deviation and mean of distribution
    float w, x1, x2;

    // Loop until w is less than 1 so that log(w) is negative
    do {
        x1 = uniformRandom(-1.0, 1.0);
        x2 = uniformRandom(-1.0, 1.0);

        w = float(square(x1) + square(x2));
    } while (w > 1.0f);

    // Transform to gassian distribution
    // Multiply by sigma (stdev ^ 2) and add mean.
    return x2 * (float)square(stdev) * sqrtf((-2.0f * logf(w) ) / w) + mean; 
}

/** 
    This value should not be tested against directly, instead
    G3D::isNan() and G3D::isFinite() will return reliable results. */
double inf() {
    return std::numeric_limits<double>::infinity();
}

// --fast-math breaks other methods of testing for NaN on g++ 4.x,
// including isnan(x) and !(x == x)

bool isNaN(float x) {
    // Wipe out the sign bit
    const uint32 y = *(uint32*)(&x) & BIN32(01111111,11111111,11111111,11111111);

    // If the remaining number has all of the exponent bits set and atleast one
    // fraction bit set, then it is NaN
    return (y > 0x7F800000);
}

bool isNaN(double x) {
    // Wipe out the sign bit
    const uint64 y = *(uint64*)(&x) &
        ((uint64(BIN32(01111111,11111111,11111111,11111111)) << 32) +
         0xFFFFFFFF);

    // If the remaining number has all of the exponent bits set and atleast one
    // fraction bit set, then it is NaN
    return (y > (uint64(BIN32(01111111,11110000,00000000,00000000)) << 32));
}


/** 
    This value should not be tested against directly, instead
    G3D::isNan() and G3D::isFinite() will return reliable results. */
float finf() {
    return std::numeric_limits<float>::infinity();
}

/** This value should not be tested against directly, instead
    G3D::isNan() and G3D::isFinite() will return reliable results. */
double nan() {
    // double is a standard type and should have quiet NaN
    return std::numeric_limits<double>::quiet_NaN();
}

float fnan() {
    // double is a standard type and should have quiet NaN
    return std::numeric_limits<float>::quiet_NaN();
}


int highestBit(uint32 x) {
    // Binary search.
    int base = 0;

    if (x & 0xffff0000)    {
        base = 16;
        x >>= 16;
    }
    if (x & 0x0000ff00) {
        base += 8;
        x >>= 8;
    }
    if (x & 0x000000f0) {
        base += 4;
        x >>= 4;
    }
    
    static const int lut[] = {-1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3};
    return base + lut[x];
}



}
