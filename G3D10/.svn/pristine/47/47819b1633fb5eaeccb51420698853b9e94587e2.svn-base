/**
  \file G3D-app.lib/include/G3D-app/FogVolumeSurface.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_FogVolumeSurface_h
#include "G3D-base/platform.h"
#include "G3D-base/AABox.h"
#include "G3D-base/Sphere.h"
#include "G3D-app/Surface.h"

namespace G3D {

/** 
   An *infinite* fog volume, implemented by drawing at the
   near plane and ray marching until it hits the depth buffer.

   A general implementation for a finite volume would draw at the
   closest point on the volume that is not past the near plane,
   in order to get the benefit of early depth testing.

   This is not optimized...it just brute-force marches through
   the volume.
   
 */
class FogVolumeSurface : public Surface {
protected:

    FogVolumeSurface() {
        m_preferLowResolutionTransparency = true;
    }

public:

    static shared_ptr<FogVolumeSurface> create();


    /** FogVolumeSurface can't convert its special material to anything other than the GPU, so it just ignores this right now. */
    virtual void setStorage(ImageStorage newStorage) override {}

    virtual TransparencyType transparencyType() const override {
        return TransparencyType::ALL;
    }

    virtual bool canBeFullyRepresentedInGBuffer(const GBuffer::Specification &specification) const override {
        return false;
    }

    virtual bool canRenderIntoSVO() const override {
        return false;
    }

    virtual bool hasTransmission() const override {
        return true;
    }
    
    virtual void getObjectSpaceBoundingBox(AABox& box, bool previous = false) const override {
        box = AABox::inf();
    }

    virtual void getObjectSpaceBoundingSphere(Sphere& sphere, bool previous = false) const override {
        sphere.radius = finf();
        sphere.center = Point3::zero();
    }

    virtual void render
       (RenderDevice*                       rd,
        const LightingEnvironment&          environment,
        RenderPassType                      passType) const override;

    /** Does nothing, since this casts no shadows and renders no depth */
    virtual void renderDepthOnlyHomogeneous
        (RenderDevice*                      rd, 
         const Array<shared_ptr<Surface> >& surfaceArray,
         const shared_ptr<Texture>&         depthPeelTexture,
         const float                        depthPeelEpsilon,
         TransparencyTestMode               transparencyTestMode,
        const Color3&                       transmissionWeight) const override {};

    /** Does nothing */
    virtual void renderWireframeHomogeneous 
       (RenderDevice*                       rd,
        const Array<shared_ptr<Surface>>&   surfaceArray,
        const Color4&                       color,
        bool                                previous) const override {}

};

}
