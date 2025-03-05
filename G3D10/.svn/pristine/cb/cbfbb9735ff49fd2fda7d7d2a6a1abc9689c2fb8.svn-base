/**
  \file G3D-app.lib/include/G3D-app/Renderer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_Renderer_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/Array.h"

namespace G3D {

class RenderDevice;
class Framebuffer;
class GBuffer;
class Surface;
class Camera;
class LightingEnvironment;
class SceneVisualizationSettings;
class RenderPassType;
class Rect2D;
class TriTree;

/** \brief Base class for 3D rendering pipelines. 
    \sa GApp::onGraphics3D */
class Renderer : public ReferenceCountedObject {
protected:

    enum Order { 
        /** Good for early depth culling */
        FRONT_TO_BACK,
        
        /** Good for painter's algorithm sorted transparency */
        BACK_TO_FRONT,

        /** Allows minimizing state changes by batching primitives */
        ARBITRARY 
    };
    
    /** For VR. Default is false. */
    bool                        m_diskFramebuffer = false;
    
    /**
     \brief Appends to \a sortedVisibleSurfaces and \a forwardSurfaces.

     \param sortedVisibleSurfaces All surfaces visible to the GBuffer::camera(), sorted from back to front

     \param forwardOpaqueSurfaces Surfaces for which Surface::canBeFullyRepresentedInGBuffer() returned false.
     These require a forward pass in a deferred shader.
     (They may be capable of deferred shading for <i>some</i> pixels covered, e.g.,
      if the GBuffer did not contain a sufficient emissive channel.)

     \param forwardBlendedSurfaces Surfaces that returned true Surface::hasBlendedTransparency()
     because they require per-pixel blending at some locations 
    */
    virtual void cullAndSort
       (const shared_ptr<Camera>&           camera,
        const shared_ptr<GBuffer>&          gbuffer,
        const Rect2D&                       viewport,
        const Array<shared_ptr<Surface>>&   allSurfaces, 
        Array<shared_ptr<Surface>>&         sortedVisibleSurfaces, 
        Array<shared_ptr<Surface>>&         forwardOpaqueSurfaces,
        Array<shared_ptr<Surface>>&         forwardBlendedSurfaces);

    /** 
       \brief Render z-prepass, depth peel, and G-buffer. 
       Called from render().

       \param gbuffer Must already have had GBuffer::prepare() called
       \param depthPeelFramebuffer Only rendered if notNull()
    */
    virtual void computeGBuffer
       (RenderDevice*                       rd,
        const Array<shared_ptr<Surface>>&   sortedVisibleSurfaces,
        const shared_ptr<GBuffer>&          gbuffer,
        const shared_ptr<Framebuffer>&      depthPeelFramebuffer,
        float                               depthPeelSeparationHint);

    /** \brief Compute ambient occlusion and direct illumination shadow maps. */
    virtual void computeShadowing
       (RenderDevice*                       rd,
        const Array<shared_ptr<Surface>>&   allSurfaces, 
        const shared_ptr<GBuffer>&          gbuffer,
        const shared_ptr<Framebuffer>&      depthPeelFramebuffer,
        LightingEnvironment&                lightingEnvironment);

    /** \brief Forward shade everything in \a surfaceArray.
        Called from render() 
        
        \param surfaceArray Visible surfaces sorted from front to back

      */
    virtual void forwardShade
       (RenderDevice*                       rd, 
        Array<shared_ptr<Surface> >&        surfaceArray, 
        const shared_ptr<GBuffer>&          gbuffer, 
        const LightingEnvironment&          environment,
        const RenderPassType&               renderPassType,
        Order                               order);

public:
    
    void setDiskFramebuffer(bool b) {
        m_diskFramebuffer = b;
    }

    bool diskFramebuffer() const {
        return m_diskFramebuffer;
    }

    virtual const String& className() const = 0;

    /** 
      The active camera and time interval are taken from the GBuffer.

      \param framebuffer Target color and depth framebuffer. Will be rendered in high dynamic range (HDR) linear radiance.
      \param gbuffer Must be allocated, sized, and prepared. Will be rendered according to its specification by this method.
      \param allSurfaces Surfaces not visible to the camera will automatically be culled
      \param depthPeelFramebuffer May be nullptr
      \param lightingEnvironment Shadow maps will be updated for any lights that require them. AO will be updated if the
             ambientOcclusion field is non-nullptr. Screen-space color buffer will be updated with textures the next frame.
      \param tritree The BVH for ray tracing queries.
    */
    virtual void render
       (RenderDevice*                       rd, 
        const shared_ptr<Camera>&           camera,
        const shared_ptr<Framebuffer>&      framebuffer,
        const shared_ptr<Framebuffer>&      depthPeelFramebuffer,
        LightingEnvironment&                lightingEnvironment,
        const shared_ptr<GBuffer>&          gbuffer, 
        const Array<shared_ptr<Surface>>&   allSurfaces,
        // std::function<return-value(arguments)>
        // This argument is a reference to a const function that takes no arguments and returns a reference to a const shared_ptr<TriTree>
        const std::function<const shared_ptr<TriTree>& ()>& tritreeFunction = nullptr) = 0;
};

} // namespace G3D
