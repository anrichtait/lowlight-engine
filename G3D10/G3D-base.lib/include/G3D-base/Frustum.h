/**
  \file G3D-base.lib/include/G3D-base/Frustum.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_Frustum_h
#define G3D_Frustum_h

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Plane.h"
#include "G3D-base/SmallArray.h"
#include "G3D-base/Vector4.h"

namespace G3D {

class Box;

/** \see Projection */
class Frustum {
public:
    class Face {
    public:
        /** Counter clockwise indices into vertexPos */
        int             vertexIndex[4];

        /** The plane containing the face. */
        Plane           plane;
    };
        
    /** The vertices, in homogeneous space.  The order is that of
        the near face, starting from the (object space) +x,+y corner
        and proceeding CCW from the camera's point of view; followed
        by the far face also in CCW order.
    
        If w == 0,
        a vertex is at infinity. */
    SmallArray<Vector4, 8>      vertexPos;

    /** The faces in the frustum.  When the
        far plane is at infinity, there are 5 faces,
        otherwise there are 6.  The faces are in the order
        N,R,L,B,T,[F].
        */
    SmallArray<Face, 6>         faceArray;

    /** The planes in the frustum */
    void getPlanes(Array<Plane>& plane) const;

    /** \param minObjectSpaceDepth Smallest value permitted for the near plane Z - far plane Z (e.g., to force finite bounds)*/
    Box boundingBox(float minObjectSpaceDepth = finf()) const;
};

} // namespace G3D

#endif
