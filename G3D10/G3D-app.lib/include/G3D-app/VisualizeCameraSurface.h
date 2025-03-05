/**
  \file G3D-app.lib/include/G3D-app/VisualizeCameraSurface.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef GLG3D_VisualizeCameraSurface_h
#define GLG3D_VisualizeCameraSurface_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-app/Surface.h"

namespace G3D {

class Camera;

/** 
  Displays a 3D representation of a Camera.

  Intended for debugging.
 */
class VisualizeCameraSurface : public Surface {
protected:
    shared_ptr<Camera>  m_camera;

    VisualizeCameraSurface(const shared_ptr<Camera>& c);

public:

    virtual void setStorage(ImageStorage newStorage) override {}
    static shared_ptr<VisualizeCameraSurface> create(const shared_ptr<Camera>& c);

    virtual String name() const override;

    virtual TransparencyType transparencyType() const override {
        return TransparencyType::NONE;
    }

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

};

} // G3D

#endif
