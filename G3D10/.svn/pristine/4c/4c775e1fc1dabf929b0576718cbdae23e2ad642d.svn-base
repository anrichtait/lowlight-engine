/**
  \file test/tfilter.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D/G3D.h"
#include "testassert.h"
using G3D::uint8;
using G3D::uint32;
using G3D::uint64;

void testfilter() {
    printf("G3D::gaussian1D  ");
    Array<float> coeff;

    gaussian1D(coeff, 5, 0.5f);
    testAssert(fuzzyEq(coeff[0], 0.00026386508f));
    testAssert(fuzzyEq(coeff[1], 0.10645077f));
    testAssert(fuzzyEq(coeff[2], 0.78657067f));
    testAssert(fuzzyEq(coeff[3], 0.10645077f));
    testAssert(fuzzyEq(coeff[4], 0.00026386508f));

    printf("passed\n");
}
