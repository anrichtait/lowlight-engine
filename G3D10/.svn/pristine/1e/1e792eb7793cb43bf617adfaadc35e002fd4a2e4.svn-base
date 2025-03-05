/**
  \file G3D-app.lib/source/DepthOfField.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/DepthOfField.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/Camera.h"
#include "G3D-app/Draw.h"

namespace G3D {


DepthOfField::DepthOfField(const String& debugName) : m_debugName(debugName) {
    // Intentionally empty
}


shared_ptr<DepthOfField> DepthOfField::create(const String& debugName) {
    return createShared<DepthOfField>(debugName);
}


void DepthOfField::apply
(RenderDevice*                  rd, 
 shared_ptr<Texture>            color,
 shared_ptr<Texture>            depth, 
 const shared_ptr<Camera>&      camera,
 Vector2int16                   trimBandThickness,
 DebugOption                    debugOption) {
    
    if ((camera->depthOfFieldSettings().model() == DepthOfFieldModel::NONE) || ! camera->depthOfFieldSettings().enabled()) {
        const shared_ptr<Framebuffer>& f = rd->framebuffer();
        const shared_ptr<Framebuffer::Attachment>& a = f->get(Framebuffer::COLOR0);
       
        if (isNull(f) || (a->texture() != color)) {
            Texture::copy(color, a->texture(), 0, 0, 1.0f, trimBandThickness, CubeFace::POS_X, CubeFace::POS_X, rd, false);
        }

        // Exit abruptly because DoF is disabled
        return;
    }

    alwaysAssertM(notNull(color), "Color buffer may not be nullptr");
    alwaysAssertM(notNull(depth), "Depth buffer may not be nullptr");

    BEGIN_PROFILER_EVENT("DepthOfField::apply");
    resizeBuffers(color, camera->depthOfFieldSettings().reducedResolutionFactor(), trimBandThickness);

    const Rect2D& viewport = color->rect2DBounds();

    // Magic scaling factor for the artist mode depth of field model far-field radius
    float farRadiusRescale = 1.0f;
    const float maxCoCRadiusPixels = ceil(camera->maxCircleOfConfusionRadiusPixels(viewport));
    const bool diskFramebuffer = camera->depthOfFieldSettings().diskFramebuffer();

    debugAssert(maxCoCRadiusPixels >= 0.0f);
    computeCoC(rd, color, depth, camera, trimBandThickness, farRadiusRescale, maxCoCRadiusPixels);
    blurPass(rd, m_packedBuffer, m_packedBuffer, m_horizontalFramebuffer, true, camera, viewport, maxCoCRadiusPixels, diskFramebuffer);
    blurPass(rd, m_tempBlurBuffer, m_tempNearBuffer, m_verticalFramebuffer, false, camera, viewport, maxCoCRadiusPixels, diskFramebuffer);
    composite(rd, m_packedBuffer, m_blurBuffer, m_nearBuffer, debugOption, trimBandThickness, farRadiusRescale, diskFramebuffer);
    END_PROFILER_EVENT();
}


void DepthOfField::computeCoC
(RenderDevice*                  rd, 
 const shared_ptr<Texture>&     color, 
 const shared_ptr<Texture>&     depth, 
 const shared_ptr<Camera>&      camera,
 Vector2int16                   trimBandThickness,
 float&                         farRadiusRescale,
 float                          maxCoCRadiusPixels) {

    rd->push2D(m_packedFramebuffer); {
        rd->clear();
        Args args;

        args.setUniform("clipInfo",             camera->projection().reconstructFromDepthClipInfo());
        args.setUniform("COLOR_buffer",         color, Sampler::video());
        args.setUniform("DEPTH_buffer",         depth, Sampler::buffer());
        args.setUniform("trimBandThickness",    trimBandThickness);
        args.setRect(rd->viewport());

        const float axisSize = (camera->fieldOfViewDirection() == FOVDirection::HORIZONTAL) ? float(color->width()) : float(color->height());
        
        if (camera->depthOfFieldSettings().model() == DepthOfFieldModel::ARTIST) {

            args.setUniform("nearBlurryPlaneZ", camera->depthOfFieldSettings().nearBlurryPlaneZ());
            args.setUniform("nearSharpPlaneZ",  camera->depthOfFieldSettings().nearSharpPlaneZ());
            args.setUniform("farSharpPlaneZ",   camera->depthOfFieldSettings().farSharpPlaneZ());
            args.setUniform("farBlurryPlaneZ",  camera->depthOfFieldSettings().farBlurryPlaneZ());

            // This is a positive number
            const float nearScale =             
                camera->depthOfFieldSettings().nearBlurRadiusFraction() / 
                (camera->depthOfFieldSettings().nearBlurryPlaneZ() - camera->depthOfFieldSettings().nearSharpPlaneZ());
            alwaysAssertM(nearScale >= 0.0f, "Near normalization must be a non-negative factor");
            args.setUniform("nearScale", nearScale * axisSize / maxCoCRadiusPixels); 

            // This is a positive number
            const float farScale =    
                camera->depthOfFieldSettings().farBlurRadiusFraction() / 
                (camera->depthOfFieldSettings().farSharpPlaneZ() - camera->depthOfFieldSettings().farBlurryPlaneZ());
            alwaysAssertM(farScale >= 0.0f, "Far normalization must be a non-negative factor");
            args.setUniform("farScale", farScale * axisSize / maxCoCRadiusPixels);

            farRadiusRescale =
                max(camera->depthOfFieldSettings().farBlurRadiusFraction(), camera->depthOfFieldSettings().nearBlurRadiusFraction()) /
                max(camera->depthOfFieldSettings().farBlurRadiusFraction(), 0.0001f);

        } else {
            farRadiusRescale = 1.0f;
            const float screenSize = (camera->fieldOfViewDirection() == FOVDirection::VERTICAL) ? rd->viewport().height() : rd->viewport().width();

            // Collect terms from the CoC computation that are constant across the screen into a single
            // constant
            const float scale = (screenSize * 0.5f / tan(camera->fieldOfViewAngle() * 0.5f)) * camera->depthOfFieldSettings().lensRadius() / 
                (camera->depthOfFieldSettings().focusPlaneZ() * maxCoCRadiusPixels);

            args.setUniform("focusPlaneZ", camera->depthOfFieldSettings().focusPlaneZ());
            args.setUniform("scale", scale);

            // This is a hack to support ChromaBlur experiments. It is not used by the default 
            // G3D shaders, and is not the intended use of nearSharpPlaneZ.
            args.setUniform("nearSharpPlaneZ", camera->depthOfFieldSettings().nearSharpPlaneZ());
        }
        
        args.setMacro("MODEL", camera->depthOfFieldSettings().model().toString());
        args.setMacro("PACK_WITH_COLOR", 1);

        // In case the output is an unsigned format
        args.setUniform("writeScaleBias", Vector2(0.5f, 0.5f));
        args.setMacro("COMPUTE_PERCENT", camera->depthOfFieldSettings().diskFramebuffer() ? 100 : -1);
        LAUNCH_SHADER("DepthOfField_circleOfConfusion.pix", args);

    } rd->pop2D();
}


void DepthOfField::blurPass
(RenderDevice*                  rd,
 const shared_ptr<Texture>&     blurInput,
 const shared_ptr<Texture>&     nearInput,
 const shared_ptr<Framebuffer>& output,
 bool                           horizontal,
 const shared_ptr<Camera>&      camera,
 const Rect2D&                  fullViewport,
 float                          maxCoCRadiusPixels,
 bool                           diskFramebuffer) {

    alwaysAssertM(notNull(blurInput), "Input is nullptr");

    // Dimension along which the blur fraction is measured
    const float dimension = 
        float((camera->fieldOfViewDirection() == FOVDirection::HORIZONTAL) ?
            fullViewport.width() : fullViewport.height());

    // Compute the worst-case near plane blur
    int nearBlurRadiusPixels;
    {
        float n = 0.0f;
        if  (camera->depthOfFieldSettings().model() == DepthOfFieldModel::ARTIST) {
            n = camera->depthOfFieldSettings().nearBlurRadiusFraction() * dimension;
        } else {
            n = -camera->circleOfConfusionRadiusPixels(
                                  min(camera->m_closestNearPlaneZForDepthOfField,
                                      camera->projection().nearPlaneZ()),
                                  fullViewport);
        }

        // Clamp to the maximum permitted radius for this camera
        nearBlurRadiusPixels = iCeil(min(camera->m_viewportFractionMaxCircleOfConfusion * fullViewport.width(), n));
        
        if (nearBlurRadiusPixels < camera->depthOfFieldSettings().reducedResolutionFactor() - 1) {
            // Avoid ever showing the downsampled buffer without blur
            nearBlurRadiusPixels = 0;
        }
    }


    rd->push2D(output); {
        rd->clear();
        Args args;
        args.setUniform("blurSourceBuffer",   blurInput, Sampler::buffer());
        args.setUniform("nearSourceBuffer",   nearInput, Sampler::buffer(), true);
        args.setUniform("maxCoCRadiusPixels", int(maxCoCRadiusPixels));
        args.setUniform("lowResolutionFactor", (float)camera->depthOfFieldSettings().reducedResolutionFactor());
        args.setUniform("nearBlurRadiusPixels", nearBlurRadiusPixels);
        args.setUniform("invNearBlurRadiusPixels", 1.0f / max(float(nearBlurRadiusPixels), 0.0001f));
        args.setUniform("fieldOfView", (float)camera->fieldOfViewAngle());
        args.setMacro("HORIZONTAL", horizontal ? 1 : 0);
        args.setMacro("COMPUTE_PERCENT", diskFramebuffer ? 100 : -1);
        args.setRect(rd->viewport());
        LAUNCH_SHADER("DepthOfField_blur.*", args);
    } rd->pop2D();
}


void DepthOfField::composite
(RenderDevice*          rd,
 shared_ptr<Texture>    packedBuffer, 
 shared_ptr<Texture>    blurBuffer,
 shared_ptr<Texture>    nearBuffer,
 DebugOption            debugOption,
 Vector2int16           outputGuardBandThickness,
 float                  farRadiusRescale,
 bool                   diskFramebuffer) {

    debugAssert(farRadiusRescale >= 0.0);
    rd->push2D(); {
        rd->clear(true, false, false);
        rd->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS);
        rd->setDepthWrite(false);
        Args args;
        
        args.setUniform("blurBuffer",   blurBuffer, Sampler::video());
        args.setUniform("nearBuffer",   nearBuffer, Sampler::video());
        args.setUniform("packedBuffer", packedBuffer, Sampler::buffer());
        args.setUniform("packedBufferInvSize", Vector2(1.0f, 1.0f) / packedBuffer->vector2Bounds());
        args.setUniform("farRadiusRescale", farRadiusRescale);
        args.setMacro("COMPUTE_PERCENT", diskFramebuffer ? 100 : -1);
        args.setUniform("debugOption",  debugOption);
        args.setRect(Rect2D::xywh(Vector2(outputGuardBandThickness), rd->viewport().wh() - 2.0f * Vector2(outputGuardBandThickness)));

        LAUNCH_SHADER("DepthOfField_composite.*", args);
    } rd->pop2D();
}


/** Allocates or resizes a texture and framebuffer to match a target
    format and dimensions. */
