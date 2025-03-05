/**
  \file G3D-base.lib/include/G3D-base/HaltonSequence.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_HaltonSequence_h
#define G3D_HaltonSequence_h

#include "G3D-base/Vector2int32.h"
#include "G3D-base/Vector2.h"


namespace G3D {

class HaltonSequence {

private:
    Point2int32 m_base;
    Point2int32 m_xCurrent;
    Point2int32 m_yCurrent;
    int m_currentIndex;

public:
    /** The next point in the Halton Sequence */
    void next(Point2& p);

    Point2 next() {
        Point2 p;
        next(p);
        return p;
    }

    /** Throw out the next N terms in the sequence */
    void trash(int n) {
        m_currentIndex += n;       
    }

    void reset() {
        m_xCurrent = Point2int32(1, m_base.x);
        m_yCurrent = Point2int32(1, m_base.y);
    }

    /** To be a true Halton sequence, xBase and yBase must be prime, and not equal */
    HaltonSequence(int xBase, int yBase) : m_base(Point2int32(xBase,yBase)), m_xCurrent(Point2int32(1,xBase)), m_yCurrent(Point2int32(1,yBase)), m_currentIndex(1) {}

    /** Length of primeBases */
    static const int numPrimes = 46;
    static const float primeBases[numPrimes];

    /** Samples the Halton Van der Corput sequence statelessly.
        Base should be prime. Returns a number on [0, 1] */
    static float sample(int index, float base) {
        // https://en.wikipedia.org/wiki/Halton_sequence
        const float invBase = 1.0f / base;
        float result = 0.0f;
        float f = 1.0;
        float i = float(index);

        while (i > 0.0f) {
            f *= invBase;
            result += f * fmodf(i, base);
            i = floor(i * invBase);
        }

        return result;
    }
};

}

#endif
