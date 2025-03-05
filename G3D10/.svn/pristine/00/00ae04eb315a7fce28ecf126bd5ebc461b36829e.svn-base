/**
  \file G3D-app.lib/source/Renderer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/typeutils.h"
#include "G3D-app/Renderer.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Framebuffer.h"
#include "G3D-app/LightingEnvironment.h"
#include "G3D-app/Camera.h"
#include "G3D-app/Surface.h"
#include "G3D-app/AmbientOcclusion.h"
#include "G3D-app/SkyboxSurface.h"
#include "G3D-app/Light.h"

namespace G3D {

void Renderer::computeGBuffer
   (RenderDevice*                       rd,
    const Array<shared_ptr<Surface>>&   sortedVisibleSurfaces,
    const shared_ptr<GBuffer>&          gbuffer,
    const shared_ptr<Framebuffer>&      depthPeelFramebuffer,
    float                               depthPeelSeparationHint) {

    BEGIN_PROFILER_EVENT("Renderer::computeGBuffer");
    //const shared_ptr<Camera>& camera = gbuffer->camera();
    
    Surface::renderIntoGBuffer(rd, sortedVisibleSurfaces, gbuffer);
    /* // TODO
    if (notNull(depthPeelFramebuffer)) {
        rd->pushState(depthPeelFramebuffer); {
            rd->clear();
            rd->setProjectionAndCameraMatrix(camera->projection(), camera->frame());
            Surface::renderDepthOnly(rd, sortedVisibleSurfaces, CullFace::BACK, gbuffer->texture(GBuffer::Field::DEPTH_AND_STENCIL), depthPeelSeparationHint, TransparencyTestMode::REJECT_TRANSPARENCY, Color3::white() / 3.0f);
        } rd->popState();
    }*/
    END_PROFILER_EVENT();
}


void Renderer::computeShadowing
   (RenderDevice*                       rd,
    const Array<shared_ptr<Surface>>&   allSurfaces, 
    const shared_ptr<GBuffer>&          gbuffer,
    const shared_ptr<Framebuffer>&      depthPeelFramebuffer,
    LightingEnvironment&                lightingEnvironment) {

    BEGIN_PROFILER_EVENT("Renderer::computeShadowing");
    Light::renderShadowMaps(rd, lightingEnvironment.lightArray, allSurfaces);

    // Compute AO
    if (notNull(lightingEnvironment.ambientOcclusion) && lightingEnvironment.ambientOcclusionSettings.enabled) {
        lightingEnvironment.ambientOcclusion->update
           (rd, 
            lightingEnvironment.ambientOcclusionSettings,
            gbuffer->camera(), gbuffer->texture(GBuffer::Field::DEPTH_AND_STENCIL), 
            notNull(depthPeelFramebuffer) ? depthPeelFramebuffer->texture(Framebuffer::DEPTH) : shared_ptr<Texture>(), 
            gbuffer->texture(GBuffer::Field::CS_NORMAL), 
            gbuffer->texture(GBuffer::Field::SS_POSITION_CHANGE), 
            gbuffer->depthGuardBandThickness() - gbuffer->colorGuardBandThickness());
    }
    END_PROFILER_EVENT();
}


void Renderer::cullAndSort
   (const shared_ptr<Camera>&           camera,
    const shared_ptr<GBuffer>&          gbuffer,
    const Rect2D&                       viewport,
    const Array<shared_ptr<Surface>>&   allSurfaces, 
    Array<shared_ptr<Surface>>&         allVisibleSurfaces,
    Array<shared_ptr<Surface>>&         forwardOpaqueSurfaces,
    Array<shared_ptr<Surface>>&         forwardBlendedSurfaces) {

    BEGIN_PROFILER_EVENT("Renderer::cullAndSort");
    Surface::cull(camera->frame(), camera->projection(), viewport, allSurfaces, allVisibleSurfaces);

    Surface::sortBackToFront(allVisibleSurfaces, camera->frame().lookVector());

    // Extract everything that uses a forward rendering pass (including the skybox, which is emissive
    // and benefits from a forward pass because it may have high dynamic range). Leave the skybox in the
    // deferred pass to produce correct motion vectors as well.
    for (int i = 0; i < allVisibleSurfaces.size(); ++i) {  
        const shared_ptr<Surface>& surface = allVisibleSurfaces[i];

        if (isNull(gbuffer) || ! surface->canBeFullyRepresentedInGBuffer(gbuffer->specification())) {
            if (surface->transparencyType() != TransparencyType::NONE) {
                forwardBlendedSurfaces.append(surface);
            }

            if (surface->transparencyType() != TransparencyType::ALL) {
                forwardOpaqueSurfaces.append(surface);
            }
        }
    }
    END_PROFILER_EVENT();
}


void Renderer::forwardShade
   (RenderDevice*                   rd, 
    Array<shared_ptr<Surface> >&    surfaceArray, 
    const shared_ptr<GBuffer>&      gbuffer, 
    const LightingEnvironment&      environment,
    const RenderPassType&           renderPassType, 
    Order                           order) {

    debugAssertM(surfaceArray.size() < 500000, "It is very unlikely that you intended to draw 500k surfaces. There is probably heap corruption.");

    const Vector2int16 trimBand = gbuffer->trimBandThickness();
    if (notNull(gbuffer) && ! trimBand.isZero()) {
        rd->pushState();
        rd->setGuardBandClip2D(trimBand);
    }

    rd->setDepthWrite(renderPassType == RenderPassType::OPAQUE_SAMPLES);

    switch (order) {
    case FRONT_TO_BACK:
        // Render in the reverse order
        for (int i = surfaceArray.size() - 1; i >= 0; --i) {
            surfaceArray[i]->render(rd, environment, renderPassType);
        }
        break;

    case BACK_TO_FRONT:
        // Render in the provided order
        for (int i = 0; i < surfaceArray.size(); ++i) {
            surfaceArray[i]->render(rd, environment, renderPassType);
        }
        break;

    case ARBITRARY:
        {
            // Separate by type.  This preserves the sort order and ensures that the closest
            // object will still render first.
            Array< Array<shared_ptr<Surface> > > derivedTable;
            categorizeByDerivedType(surfaceArray, derivedTable);

            const Array<shared_ptr<Surface>>* skyboxArray = nullptr;

            for (int t = 0; t < derivedTable.size(); ++t) {
                const Array<shared_ptr<Surface> >& derivedArray = derivedTable[t];
                debugAssertM(derivedArray.size() > 0, "categorizeByDerivedType produced an empty subarray");

                if ((derivedArray.size() > 0) && derivedArray[0]->isSkybox() && isNull(skyboxArray)) {
                    skyboxArray = &derivedArray;
                } else {
                    derivedArray[0]->renderHomogeneous(rd, derivedArray, environment, renderPassType);
                }
            } // for each derived type

            if (notNull(skyboxArray)) {
                (*skyboxArray)[0]->renderHomogeneous(rd, *skyboxArray, environment, renderPassType);
            }
            break;
        } // ARBITRARY
    } // switch

    if (notNull(gbuffer) && ! trimBand.isZero()) {
        rd->popState();
    }

}

} // namespace G3D
