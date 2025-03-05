/**
  \file G3D-app.lib/source/BilateralFilter.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/BilateralFilter.h"
#include "G3D-gfx/Shader.h"
#include "G3D-app/GBuffer.h"
#include "G3D-gfx/RenderDevice.h"

namespace G3D {

void BilateralFilter::apply1D
(RenderDevice*                   rd,
 const shared_ptr<Texture>&      source,
 const shared_ptr<GBuffer>&      gbuffer,
 const Vector2&                  direction,
 const BilateralFilterSettings & settings) const {
    Args args;
    args.setMacro("DEPTH_WEIGHT", settings.depthWeight);
    args.setMacro("NORMAL_WEIGHT", settings.normalWeight);
    args.setMacro("PLANE_WEIGHT", settings.planeWeight);
    args.setMacro("GLOSSY_WEIGHT", settings.glossyWeight);

    args.setMacro("R", iMin(settings.radius, 6));
    args.setMacro("MDB_WEIGHTS", settings.monotonicallyDecreasingBilateralWeights ? 1 : 0);
    args.setUniform("axis", direction * float(settings.stepSize));
    args.setUniform("source", source, Sampler::buffer());
    args.setMacro("COMPUTE_PERCENT", iRound(settings.computeFraction * 100.0f));

    gbuffer->setShaderArgsRead(args, "gbuffer_");
    args.setRect(rd->viewport());
    LAUNCH_SHADER("BilateralFilter_apply.pix", args);
}


void BilateralFilter::apply
(RenderDevice*                      rd,
const shared_ptr<Texture>&          source,
const shared_ptr<Framebuffer>&      destination,
const shared_ptr<GBuffer>&          gbuffer,
const BilateralFilterSettings &     settings) const {
    debugAssert(notNull(this));
    if (settings.radius > 0) {
        if (isNull(m_intermediateFramebuffer) || (m_intermediateFramebuffer->texture(0)->encoding() != source->encoding())) {
            m_intermediateFramebuffer = Framebuffer::create(Texture::createEmpty("BilateralFilter::intermediate",
                                                                                    source->width(), source->height(), source->encoding()));
        }
        m_intermediateFramebuffer->resize(source->width(), source->height());

        // HBlur
        rd->push2D(m_intermediateFramebuffer); {
            apply1D(rd, source, gbuffer, Vector2(1, 0), settings);
        } rd->pop2D();

        // VBlur
        rd->push2D(destination); {
            apply1D(rd, m_intermediateFramebuffer->texture(0), gbuffer, Vector2(0, 1), settings);
        } rd->pop2D();

    } else {
        Texture::copy(source, destination->texture(0));
    }
}

}
