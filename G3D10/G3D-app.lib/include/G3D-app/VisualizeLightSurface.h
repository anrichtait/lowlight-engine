/**
  \file G3D-app.lib/include/G3D-app/VisualizeLightSurface.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef GLG3D_VisualizeLightSurface_h
#define GLG3D_VisualizeLightSurface_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-app/Surface.h"

namespace G3D {

class Light;

/** 
  Displays a 3D representation of a Light.

  Intended for debugging.
 */
class VisualizeLightSurface : public Surface {
protected:

    /** For shadow map */
    bool                m_showBounds;
    shared_ptr<Light>   m_light;

    VisualizeLightSurface(const shared_ptr<Light>& c, bool showBounds);

public:

    virtual void setStorage(ImageStorage newStorage) override {}

    static shared_ptr<VisualizeLightSurface> create(const shared_ptr<Light>& c, bool showBounds = false);

    virtual bool hasTransmission() const override {
        return false;
    }

    virtual String name() const override;

    virtual void getCoordinateFrame(CoordinateFrame& cframe, bool previous = false) const override;

    virtual void getObjectSpaceBoundingBox(AABox& box, bool previous = false) const override;

    virtual void getObjectSpaceBoundingSphere(Sphere& sphere, bool previous = false) const override;

    virtual void render
    (RenderDevice*                          rd, 
     const LightingEnvironment&             environment,
     RenderPassType                         passType) const override;

    virtual void renderDepthOnlyHomogeneous
    (RenderDevice*                          rd, 
     const Array<shared_ptr<Surface> >&     surfaceArray,
     const shared_ptr<Texture>&             depthPeelTexture,
     const float                            depthPeelEpsilon,
     TransparencyTestMode                   transparencyTestMode,  
     const Color3&                          transmissionWeight) const override;
    
    virtual void renderWireframeHomogeneous
    (RenderDevice*                          rd, 
     const Array<shared_ptr<Surface> >&     surfaceArray, 
     const Color4&                          color,
     bool                                   previous) const override;

    virtual bool canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const override;
    
    virtual TransparencyType transparencyType() const override {
        return TransparencyType::SOME;
    }

};

} // G3D

#endif
