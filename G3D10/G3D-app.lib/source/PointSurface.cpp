/**
  \file G3D-app.lib/source/PointSurface.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/PointSurface.h"
#include "G3D-app/Entity.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Shader.h"
#include "G3D-base/Projection.h"

namespace G3D {

PointSurface::PointSurface
   (const String&                       name,
    const CFrame&                       frame,
    const CFrame&                       previousFrame,
    const shared_ptr<PointModel::PointArray>& pointArray, 
    const shared_ptr<PointModel>&       model,
    const shared_ptr<Entity>&           entity, 
    const Surface::ExpressiveLightScatteringProperties& expressive) : 
    Surface(expressive),
    m_name(name), 
    m_frame(frame),
    m_previousFrame(previousFrame),
    m_pointArray(pointArray),
    m_pointModel(model),
    m_profilerHint((notNull(entity) ? entity->name() + "/" : "") + (notNull(model) ? model->name() + "/" : "") + m_name) {

    m_model  = model;
    m_entity = entity;

    m_renderAsDisk = model->renderAsDisk() ? 1 : 0;
}


shared_ptr<PointSurface> PointSurface::create
   (const String&                       name,
    const CFrame&                       frame,
    const CFrame&                       previousFrame,
    const shared_ptr<PointModel::PointArray>& pointArray, 
    const shared_ptr<PointModel>&       model, 
    const shared_ptr<Entity>&           entity, 
    const Surface::ExpressiveLightScatteringProperties& expressive) {

    return createShared<PointSurface>(name, frame, previousFrame, pointArray, model, entity, expressive);

}


void PointSurface::render
   (RenderDevice*               rd,
    const LightingEnvironment&  environment,
    RenderPassType              passType) const {

    Args args;

    // Choose LOD based on distance ot the camera
    rd->setObjectToWorldMatrix(m_frame);
    
    // Object to world matrix
    const CFrame& o2w = rd->worldToCameraMatrix() * m_frame;
    
    const Point3& center = o2w.pointToWorldSpace(m_pointArray->sphereBounds.center);

    // Negative value (when not clipped)
    const float nearestZ = min(0.01f, center.z + m_pointArray->sphereBounds.radius);
    
    args.setPrimitiveType(PrimitiveType::POINTS);

    args.setAttributeArray("position", m_pointArray->gpuPosition);
    args.setAttributeArray("emission", m_pointArray->gpuRadiance);

    const bool VERTEX_SHADER_LOD = false;

    //On this branch, we pass LOD > 0 to the shader and have the shader optimize for level of detail
    if (VERTEX_SHADER_LOD) {
        // Choose LOD based on the nearest possible point and apply it in the vertex shader. 
        // This is an attempt to avoid overdraw. 
        // Use magic constants for resolution.
        const int lod = max(0, iFloor(::log2(-nearestZ * 0.015f / m_pointModel->m_pointRadius)));
        args.setMacro("LOD", lod);
        args.setUniform("pointRadius", m_pointModel->m_pointRadius);
    } else {
        //For this branch. always send LOD == 0 to the shader, but only send 1/2^n points, because we know the points are
        //even distributed over the space in the m_pointArray.
        // CPU LOD

        // Choose LOD based on the nearest possible point and apply it on the CPU.
        // This is an attempt to avoid overdraw and VS work.
        // Larger k = lower resolution
        const float k = max(1.0f, sqrtf(-nearestZ * 0.003f / m_pointModel->m_pointRadius));

        args.setMacro("LOD", 0);
        args.setNumIndices(iCeil(m_pointArray->gpuPosition.size() / k));

        // Points on a line would scale like k^1, points covering a square would scale like k^2.
        // We split the difference.
        args.setUniform("pointRadius", m_pointModel->m_pointRadius * pow(k, 1.6f));
    }

    if (m_renderAsDisk) {
        glEnable(GL_POINT_SMOOTH);
    }
    LAUNCH_SHADER("PointSurface_render.*", args);
}


void PointSurface::renderDepthOnlyHomogeneous
    (RenderDevice*                      rd,
     const Array<shared_ptr<Surface> >& surfaceArray,
     const shared_ptr<Texture>&         previousDepthBuffer,
     const float                        minZSeparation,
     TransparencyTestMode               transparencyTestMode,
     const Color3&                      transmissionWeight) const {
   
    debugAssertGLOk();

    rd->pushState(); {

        for (int i = 0; i < surfaceArray.size(); ++i) {
            const shared_ptr<PointSurface>& surface = dynamic_pointer_cast<PointSurface>(surfaceArray[i]);
            debugAssertM(surface, "Surface::renderDepthOnlyHomogeneous passed the wrong subclass");
            Args args;

            args.setPrimitiveType(PrimitiveType::POINTS);
            CFrame cframe;
            surface->getCoordinateFrame(cframe, false);
            rd->setObjectToWorldMatrix(cframe);

            args.setMacro("RENDER_AS_DISK", m_renderAsDisk);
            args.setMacro("LOD", 0);
            args.setUniform("pointRadius", surface->m_pointModel->m_pointRadius);
            args.setAttributeArray("position", surface->m_pointArray->gpuPosition);

            bindDepthPeelArgs(args, rd, previousDepthBuffer, minZSeparation);

            if (notNull(previousDepthBuffer)) {
                LAUNCH_SHADER_WITH_HINT("PointSurface_depthPeel.*", args, surface->m_profilerHint);
            } else {
                LAUNCH_SHADER_WITH_HINT("PointSurface_depthOnly.vrt", args, surface->m_profilerHint);
            }
        }
    } rd->popState();
}


void PointSurface::renderIntoGBufferHomogeneous
    (RenderDevice*                          rd, 
     const Array< shared_ptr< Surface > >&  surfaceArray,
     const shared_ptr< GBuffer >&           gbuffer,
     const shared_ptr< Texture >&           depthPeelTexture,
     const float                            minZSeparation,
     const LightingEnvironment&             lighting) const {
    

    if (m_renderAsDisk) {
        glEnable(GL_POINT_SMOOTH);
    }

    rd->pushState(); {

        // Render front-to-back for early-out Z
        for (int s = surfaceArray.size() - 1; s >= 0; --s) {
            const shared_ptr<PointSurface>& surface = dynamic_pointer_cast<PointSurface>(surfaceArray[s]);

            debugAssertM(notNull(surface),
                "Non PointSurface element of surfaceArray "
                "in PointSurface::renderIntoGBufferHomogeneous");

            Args args;

            args.setPrimitiveType(PrimitiveType::POINTS);
            CFrame cframe;
            surface->getCoordinateFrame(cframe, false);
            rd->setObjectToWorldMatrix(cframe);
                        
            CFrame previousFrame;
            surface->getCoordinateFrame(previousFrame, true);

            //Setting macros in attempt to satisfy compiler
            args.setMacro("HAS_VERTEX_COLOR", 1);
            args.setMacro("NUM_LIGHTMAP_DIRECTIONS", 0);
            args.setMacro("LOD", 0);
            args.setUniform("pointRadius", surface->m_pointModel->m_pointRadius);
            args.setAttributeArray("position", surface->m_pointArray->gpuPosition);
            args.setAttributeArray("emission", surface->m_pointArray->gpuRadiance);

            gbuffer->setShaderArgsWritePosition(previousFrame, rd, args);
            
            const Rect2D& colorRect = gbuffer->colorRect();
            args.setUniform("lowerCoord", colorRect.x0y0());
            args.setUniform("upperCoord", colorRect.x1y1());

            // N.B. Alpha testing is handled explicitly inside the shader.
            LAUNCH_SHADER_WITH_HINT("PointSurface_gbuffer.*", args, surface->m_profilerHint);
        }
        
    } rd->popState();
}


void PointSurface::bindDepthPeelArgs
(Args& args, 
 RenderDevice* rd, 
 const shared_ptr<Texture>& depthPeelTexture, 
 const float minZSeparation) const {

    const bool useDepthPeel = notNull(depthPeelTexture);
    args.setMacro("USE_DEPTH_PEEL", useDepthPeel ? 1 : 0);
    if (useDepthPeel) {
        const Vector3& clipInfo = Projection(rd->projectionMatrix(), rd->viewport().wh()).reconstructFromDepthClipInfo();
        args.setUniform("previousDepthBuffer", depthPeelTexture, Sampler::buffer());
        args.setUniform("minZSeparation",  minZSeparation);
        args.setUniform("currentToPreviousScale", Vector2(depthPeelTexture->width()  / rd->viewport().width(),
                                                          depthPeelTexture->height() / rd->viewport().height()));
        args.setUniform("clipInfo", clipInfo);
    }
}


void PointSurface::getObjectSpaceBoundingBox(G3D::AABox& bounds, bool previous) const {
    bounds = m_pointArray->boxBounds;
}
  

void PointSurface::getObjectSpaceBoundingSphere(G3D::Sphere& bounds, bool previous) const {
    bounds = m_pointArray->sphereBounds;
}

}
