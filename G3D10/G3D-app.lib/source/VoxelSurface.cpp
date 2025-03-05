/**
  \file G3D-app.lib/source/VoxelSurface.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/VoxelSurface.h"    
#include "G3D-app/Entity.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Shader.h"
#include "G3D-base/Projection.h"

namespace G3D {
VoxelSurface::VoxelSurface
   (const String&                       name,
    const CFrame&                       frame,
    const CFrame&                       previousFrame,
    const shared_ptr<VoxelModel>&       model,
    const shared_ptr<Entity>&           entity, 
    const Surface::ExpressiveLightScatteringProperties& expressive) : 
    Surface(expressive),
    m_name(name), 
    m_frame(frame),
    m_previousFrame(previousFrame),
    m_voxelModel(model),
    m_profilerHint((notNull(entity) ? entity->name() + "/" : "") + (notNull(model) ? model->name() + "/" : "") + m_name) {

    m_model  = model;
    m_entity = entity;

}


shared_ptr<VoxelSurface> VoxelSurface::create
   (const String&                       name,
    const CFrame&                       frame,
    const CFrame&                       previousFrame,
    const shared_ptr<VoxelModel>&       model, 
    const shared_ptr<Entity>&           entity, 
    const Surface::ExpressiveLightScatteringProperties& expressive) {

    return createShared<VoxelSurface>(name, frame, previousFrame, model, entity, expressive);

}


void VoxelSurface::render
   (RenderDevice*               rd,
    const LightingEnvironment&  environment,
    RenderPassType              passType) const {

    Args args;

    // Choose LOD based on distance ot the camera
    rd->setObjectToWorldMatrix(m_frame);
    
    // Object to world matrix
    //const CFrame& o2w = rd->worldToCameraMatrix() * m_frame;
    
    // const Point3& center = o2w.pointToWorldSpace(m_voxelModel->m_sphereBounds.center);

    // Negative value (when not clipped)
    // const float nearestZ = min(0.01f, center.z + m_voxelModel->m_sphereBounds.radius);
    
    args.setPrimitiveType(PrimitiveType::POINTS);

    args.setAttributeArray("position", m_voxelModel->m_gpuPosition);
    args.setAttributeArray("color", m_voxelModel->m_gpuColor);

    args.setUniform("osCameraPos", m_frame.pointToObjectSpace(rd->cameraToWorldMatrix().translation));
    args.setUniform("voxelRadius", m_voxelModel->m_voxelRadius);
    args.setUniform("invVoxelRadius", 1.0 / m_voxelModel->m_voxelRadius);
    args.setUniform("halfScreenSize", rd->viewport().wh() / 2.0f);

    glEnable(GL_DEPTH_CLAMP);

    //there will be g3d_objectToCameraMatrix already set in the shader
    //need to multiplt point local and scale by 3x3 rotation component of the objectToCameraMatrix.
    //passed to pixel shader (out from vrt, in to pix), use to compute the ray in camera space
    LAUNCH_SHADER("VoxelSurface_render.*", args);

}


void VoxelSurface::renderDepthOnlyHomogeneous
    (RenderDevice*                      rd,
     const Array<shared_ptr<Surface> >& surfaceArray,
     const shared_ptr<Texture>&         previousDepthBuffer,
     const float                        minZSeparation,
     TransparencyTestMode               transparencyTestMode,
     const Color3&                      transmissionWeight) const {
   
    debugAssertGLOk();

    rd->pushState(); {
        glEnable(GL_DEPTH_CLAMP);

        for (int i = 0; i < surfaceArray.size(); ++i) {
            const shared_ptr<VoxelSurface>& surface = dynamic_pointer_cast<VoxelSurface>(surfaceArray[i]);
            debugAssertM(surface, "Surface::renderDepthOnlyHomogeneous passed the wrong subclass");
            Args args;

            args.setPrimitiveType(PrimitiveType::POINTS);

            CFrame cframe;
            surface->getCoordinateFrame(cframe, false);
            rd->setObjectToWorldMatrix(cframe);
            debugAssertM(fuzzyEq(cframe.rotation.determinant(), 1.0f), "scaled transformation matrix!");
            args.setUniform("voxelRadius", surface->m_voxelModel->m_voxelRadius);
            args.setUniform("invVoxelRadius", 1.0f / m_voxelModel->m_voxelRadius);
            args.setUniform("halfScreenSize", rd->viewport().wh() / 2.0f);
            args.setUniform("osCameraPos", m_frame.pointToObjectSpace(rd->cameraToWorldMatrix().translation));

            args.setAttributeArray("position", surface->m_voxelModel->m_gpuPosition);

            bindDepthPeelArgs(args, rd, previousDepthBuffer, minZSeparation);

            if (notNull(previousDepthBuffer)) {
                LAUNCH_SHADER_WITH_HINT("VoxelSurface_depthPeel.*", args, surface->m_profilerHint);
            } else {
                LAUNCH_SHADER_WITH_HINT("VoxelSurface_depthOnly.*", args, surface->m_profilerHint);
            }
        }
    } rd->popState();
}


void VoxelSurface::renderIntoGBufferHomogeneous
    (RenderDevice*                          rd, 
     const Array< shared_ptr< Surface > >&  surfaceArray,
     const shared_ptr< GBuffer >&           gbuffer,
     const shared_ptr< Texture >&           depthPeelTexture,
     const float                            minZSeparation,
     const LightingEnvironment&             lighting) const {
        
    rd->pushState(); {

        glEnable(GL_DEPTH_CLAMP);

        // const CullFace oldCullFace = rd->cullFace();

        // Render front-to-back for early-out Z
        for (int s = surfaceArray.size() - 1; s >= 0; --s) {
            const shared_ptr<VoxelSurface>& surface = dynamic_pointer_cast<VoxelSurface>(surfaceArray[s]);

            debugAssertM(notNull(surface),
                "Non VoxelSurface element of surfaceArray "
                "in VoxelSurface::renderIntoGBufferHomogeneous");

            Args args;

            args.setPrimitiveType(PrimitiveType::POINTS);

            CFrame cframe;
            surface->getCoordinateFrame(cframe, false);
            rd->setObjectToWorldMatrix(cframe);

            const shared_ptr<VoxelModel>& model = surface->m_voxelModel;

            //Setting macros in attempt to satisfy compiler
            args.setMacro("HAS_VERTEX_COLOR", 1);
            args.setMacro("NUM_LIGHTMAP_DIRECTIONS", 0);
            args.setUniform("voxelRadius", model->m_voxelRadius);
            args.setUniform("osCameraPos", m_frame.pointToObjectSpace(rd->cameraToWorldMatrix().translation));
            args.setUniform("invVoxelRadius", 1.0f / model->m_voxelRadius);
            args.setUniform("halfScreenSize", rd->viewport().wh() / 2.0f);
            args.setAttributeArray("position", model->m_gpuPosition);
            args.setAttributeArray("color", model->m_gpuColor);
            
            CFrame previousFrame;
            surface->getCoordinateFrame(previousFrame, true);

            gbuffer->setShaderArgsWritePosition(previousFrame, rd, args);

            //  Debugging
            const int count = int(clamp(UniversalSurface::debugSubmitFraction, 0.0f, 1.0f) * float(model->m_gpuPosition.size()));
            
            if (UniversalSurface::debugSubmitFraction < 1.0f) {
                args.setNumIndices(count);
            }

            VoxelModel::voxelsRendered += count;
            // N.B. Alpha testing is handled explicitly inside the shader.
            LAUNCH_SHADER_WITH_HINT("VoxelSurface_gbuffer.*", args, surface->m_profilerHint);
        }
    } rd->popState();
}


void VoxelSurface::bindDepthPeelArgs
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


void VoxelSurface::getObjectSpaceBoundingBox(G3D::AABox& bounds, bool previous) const {
    bounds = m_voxelModel->m_boxBounds;
}
  

void VoxelSurface::getObjectSpaceBoundingSphere(G3D::Sphere& bounds, bool previous) const {
    bounds = m_voxelModel->m_sphereBounds;
}
}//namespace G3D
