/**
  \file G3D-app.lib/include/G3D-app/PointSurface.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once

#include "G3D-app/PointModel.h"
#include "G3D-base/Sphere.h"

namespace G3D {

class PointModel;

/**
  A G3D::Surface for point splatting. The current implementation forces forward rendering
  and refuses to write to a depth-only or GBuffer for performance (to avoid making two passes
  when deferred shading is enabled).

  A future implementation may support deferred shading.
 */
class PointSurface : public Surface {
public:
    static shared_ptr<PointSurface> create
       (const String&                       name,
        const CFrame&                       frame,
        const CFrame&                       previousFrame,
        const shared_ptr<PointModel::PointArray>& pointArray,
        const shared_ptr<PointModel>&       model,
        const shared_ptr<Entity>&           entity,
        const Surface::ExpressiveLightScatteringProperties& expressive);

    virtual TransparencyType transparencyType() const override {
        return TransparencyType::SOME;
    }

    virtual bool canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const override {
        return true;
    }


    virtual CoordinateFrame frame(bool previous = false) const override {
        if (previous) {
            return m_previousFrame;
        }
        else {
            return m_frame;
        }
    }

    virtual void render(RenderDevice *rd, const LightingEnvironment &environment, RenderPassType passType) const override;

    virtual void setStorage(ImageStorage newStorage) override {}

    virtual void renderDepthOnlyHomogeneous
    (RenderDevice*                      rd,
     const Array<shared_ptr<Surface> >& surfaceArray,
     const shared_ptr<Texture>&         previousDepthBuffer,
     const float                        minZSeparation,
     TransparencyTestMode               transparencyTestMode,
     const Color3&                      transmissionWeight) const override;

    virtual void renderIntoGBufferHomogeneous
    (RenderDevice *rd,
     const Array< shared_ptr< Surface > > &surfaceArray,
     const shared_ptr< GBuffer > &gbuffer,
     const shared_ptr< Texture > &depthPeelTexture,
     const float minZSeparation,
     const LightingEnvironment &lighting) const override;

    virtual void bindDepthPeelArgs
    (Args& args,
     RenderDevice* rd,
     const shared_ptr<Texture>& depthPeelTexture,
     const float minZSeparation) const;
    
    /** Intentionally does nothing */
    virtual void renderIntoSVOHomogeneous(RenderDevice* rd, Array< shared_ptr< Surface > > &surfaceArray, const shared_ptr< SVO > &svo, const CFrame &previousCameraFrame) const override {}

    /** Intentionally does nothing */
    virtual void renderWireframeHomogeneous(RenderDevice* rd, const Array< shared_ptr< Surface > > &surfaceArray, const Color4 &color, bool previous) const override {}

    virtual void getObjectSpaceBoundingBox(AABox&, bool) const override;

    virtual void getObjectSpaceBoundingSphere(Sphere&, bool) const override;

protected:

    enum DepthPassType {
        FIXED_FUNCTION_NO_ALPHA,
        FIXED_FUNCTION_ALPHA,
        PARALLAX_AND_ALPHA
    };


    const String                                m_name;
    const CFrame                                m_frame;
    const CFrame                                m_previousFrame;
    const shared_ptr<PointModel::PointArray>    m_pointArray;
    const shared_ptr<PointModel>                m_pointModel;

    const String                                m_profilerHint;
    int                                         m_renderAsDisk;     

    PointSurface
    (const String&                                          name,
        const CFrame&                                       frame,
        const CFrame&                                       previousFrame,
        const shared_ptr<PointModel::PointArray>&           pointArray,
        const shared_ptr<PointModel>&                       model,
        const shared_ptr<Entity>&                           entity,
        const Surface::ExpressiveLightScatteringProperties& expressive);

};

}
