/**
  \file G3D-gfx.lib/include/G3D-gfx/tesselate.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_TESSELATE_H
#define G3D_TESSELATE_H

#include "G3D-base/Vector3.h"
#include "G3D-base/Triangle.h"
#include "G3D-base/Array.h"

namespace G3D {

/**
 Tesselates a complex polygon into a triangle set which is appended
 to the output.  The input is a series of counter-clockwise winding
 vertices, where the last is implicitly connected to the first.
 Self-intersections are allowed; "inside" is determined by an "odd"
 winding rule.  You may need to introduce a sliver polygon to cut
 holes out of the center.
 */
void tesselateComplexPolygon(const Array<Vector3>& input, Array<Triangle>& output);

}

#endif
