/**
  \file G3D-app.lib/source/Film.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/debugAssert.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/Image.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-app/Film.h"
#include "G3D-gfx/glcalls.h"
#include "G3D-app/GuiPane.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/Draw.h"

namespace G3D {

Film::Film() {}


shared_ptr<Film> Film::create() {
    return createShared<Film>();
}


shared_ptr<Texture> Film::exposeAndRender
   (RenderDevice*                   rd,
    const FilmSettings&             settings,
    const shared_ptr<Image>&        image) {
       
    const shared_ptr<Texture>& hdrImage = Texture::fromImage("Source", image);
    shared_ptr<Texture> dst;
    exposeAndRender(rd, settings, hdrImage, 0, 0, dst);
    return dst;
}

   
void Film::exposeAndRender
   (RenderDevice*                   rd,
    const FilmSettings&             settings,
    const shared_ptr<Texture>&      input,
    int                             sourceTrimBandThickness, 
    int                             sourceColorBandThickness,
    shared_ptr<Texture>&            output,
    CubeFace                        outputCubeFace,
    int                             outputMipLevel,
    const shared_ptr<Texture>&      screenSpaceMotion,
    const Vector2                   jitterMotion) {
    
    if (isNull(output)) {
        // Allocate new output texture
        const bool generateMipMaps = false;
        output = Texture::createEmpty("Exposed image", input->width(), input->height(), input->format(), 
                                      input->dimension(), generateMipMaps);
    }
    //    debugAssertM(output->width() % 4 == 0 && output->width() % 4 == 0, "Width and Height must be a multiple of 4");
    const shared_ptr<Framebuffer>& fb = Framebuffer::create("Film temp");
    fb->set(Framebuffer::COLOR0, output, outputCubeFace, outputMipLevel);
    rd->push2D(fb); {
        rd->clear();
        exposeAndRender(rd, settings, input, sourceTrimBandThickness, sourceColorBandThickness, screenSpaceMotion, jitterMotion);
    } rd->pop2D();

    // Override the document gamma
    output->visualization = Texture::Visualization::sRGB();
    output->visualization.documentGamma = settings.gamma();
}


void Film::Filter::allocate
   (const String&                   name, 
    const shared_ptr<Texture>&      source, 
    const shared_ptr<Framebuffer>&  argTarget, 
    int                             sourceDepthGuardBandThickness, 
    const ImageFormat*              fmt) const {

    if (isNull(argTarget)) {
        // Allocate the framebuffer or resize as needed
        const int w = source->width()  - 2 * sourceDepthGuardBandThickness;
        const int h = source->height() - 2 * sourceDepthGuardBandThickness;
        if (isNull(m_intermediateResultFramebuffer)) {
            m_intermediateResultFramebuffer = Framebuffer::create(
                Texture::createEmpty("G3D::Film::" + name + "::m_intermediateResultFramebuffer", 
                                     w, h, ImageFormat::RGBA8(), Texture::DIM_2D, false));
        } else {
            m_intermediateResultFramebuffer->texture(0)->resize(w, h);
        }

        target = m_intermediateResultFramebuffer;
    } else {
        target = argTarget;
    }
}


void Film::FXAAFilter::apply
   (RenderDevice*                   rd,
    const FilmSettings&             settings,
    const shared_ptr<Texture>&      source,
    const shared_ptr<Texture>&      motion, 
    const shared_ptr<Framebuffer>&  argTarget,
    int                             sourceTrimBandThickness,
    int                             sourceDepthGuardBandThickness) const {

    debugAssert((sourceTrimBandThickness == 0) && (sourceDepthGuardBandThickness == 0));
    allocate("FXAAFilter", source, argTarget, sourceDepthGuardBandThickness);

    rd->push2D(target); {
        Args args;
        args.setMacro("COMPUTE_PERCENT", settings.diskFramebuffer() ? 100 : -1);

        source->setShaderArgs(args, "sourceTexture_", Sampler::video());
        args.setRect(rd->viewport());

        if (settings.antialiasingHighQuality()) {
            LAUNCH_SHADER("Film_FXAA_13_quality.*", args);
        } else {
            LAUNCH_SHADER("Film_FXAA_13_performance.*", args);
        }
    } rd->pop2D();
}


void Film::WideAAFilter::apply(RenderDevice* rd, const FilmSettings& settings, const shared_ptr<Texture>& source, const shared_ptr<Texture>& motion, const shared_ptr<Framebuffer>& argTarget, int sourceTrimBandThickness, int sourceDepthGuardBandThickness) const  {
    debugAssert((sourceTrimBandThickness == 0) && (sourceDepthGuardBandThickness == 0));    
    allocate("WideAAFilter", source, argTarget, sourceDepthGuardBandThickness);

    rd->push2D(target); {
        Args args;
        source->setShaderArgs(args, "sourceTexture_", Sampler::video());
        args.setUniform("radius", settings.antialiasingFilterRadius());
        args.setRect(rd->viewport());
        LAUNCH_SHADER("Film_wideAA.*", args);
    } rd->pop2D();
}


void Film::TAAFilter::apply
   (RenderDevice*                   rd,
    const FilmSettings&             settings,
    const shared_ptr<Texture>&      source,
    const shared_ptr<Texture>&      motion, 
    const shared_ptr<Framebuffer>&  argTarget,
    int                             sourceTrimBandThickness,
    int                             sourceDepthGuardBandThickness) const {

    allocate("TAAFilter", source, argTarget, sourceDepthGuardBandThickness);
    
    Args args;
    if ((target != argTarget) && notNull(argTarget) && ! argTarget->isHardwareFramebuffer()) {
        // Had to use an intermediate render target for precision
        target->set(Framebuffer::COLOR1, argTarget->texture(0));
        args.setMacro("DUAL_OUTPUT", 1);
    } else {
        args.setMacro("DUAL_OUTPUT", 0);
    }

    rd->push2D(target); {
        args.setUniform("invertY", target->isHardwareFramebuffer());
        source->setShaderArgs(args, "sourceTexture.", Sampler::buffer());
        if (notNull(motion) && (motion != Texture::opaqueBlack())) {
            args.setMacro("HAS_SOURCE_MOTION", true);
            args.setUniform("motionGuardBand", (motion->vector2Bounds() - source->vector2Bounds()) / 2.0f);
            args.setUniform("jitterMotion", jitterMotion);
            motion->setShaderArgs(args, "sourceMotionTexture.",  Sampler::buffer());
        } else {
            args.setMacro("HAS_SOURCE_MOTION", false);
        }
        m_history->setShaderArgs(args, "historyTexture_", Sampler::video());
        args.setUniform("hysteresis", m_maxHysteresis);
        args.setUniform("trimBand", Vector2int32((source->vector2Bounds() - rd->viewport().wh()) / 2.0f));
        args.setRect(rd->viewport());
        args.setMacro("COMPUTE_PERCENT", settings.diskFramebuffer() ? 100 : -1);

        LAUNCH_SHADER("Film_temporalAA.*", args);

        // Copy the result to the history buffer
        if (target->isHardwareFramebuffer()) {
            m_history->copyFromScreen(target->rect2DBounds());
        } else {
            target->texture(0)->copyInto(m_history);
        }
    } rd->pop2D();


    if ((target != argTarget) && (isNull(argTarget) || argTarget->isHardwareFramebuffer())) {
        // We could't write to the HW frame buffer at the same time as the regular output,
        // and it wasn't high enough precision to use outright. So, copy target to
        // the HW framebuffer now.
        rd->push2D(); {
            Draw::rect2D(target->rect2DBounds(), rd, Color3::white(), target->texture(0)); 
        } rd->pop2D();
    }
}


void Film::TAAFilter::allocate
    (const String&                      name,
    const shared_ptr<Texture>&          source,
    const shared_ptr<Framebuffer>&      argTarget,
    int                                 sourceDepthGuardBandThickness,
    const ImageFormat*                  fmt) const {
    
    const bool lowPrecisionTarget = notNull(argTarget) && (argTarget->isHardwareFramebuffer() || (argTarget->texture(0)->format()->redBits < 10));

    if (isNull(argTarget) || lowPrecisionTarget) {
        // Allocate the framebuffer or resize as needed
        const int w = source->width()  - 2 * sourceDepthGuardBandThickness;
        const int h = source->height() - 2 * sourceDepthGuardBandThickness;
        if (isNull(m_intermediateResultFramebuffer)) {
            m_intermediateResultFramebuffer = Framebuffer::create(
                Texture::createEmpty("G3D::Film::" + name + "::m_intermediateResultFramebuffer", 
                                     w, h, ImageFormat::RGBA16F(), Texture::DIM_2D, false));
        } else {
            m_intermediateResultFramebuffer->texture(0)->resize(w, h);
        }

        target = m_intermediateResultFramebuffer;
    } else {
        target = argTarget;
    }

    if (isNull(m_history) || (m_history->vector2Bounds() != target->vector2Bounds())) {
        m_history = Texture::createEmpty("G3D::Film::TAAFilter::m_history", target->width(), target->height(), ImageFormat::RGBA16F());
    }
}



void Film::DebugZoomFilter::apply
   (RenderDevice*                   rd,
    const FilmSettings&             settings,
    const shared_ptr<Texture>&      source, 
    const shared_ptr<Texture>&      motion, 
    const shared_ptr<Framebuffer>&  argTarget,
    int                             sourceTrimBandThickness, 
    int                             sourceDepthGuardBandThickness) const  {
    
    debugAssert((sourceTrimBandThickness == 0) && (sourceDepthGuardBandThickness == 0));
    debugAssert(settings.debugZoom() > 1);

    allocate("DebugZoomFilter", source, argTarget, sourceDepthGuardBandThickness);
    const bool invertY = target->invertY();
    
    rd->push2D(target); {
        Args args;
        args.setUniform("source", source, Sampler::video());
        args.setUniform("yOffset", invertY ? rd->height() : 0);
        args.setUniform("ySign", invertY ? -1 : 1);
        args.setUniform("dstOffset", Point2::zero());
        args.setUniform("offset", Vector2int32((target->vector2Bounds() -
            target->vector2Bounds() / float(settings.debugZoom())) / 2));
        args.setUniform("scale", settings.debugZoom());
        args.setRect(rd->viewport());
        LAUNCH_SHADER("Film_zoom.*", args);
    } rd->pop2D();
}


void Film::EffectsDisabledBlitFilter::apply(RenderDevice* rd, const FilmSettings& settings, const shared_ptr<Texture>& source, const shared_ptr<Texture>& motion, 
    const shared_ptr<Framebuffer>& argTarget, int sourceTrimBandThickness, int sourceDepthGuardBandThickness) const  {
    
    debugAssert(sourceTrimBandThickness <= sourceDepthGuardBandThickness);
    allocate("EffectsDisabledBlitFilter", source, argTarget, sourceDepthGuardBandThickness);
    rd->push2D(target); {
        Args args;
        source->setShaderArgs(args, "sourceTexture_", Sampler::video());
        args.setUniform("invertX", settings.invertX());
        args.setUniform("invertY", settings.invertY());
        args.setMacro("FILTER", filter);
        args.setMacro("SHARPEN", sharpen);
        args.setUniform("guardBandSize", Vector2int32(sourceDepthGuardBandThickness, sourceDepthGuardBandThickness));
        args.setRect(rd->viewport());
        LAUNCH_SHADER("Film_effectsDisabledBlit.pix", args);
    } rd->pop2D();
}


void Film::exposeAndRender
   (RenderDevice*                   rd,
    const FilmSettings&             settings,
    const shared_ptr<Texture>&      inputTexture,
    int                             sourceTrimBandThickness,
    int                             sourceDepthGuardBandThickness,
    const shared_ptr<Texture>&      screenSpaceMotionTexture,
    const Vector2                   jitterMotion) {
    
    BEGIN_PROFILER_EVENT("Film::exposeAndRender");

    debugAssert(sourceTrimBandThickness <= sourceDepthGuardBandThickness);
    debugAssertM(rd->drawFramebuffer(), "In G3D 10, the drawFramebuffer should never be null");
   
    // Keep the size of this small array equal to the maximum number of filters that can be bound
    // to avoid heap allocation
    typedef SmallArray<Filter*, 6> FilterChain;
    FilterChain filterChain;

    const bool useHighQualityRescaling = true;

    // Build the filter chain (in forward order):
    if (settings.effectsEnabled()) {
        // Perform TAA before bloom on full-size, HDR data
        const bool hdrTAA = true;

        // Run TAA first if in HDR mode, otherwise last
        if (settings.temporalAntialiasingEnabled() && hdrTAA) {
            filterChain.push(&m_taaFilter);
            m_taaFilter.jitterMotion = jitterMotion;
        }

        filterChain.push(&m_compositeFilter);
                
        if (settings.antialiasingEnabled()) {
            filterChain.push(&m_fxaaFilter);

            if (settings.antialiasingFilterRadius() > 0) {
                filterChain.push(&m_wideAAFilter);
            }
        }

        if (settings.temporalAntialiasingEnabled() && ! hdrTAA) {
            filterChain.push(&m_taaFilter);
            m_taaFilter.jitterMotion = jitterMotion;
        }


        const Vector2int16 inputSize(inputTexture->vector2Bounds() - Vector2::one() * float(sourceTrimBandThickness));
        const Vector2int16 outputSize(rd->drawFramebuffer()->vector2Bounds());

        if (useHighQualityRescaling && (inputSize.x != outputSize.x)) {
            // Need a resizing filter. The other filters in the chain are capable of resizing automatically,
            // however, they only provide nearest-neighbor scaling.
            filterChain.push(&m_effectsDisabledBlitFilter);
            if (outputSize.x > inputSize.x) {
                // Upscale
                m_effectsDisabledBlitFilter.filter = min(int(settings.upscaleFilter()), 2);
                m_effectsDisabledBlitFilter.sharpen = (settings.upscaleFilter() == ResampleFilter::BICUBIC_SHARPER) ? 0.7f : 0.0f;
            } else {
                // Downscale
                m_effectsDisabledBlitFilter.filter = min(int(settings.downscaleFilter()), 2);
                m_effectsDisabledBlitFilter.sharpen = (settings.downscaleFilter() == ResampleFilter::BICUBIC_SHARPER) ? 0.7f : 0.0f;
            }
        }

        if (settings.debugZoom() > 1) {
            filterChain.push(&m_debugZoomFilter);
        }

    } else {
        filterChain.push(&m_effectsDisabledBlitFilter);
        m_effectsDisabledBlitFilter.filter = 0;
    }

    // Run the filters. The first one strips the guard band.
    // All run at the rendering resolution until the last, which
    // is responsible for upscaling or downscaling to the final
    // resolve resolution.
    for (int i = 0; i < filterChain.size(); ++i) {
        const bool first = (i == 0);
        const bool last  = (i == filterChain.size() - 1);

        // The first filter reads from the inputTexture.
        // The others read from the previous filter's framebuffer.
        const shared_ptr<Texture>& sourceTexture = first ? inputTexture : filterChain[i - 1]->target->texture(Framebuffer::COLOR0);
        
#   ifdef G3D_DEBUG
        {
            const Vector2 originalInputSize = inputTexture->vector2Bounds() - 2.0f * Vector2(float(sourceDepthGuardBandThickness), float(sourceDepthGuardBandThickness));
            const Vector2 mySourceSize = sourceTexture->vector2Bounds();
            debugAssertM(first || (originalInputSize == mySourceSize),
                "The previous filter produced an output texture of the wrong size.");
        }
#   endif
        // The last filter targets the currently bound framebuffer. The intermediates
        // write to their own internal output buffer.
        const shared_ptr<Framebuffer>& targetFramebuffer = last ? rd->drawFramebuffer() : nullptr;
        filterChain[i]->apply(rd, settings, sourceTexture,
            screenSpaceMotionTexture, targetFramebuffer,
            first ? sourceTrimBandThickness : 0,
            first ? sourceDepthGuardBandThickness : 0);
    }
    
    END_PROFILER_EVENT();
}


}