static void matchTarget
(const String&                  textureName,
 const shared_ptr<Texture>&     target, 
 int                            divWidth, 
 int                            divHeight,
 int                            guardBandRemoveX,
 int                            guardBandRemoveY,
 const ImageFormat*             format,
 shared_ptr<Texture>&           texture, 
 shared_ptr<Framebuffer>&       framebuffer,
 Framebuffer::AttachmentPoint   attachmentPoint = Framebuffer::COLOR0,
 bool                           generateMipMaps = false) {

    alwaysAssertM(format, "Format may not be nullptr");

    const int w = (target->width()  - guardBandRemoveX * 2) / divWidth;
    const int h = (target->height() - guardBandRemoveY * 2) / divHeight;
    if (isNull(texture) || (texture->format() != format)) {
        // Allocate
        texture = Texture::createEmpty
            (textureName, 
             w, 
             h,
             format,
             Texture::DIM_2D,
             generateMipMaps);

        if (isNull(framebuffer)) {
            framebuffer = Framebuffer::create("");
        }
        framebuffer->set(attachmentPoint, texture);

    } else if ((texture->width() != w) ||
               (texture->height() != h)) {
        texture->resize(w, h);
    }
}


void DepthOfField::resizeBuffers(shared_ptr<Texture> target, int reducedResolutionFactor, const Vector2int16 trimBandThickness) {
    const ImageFormat* plusAlphaFormat = ImageFormat::getFormatWithAlpha(target->format());

    // Need an alpha channel for storing radius in the packed and far temp buffers
    matchTarget(m_debugName + "::m_packedBuffer", target, 1, 1, trimBandThickness.x, trimBandThickness.y, plusAlphaFormat,     m_packedBuffer,    m_packedFramebuffer,     Framebuffer::COLOR0);

    matchTarget(m_debugName + "::m_tempNearBuffer", target, reducedResolutionFactor, 1, trimBandThickness.x, trimBandThickness.y, plusAlphaFormat,     m_tempNearBuffer,  m_horizontalFramebuffer, Framebuffer::COLOR0);
    matchTarget(m_debugName + "::m_tempBlurBuffer", target, reducedResolutionFactor, 1, trimBandThickness.x, trimBandThickness.y, plusAlphaFormat,     m_tempBlurBuffer,  m_horizontalFramebuffer, Framebuffer::COLOR1);

    // Need an alpha channel (for coverage) in the near buffer
    matchTarget(m_debugName + "::m_nearBuffer", target, reducedResolutionFactor, reducedResolutionFactor, trimBandThickness.x, trimBandThickness.y, plusAlphaFormat,     m_nearBuffer,      m_verticalFramebuffer,   Framebuffer::COLOR0);
    matchTarget(m_debugName + "::m_blurBuffer", target, reducedResolutionFactor, reducedResolutionFactor, trimBandThickness.x, trimBandThickness.y, target->format(),    m_blurBuffer,      m_verticalFramebuffer,   Framebuffer::COLOR1);
}


} // Namespace G3D
