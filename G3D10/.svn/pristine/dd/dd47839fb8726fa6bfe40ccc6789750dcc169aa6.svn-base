/**
  \file G3D-app.lib/source/GaussianMIPFilter.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2020, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/GaussianMIPFilter.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Shader.h"

namespace G3D {

    GaussianMIPFilter::GaussianMIPFilter() {
        m_framebuffer = Framebuffer::create("GaussianMIPFilter");
        m_normalZ = Texture::createEmpty("GaussianMIPFilter::m_normalZ", 1000, 1000, ImageFormat::RGBA16F(), Texture::DIM_2D, true);
    }


    shared_ptr<GaussianMIPFilter> GaussianMIPFilter::create() {
        return createShared<GaussianMIPFilter>();
    }


    void GaussianMIPFilter::apply
    (RenderDevice* rd,
        const shared_ptr<Texture>& texture,
        const shared_ptr<Texture>& wsPosition,
        const shared_ptr<Texture>& wsNormal,
        const shared_ptr<Texture>& glossy,
        const CFrame& cameraFrame) const {

        // Note that the glossy gather shader is clamped at 5 mip levels as well to avoid blending in too much
        int width = texture->width(), height = texture->height();
        for (int scale = 1, mip = 1; (min(width, height) > 1) && (mip <= m_mipLimit); scale *= 2, ++mip, width /= 2, height /= 2) {
            m_framebuffer->clear();
            m_framebuffer->set(Framebuffer::COLOR0, texture, CubeFace::POS_X, mip);

            rd->push2D(m_framebuffer); {
                Args args;
                //args.setUniform("srcNormalZ", m_normalZ, Sampler::buffer());
                args.setUniform("srcColor", texture, Sampler::buffer());
                args.setUniform("bias", (mip % 2) * 2 - 1);
                args.setUniform("dstMIP", mip);
                args.setUniform("srcWidth", width);
                args.setUniform("srcHeight", height);
                args.setRect(rd->viewport());
                LAUNCH_SHADER("GaussianMIPFilter_apply.pix", args);
            } rd->pop2D();
        }
        m_framebuffer->clear();
    }
}