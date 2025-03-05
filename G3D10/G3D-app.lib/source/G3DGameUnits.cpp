/**
  \file G3D-app.lib/source/G3DGameUnits.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/G3DGameUnits.h"

namespace G3D {

SimTime toSeconds(int hour, int minute, AMPM ap) {
    return toSeconds(hour, minute, 0, ap);
}

SimTime toSeconds(int hour, int minute, double seconds, AMPM ap) {
    double t = ((hour % 12) * 60 + minute) * 60 + seconds;

    if (ap == PM) {
        t += 12 * 60 * 60;
    }

    return SimTime(t);
}

}

