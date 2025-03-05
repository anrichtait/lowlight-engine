/**
  \file G3D-base.lib/include/G3D-base/Intersect.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_Intersect
#define G3D_Intersect

#include "G3D-base/platform.h"

namespace G3D {

class Ray;
class PrecomputedRay;
class AABox;

/**
 @beta
 */
class Intersect {
public:

    /** \brief Returns true if the intersection of the ray and the solid box is non-empty. 

      \cite "Fast Ray / Axis-Aligned Bounding Box Overlap Tests using Ray Slopes" 
      by Martin Eisemann, Thorsten Grosch, Stefan Mueller and Marcus Magnor
      Computer Graphics Lab, TU Braunschweig, Germany and
      University of Koblenz-Landau, Germany
    */
    static bool rayAABox(const PrecomputedRay& ray, const AABox& box);

    /** \brief Returns true if the intersection of the ray and the solid box is non-empty. 
     
     \param time If there is an intersection, set to the time to that intersection. If the ray origin is inside the box,
     this is a negative value indicating the distance backwards from the ray origin to the first intersection.
     \a time is not set if there is no intersection.
          
     \cite Slope-Mul method from "Fast Ray / Axis-Aligned Bounding Box Overlap Tests using Ray Slopes" 
      by Martin Eisemann, Thorsten Grosch, Stefan Mueller and Marcus Magnor
      Computer Graphics Lab, TU Braunschweig, Germany and
      University of Koblenz-Landau, Germany
      */
     static bool rayAABox(const PrecomputedRay& ray, const AABox& box, float& time);
};

}

#endif
