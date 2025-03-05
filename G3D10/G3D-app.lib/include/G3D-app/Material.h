/**
  \file G3D-app.lib/include/G3D-app/Material.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/lazy_ptr.h"
#include "G3D-app/Component.h"
#include "G3D-app/Surfel.h"

namespace G3D {

class Surface;
class Tri;
class CPUVertexArray;

/** 
    Base class for materials in G3D, mostly useful as an interface for
    ray tracing since hardware rasterization rendering needs to be specialized
    for each Surface and Material subclass.

  \section lazy_ptr
  Material is a lazy_ptr subclass so that classes using it  mayassociate arbitrary data with UniversalMaterial%s 
  or computing Materials on demand without having to subclass UniversalMaterial itself. 
  
  Subclassing UniversalMaterial is often undesirable because
  that class has complex initialization and data management routines.
  Note that UniversalMaterial itself implements lazy_ptr<UniversalMaterial>, so you can simply use a UniversalMaterial with any API
  (such as Tri) that requires a proxy.  


    \see UniversalMaterial
    
 */
class Material : public ReferenceCountedObject {
public:

    /** Return true if coverageLessThanEqual(1) can ever return true. */
    virtual bool hasPartialCoverage() const = 0;

    /** Returns true if this material has an alpha value less than \a alphaThreshold at texCoord. */
    virtual bool coverageLessThanEqual(const float alphaThreshold, const Point2& texCoord) const = 0;
    virtual void setStorage(ImageStorage s) const = 0;
    virtual const String& name() const = 0;
    virtual void sample(const Tri& tri, float u, float v, int triIndex, const CPUVertexArray& vertexArray, bool backside, shared_ptr<Surfel>& surfel, float du = 0, float dv = 0, bool twoSided = true) const = 0;
};

} // namespace G3D
