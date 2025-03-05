/**
  \file G3D-base.lib/include/G3D-base/PhysicsFrameSpline.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_PhysicsFrameSpline_h
#define G3D_PhysicsFrameSpline_h

#include "G3D-base/platform.h"
#include "G3D-base/PhysicsFrame.h"
#include "G3D-base/Spline.h"

namespace G3D {

/**
 A subclass of Spline that keeps the rotation field of a
 PhysicsFrame normalized and rotating the short direction.

 \sa UprightFrameSpline
 */
class PhysicsFrameSpline : public Spline<PhysicsFrame> {
public:

    PhysicsFrameSpline();

    /** Accepts a table of properties, or any valid PhysicsFrame specification for a single control*/
    PhysicsFrameSpline(const Any& any);

    bool operator==(const PhysicsFrameSpline& a) const;
 
    bool operator!=(const PhysicsFrameSpline& a) const {
        return ! ((*this) == a);
    }

    /** Mutates all underlying PhysicsFrames by scaling their translation by \param scaleFactor */
    void scaleControlPoints(float scaleFactor);

    virtual void correct(PhysicsFrame& frame) const override;
    virtual void ensureShortestPath(PhysicsFrame* A, int N) const override;

    virtual Any toAny(const String& myName) const override {
        return Spline<PhysicsFrame>::toAny(myName);
    }

    Any toAny() const {
        return toAny("PFrameSpline");
    }
};

}

#endif
