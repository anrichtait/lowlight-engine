/**
  \file G3D-app.lib/include/G3D-app/ParticleSurface.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_ParticleSurface_h

#include "G3D-base/platform.h"
#include "G3D-app/Surface.h"
#include "G3D-app/ParticleSystem.h"

namespace G3D {

/**
    Each ParticleSurface is the set of particles for a single ParticleSystem (not a single particle--this allows them
    to be culled reasonably without creating a huge amount of CPU work managing the particles).
      
    All particles for all ParticleSystems are submitted as a single draw call. 
      
    In sorted transparency mode, the ParticleSurface sorts for each draw call. In OIT mode, there is no CPU
    work per draw call (however, there may be necessary copying during pose for CPU-animated particles).
  
*/
class ParticleSurface : public Surface {
protected:
    friend ParticleSystem;

    /** This is a POINTER to a block so that in the event of reallocation, the Surface
        will still know where to find its data. */
    shared_ptr<ParticleSystem::Block>   m_block;
    AABox                               m_objectSpaceBoxBounds;
    Sphere                              m_objectSpaceSphereBounds;

    ParticleSurface() {}
    ParticleSurface(const shared_ptr<Entity>& entity) {
        m_entity = entity;
    }

    // To be called by ParticleSystem only
    static shared_ptr<ParticleSurface> create(const shared_ptr<Entity>& entity) {
        return createShared<ParticleSurface>(entity);
    }

    static void sortAndUploadIndices(const shared_ptr<ParticleSurface>& surface, const Vector3& csz);

    /** If \param sort is true, construct an index array to render back-to-front (using \param csz), otherwise submit everything in a giant multi-draw call */
    static void setShaderArgs(Args & args, const Array<shared_ptr<Surface>>& surfaceArray, const bool sort, const Vector3& csz);

public:

    /** ParticleSurface can't convert its special material to anything other than the GPU, so it just ignores this right now. */
    virtual void setStorage(ImageStorage newStorage) override {}

    /** ParticleSystem is defined to act entirely transparently */
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
        // Conservatively enabled
        return true;
    }

    /** May be infinite */
    virtual void getObjectSpaceBoundingBox(AABox& box, bool previous = false) const override;

    /** May be infinite */
    virtual void getObjectSpaceBoundingSphere(Sphere& sphere, bool previous = false) const override;

    virtual void render
       (RenderDevice*                           rd,
        const LightingEnvironment&              environment,
        RenderPassType                          passType) const override;

    virtual void renderDepthOnlyHomogeneous
       (RenderDevice*                           rd,
        const Array< shared_ptr< Surface > > &  surfaceArray,
        const shared_ptr< Texture > &           depthPeelTexture,
        const float                             depthPeelEpsilon,
        TransparencyTestMode                    transparencyTestMode,
        const Color3&                           transmissionWeight) const override;

    virtual void renderHomogeneous
       (RenderDevice*                           rd, 
        const Array< shared_ptr< Surface > >&   surfaceArray, 
        const LightingEnvironment&              lightingEnvironment, 
        RenderPassType                          passType) const override;

    virtual void renderWireframeHomogeneous 
       (RenderDevice*                           rd,
        const Array< shared_ptr< Surface > >&   surfaceArray,
        const Color4 &                          color,
        bool                                    previous) const override;

};

} // namespace
