/**
  \file G3D-base.lib/source/filter.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/filter.h"

namespace G3D {

void gaussian1D(Array<float>& coeff, int N, float std) {
    coeff.resize(N);
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        float x = i - (N - 1) / 2.0f;
        float p = -square(x / std) / 2.0f;
        float y = exp(p);
        coeff[i] = y;
        sum += y;
    }

    for (int i = 0; i < N; ++i) {
        coeff[i] /= sum;
    }   
}


} // namespace
