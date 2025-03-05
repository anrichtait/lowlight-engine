/**
  \file G3D-app.lib/include/G3D-app/SkyboxSurface.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_SkyboxSurface_h

#include "G3D-base/platform.h"
#include "G3D-app/Surface.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Shader.h"
#include "G3D-gfx/AttributeArray.h"

namespace G3D {

/**
  \brief An infinite cube that simulates the appearance of distant objects in the scene.
 */
class SkyboxSurface : public Surface {
protected:

    static AttributeArray     s_cubeVertices;
    static IndexStream        s_cubeIndices;

    /** At alpha = 0, use m_texture0.  At alpha = 1, use m_texture1 */
    float                     m_alpha;

    /** If the textures are 2D, then they are passed as spherical-coordinate maps */
    shared_ptr<Texture>       m_texture0;
    shared_ptr<Texture>       m_texture1;

    SkyboxSurface(const shared_ptr<Texture>& texture0, const shared_ptr<Texture>& texture1, float alpha);

public:

    virtual bool isSkybox() const override { return true; }

    /** Directly creates a SkyboxSurface from a texture, without going through Skybox.
        If the textures are Texture::DIM_2D, then they are passed as spherical-coordinate maps. Otherwise they are assumed to be cube maps. */
    static shared_ptr<SkyboxSurface> create(const shared_ptr<Texture>& texture0, const shared_ptr<Texture>& texture1 = shared_ptr<Texture>(), float alpha = 0.0f);

    /** SkyboxSurface uses raw textures instead of materials, so they can't be converted to non-GPU formats and this is just ignored. */
    virtual void setStorage(ImageStorage newStorage) override {}

    static void setShaderGeometryArgs(RenderDevice* rd, Args& args);

    virtual TransparencyType transparencyType() const override {
        return TransparencyType::NONE;
    }

    const shared_ptr<Texture>& texture0() const {
        return m_texture0;
    }

    /** May be nullptr */
    const shared_ptr<Texture>& texture1() const {
        return m_texture1;
    }

    float alpha() const {
        return m_alpha;
    }

    virtual bool canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const override;
    
    virtual String name() const override;

    virtual void getCoordinateFrame(CoordinateFrame& cframe, bool previous = false) const override;

    virtual void getObjectSpaceBoundingBox(AABox& box, bool previous = false) const override;

    virtual void getObjectSpaceBoundingSphere(Sphere& sphere, bool previous = false) const override;

    virtual void render
    (RenderDevice*                        rd, 
     const LightingEnvironment&           environment,
     RenderPassType                       passType) const override;
   
    virtual void renderIntoGBufferHomogeneous
    (RenderDevice*                        rd,
     const Array<shared_ptr<Surface> >&   surfaceArray,
     const shared_ptr<GBuffer>&           gbuffer,
     const shared_ptr<Texture>&           depthPeelTexture,
     const float                          minZSeparation,
     const LightingEnvironment&           lightingEnvironment) const override;

    virtual void renderWireframeHomogeneous
    (RenderDevice*                        rd, 
     const Array<shared_ptr<Surface> >&   surfaceArray, 
     const Color4&                        color,
     bool                                 previous) const override;

    virtual void renderDepthOnlyHomogeneous
    (RenderDevice*                        rd, 
     const Array<shared_ptr<Surface> >&   surfaceArray,
     const shared_ptr<Texture>&           depthPeelTexture,
     const float                          depthPeelEpsilon,
     TransparencyTestMode                        transparencyTestMode,  
     const Color3&                        transmissionWeight) const override;

    /** Binds the cube map and alpha arguments for non-Skybox shaders to read the skybox */
    virtual void setShaderArgs(Args& args, const String& prefix = "skybox_") const;

}; // class SkyboxSurface

} // namespace G3D

