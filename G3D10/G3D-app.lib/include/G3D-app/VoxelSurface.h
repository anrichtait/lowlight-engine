/**
  \file G3D-app.lib/include/G3D-app/VoxelSurface.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/


#pragma once

#define GLG3D_VoxelSurface_h

#include "VoxelModel.h"
#include "G3D-app/UniversalSurface.h"
namespace G3D {

class VoxelModel;

/**
  A G3D::Surface for voxel rendering. The current implementation supports only deferred shading.
 */
class VoxelSurface : public Surface {
protected:
   
    const String                                m_name;
    const CFrame                                m_frame;
    const CFrame                                m_previousFrame;
    const shared_ptr<VoxelModel>                m_voxelModel;
    const String                                m_profilerHint;     

    VoxelSurface
       (const String&                                       name,
        const CFrame&                                       frame,
        const CFrame&                                       previousFrame,
        const shared_ptr<VoxelModel>&                       model,
        const shared_ptr<Entity>&                           entity,
        const Surface::ExpressiveLightScatteringProperties& expressive);


    virtual void bindDepthPeelArgs
       (Args&                                   args,
        RenderDevice*                           rd,
        const shared_ptr<Texture>&              depthPeelTexture,
        const float                             minZSeparation) const;

public:

    static shared_ptr<VoxelSurface> create
       (const String&                                      name,
        const CFrame&                                      frame,
        const CFrame&                                      previousFrame,
        const shared_ptr<VoxelModel>&                      model,
        const shared_ptr<Entity>&                          entity,
        const Surface::ExpressiveLightScatteringProperties& expressive);

    virtual TransparencyType transparencyType() const override {
        return TransparencyType::NONE;
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

    /** Renders into depthOnly buffer for shadow mapping */
    //does nothing until forward rendering works
    virtual void renderDepthOnlyHomogeneous
       (RenderDevice*                                   rd,
        const Array<shared_ptr<Surface> >&              surfaceArray,
        const shared_ptr<Texture>&                      previousDepthBuffer,
        const float                                     minZSeparation,
        TransparencyTestMode                            transparencyTestMode,
        const Color3&                                   transmissionWeight) const override;


    /** Renders into GBuffer for deferred shading */
    //does nothing until forward rendering works
    virtual void renderIntoGBufferHomogeneous
       (RenderDevice*                                   rd,
        const Array< shared_ptr< Surface > > &          surfaceArray,
        const shared_ptr< GBuffer > &                   gbuffer,
        const shared_ptr< Texture > &                   depthPeelTexture,
        const float                                     minZSeparation,
        const LightingEnvironment &                     lighting) const override;

    /** Intentionally does nothing */
    virtual void renderIntoSVOHomogeneous(RenderDevice *rd, Array< shared_ptr< Surface > > &surfaceArray, const shared_ptr< SVO > &svo, const CFrame &previousCameraFrame) const override {}

    /** Intentionally does nothing */
    virtual void renderWireframeHomogeneous(RenderDevice *rd, const Array< shared_ptr< Surface > > &surfaceArray, const Color4 &color, bool previous) const override {}

    virtual void getObjectSpaceBoundingBox(G3D::AABox &, bool) const override;

    virtual void getObjectSpaceBoundingSphere(G3D::Sphere &, bool) const override;

};
}//namespace G3D
