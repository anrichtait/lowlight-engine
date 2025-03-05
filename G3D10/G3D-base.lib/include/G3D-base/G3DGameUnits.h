/**
  \file G3D-base.lib/include/G3D-base/G3DGameUnits.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_G3DGameUnits_h
#define G3D_G3DGameUnits_h

#include "G3D-base/platform.h"

namespace G3D {

/**
 Time, in seconds in a simulation. Relatively low precision compared to RealTime.
 */
typedef float SimTime;

/**
 Actual wall clock time in seconds (Unix time).
 */
typedef double RealTime;

enum AMPM {AM, PM};

/** \deprecated */
enum {SECOND=1, MINUTE=60, HOUR = 60*60, DAY=24*60*60, SUNRISE=24*60*60/4, SUNSET=24*60*60*3/4, MIDNIGHT=0, METER=1, KILOMETER=1000};

/**
 Converts a 12 hour clock time into the number of seconds since 
 midnight.  Note that 12:00 PM is noon and 12:00 AM is midnight.

 Example: <CODE>toSeconds(10, 00, AM)</CODE>
 */
SimTime toSeconds(int hour, int minute, double seconds, AMPM ap);
SimTime toSeconds(int hour, int minute, AMPM ap);

}

#endif
