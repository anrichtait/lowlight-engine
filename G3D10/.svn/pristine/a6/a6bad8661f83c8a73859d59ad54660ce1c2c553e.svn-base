/**
  \file G3D-app.lib/source/FogVolumeSurface.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/FogVolumeSurface.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-base/Projection.h"
#include "G3D-gfx/Shader.h"

namespace G3D {

shared_ptr<FogVolumeSurface> FogVolumeSurface::create() {
    return createShared<FogVolumeSurface>();
}


void FogVolumeSurface::render
    (RenderDevice*                      rd,
    const LightingEnvironment&          environment,
    RenderPassType                      passType) const {

    // Only render in blended mode
    if ((passType != RenderPassType::SINGLE_PASS_UNORDERED_BLENDED_SAMPLES) && (passType != RenderPassType::MULTIPASS_BLENDED_SAMPLES)) {
        return;
    }
    // Grab the camera information before switching to 2D mode
    const CFrame     camraToWorldMatrix(rd->cameraToWorldMatrix());
    const Projection projection(rd->projectionMatrix());

    // Switch in to 2D mode, since we're submitting an infinite volume anyway and won't get useful depth testing
    rd->push2D(); {
        Args args;

        /*
        We explicitly compute our own weights, so don't use the Renderer's oitWritePixel

        if (passType == RenderPassType::SINGLE_PASS_UNORDERED_BLENDED_SAMPLES) {
            args.setMacro("DECLARE_writePixel", singlePassBlendedWritePixelDeclaration);
        } else {
            debugAssertM(singlePassBlendedWritePixelDeclaration.empty() || (singlePassBlendedWritePixelDeclaration == Surface::defaultWritePixelDeclaration()),
                "Passed a custom singlePassBlendedWritePixelDeclaration value with a render pass type that does not support it. G3D will ignore singlePassBlendedWritePixelDeclaration!");
        }
        */

        // Information for constructing eye rays
        args.setUniform("cameraToWorldMatrix", camraToWorldMatrix);
        args.setUniform("tanHalfFieldOfViewY",  float(tan(projection.fieldOfViewAngle() / 2.0f)));

        // Projection matrix, for computing OpenGL depth values
        Matrix4 projectionMatrix;
        projection.getProjectUnitMatrix(rd->viewport(), projectionMatrix);
        args.setUniform("projectionMatrix22", projectionMatrix[2][2]);
        args.setUniform("projectionMatrix23", projectionMatrix[2][3]);

        args.setUniform("depthBuffer", Texture::opaqueBlackIfNull(rd->framebuffer()->texture(Framebuffer::DEPTH)), Sampler::buffer());
        args.setUniform("clipInfo", projection.reconstructFromDepthClipInfo());
        environment.setShaderArgs(args, "");

        // Follow the current guard band
        args.setRect(rd->clip2D());

        LAUNCH_SHADER("FogVolumeSurface_render.*", args);
    } rd->pop2D();
}

}