/**
  \file G3D-base.lib/source/HaltonSequence.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/HaltonSequence.h"
#ifndef INT_MAX
#include <climits>
#endif
namespace G3D {
const float HaltonSequence::primeBases[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199};

#if 0
    static void advanceSequence(int base, Point2int32& current, int currentStride) {
        ++current.x;
        if (current.x == current.y) {
            debugAssertM(current.x < INT_MAX / base, "Halton Sequence wrapping around");
            current.y *= base;
            current.x = 1;
        } else if (current.x % base == 0) {
            ++current.x;
        }
    }
#endif

    void HaltonSequence::next(Point2& p) {
        p.x = sample(m_currentIndex, float(m_base.x));
        p.y = sample(m_currentIndex, float(m_base.y));
        ++m_currentIndex;
#if 0
        p.x = ((float)m_xCurrent.x) / m_xCurrent.y;
        p.y = ((float)m_yCurrent.x) / m_yCurrent.y;
        advanceSequence(m_base.x, m_xCurrent, m_xCurrentStride);
        advanceSequence(m_base.y, m_yCurrent, m_yCurrentStride);
#endif
    }
}
