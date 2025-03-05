/**
  \file G3D-app.lib/include/G3D-app/FilmSettings.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_FilmSettings_h

#include "G3D-base/platform.h"
#include "G3D-base/Spline.h"

namespace G3D {

class Any;
class GuiPane;

/** 
 \see G3D::Film, G3D::Camera
*/
class FilmSettings {
private:

    /** \brief Monitor gamma used in tone-mapping. Default is 2.0. */
    float                   m_gamma;

    /** \brief Scale factor applied to the pixel values during exposeAndRender(). */
    float                   m_sensitivity;

    /** \brief 0 = no bloom, 1 = blurred out image. */
    float                   m_bloomStrength;

    /** \brief Bloom filter kernel radius as a fraction 
     of the larger of image width/height.*/
    float                   m_bloomRadiusFraction;

    bool                    m_antialiasingEnabled;
    float                   m_antialiasingFilterRadius;
    bool                    m_antialiasingHighQuality;
    bool                    m_temporalAntialiasingEnabled;

    float                   m_vignetteTopStrength;
    float                   m_vignetteBottomStrength;
    float                   m_vignetteSizeFraction;

    int                     m_debugZoom;

    bool                    m_diskFramebuffer = false;

    bool                    m_invertX = false;
    bool                    m_invertY = false;

    ResampleFilter          m_downscaleFilter = ResampleFilter::BILINEAR;
    ResampleFilter          m_upscaleFilter = ResampleFilter::BILINEAR;

    Spline<float>           m_toneCurve;

    /** 3D color grading. See also G3D::FogVolume and G3D::ParticleSystem for a
        physically-based approach. */
    class ColorGradePoint {
    public:
        /** FrameName::WORLD, FrameName::CAMERA, or FrameName::SCREEN */
        FrameName           frameName = FrameName::CAMERA;

        /** 2D points use z=0 and fractions of width and height */
        Point3              position;

        /** Modifes the interpolation parameter. Range 0 - 1 */
        float               strength = 1.0f;

        /** 1, 2, or 3 for linear, quadratic, and cubic interpolation of effect */
        int                 order = 1;

        Color3              color = Color3::black();

        /** If true, apply as fog so that the screen pixel is interpolated to \a color.
            If false, apply as a tint on the hue only. */
        bool                opaque = true;
        /** Reserved */
        float               hueShift = 0.0f;

        /** Reserved */
        float               valueScale = 1.0f;

        /** Reserved */
        float               saturationScale = 1.0f;
    };
#if 0
    /** Enable color grading */
    bool                    m_colorGradeEnabled = false;

    /** Reserved */
    float                   m_colorGradeHueShift = 0.0f;

    /** Reserved */
    float                   m_colorGradeValueScale = 1.0f;

    /** Reserved */
    float                   m_colorGradeSaturationScale = 1.0f;
#endif
    ColorGradePoint         m_colorGradePoint[2];
        

    /** If false, skips all processing and just blits to the output.

        Defaults to true.*/
    bool                    m_effectsEnabled;


public:
    
    FilmSettings();

    /**
       \code
       FilmSettings {
         gamma = <number>;
         sensitivity = <number>;
         bloomStrength = <number>;
         bloomRadiusFraction = <number>;
         temporalAntialiasingEnabled = <boolean>;
         antialiasingEnabled = <boolean>;
         antialiasingFilterRadius = <number>;
         antialiasingHighQuality = <boolean>;
         vignetteTopStrength = <number>;
         vignetteBottomStrength = <number>;
         vignetteSizeFraction = <number>;
         toneCurve = IDENTITY | CONTRAST | CELLULOID | SUPERBOOST | SATURATE | BURNOUT | NEGATIVE | spline;
         debugZoom = <number>;
         effectsEnabled = <boolean>;
         invertX = <boolean>;
         invertY = <boolean>;
         upscaleFilter = NEAREST | BILINEAR | BICUBIC | BICUBIC_SHARPER;
         downscaleFilter = NEAREST | BILINEAR | BICUBIC | BICUBIC_SHARPER;
       }
       \endcode
     */
    FilmSettings(const Any& any);

    Any toAny() const;

    ResampleFilter upscaleFilter() const {
        return m_upscaleFilter;
    }

    ResampleFilter downscaleFilter() const {
        return m_downscaleFilter;
    }

    void setUpscaleFilter(ResampleFilter f) {
        m_upscaleFilter = f;
    }

    void setDownscaleFilter(ResampleFilter f) {
        m_downscaleFilter = f;
    }

    /** If > 1, enlarge pixels by this amount relative to the center of the screen for aid in debugging. 
        Enabling debugZoom may compromise performance. */
    int debugZoom() const {
        return m_debugZoom;
    }

    void setDebugZoom(int z) {
        alwaysAssertM(z > 0, "Zoom cannot be negative");
        m_debugZoom = z;
    }

    const Spline<float>& toneCurve() const {
        return m_toneCurve;
    }

    /** For use when targeting optically-inverted displays such as rear projectors, or other special cases. */
    bool invertX() const {
        return m_invertX;
    }

    void setInvertX(bool b) {
        m_invertX = b;
    }

    /** For use when targeting optically-inverted displays or other special cases. */
    bool invertY() const {
        return m_invertY;
    }

    void setInvertY(bool b) {
        m_invertY = b;
    }

    /** If true, only compute post-processing within a disk, for VR headsets. The exact disk radius
        is different for each framebuffer within the Film stack pass and has
        been tuned to look good on all VR headsets.
         
        This parameter must be set at runtime. It is not persisted to any files.  
      */
    bool diskFramebuffer() const {
        return m_diskFramebuffer;
    }
    
    /** \sa diskFramebuffer */
    void setDiskFramebuffer(bool b) {
        m_diskFramebuffer = b;
    }

    /** Amount of darkness due to vignetting for the top of the screen, on the range [0, 1] */
    float vignetteTopStrength() const {
        return m_vignetteTopStrength;
    }

    void setVignetteTopStrength(float s) {
        m_vignetteTopStrength = s;
    }
    
    void setVignetteBottomStrength(float s) {
        m_vignetteBottomStrength = s;
    }

    void setVignetteSizeFraction(float s) {
        m_vignetteSizeFraction = s;
    }

    /** Amount of darkness due to vignetting for the bottom of the screen, on the range [0, 1] */
    float vignetteBottomStrength() const {
        return m_vignetteBottomStrength;
    }

    /** Fraction of the diagonal radius of the screen covered by vignette, on the range [0, 1] */
    float vignetteSizeFraction() const {
        return m_vignetteSizeFraction;
    }

    /** \brief Monitor gamma used in tone-mapping. Default is 2.0. */
    float gamma() const {
        return m_gamma;
    }

    /** \brief Scale factor applied to the pixel values during exposeAndRender() */
     float sensitivity() const {
        return m_sensitivity;
    }

    /** \brief 0 = no bloom, 1 = blurred out image. */
    float bloomStrength() const {
        return m_bloomStrength;
    }

    /** \brief Bloom filter kernel radius as a fraction 
     of the larger of image width/height.*/
    float bloomRadiusFraction() const {
        return m_bloomRadiusFraction;
    }

    /** Enabled screen-space antialiasing post-processing. This reduces the
     artifacts from undersampling edges but may blur textures.
    By default, this is disabled.

    The antialiasing algorithm is "FXAA 13", which is a modified
    version of Timothy Lottes' FXAA 11 and 12 algorithms.
    */
    void setAntialiasingEnabled(bool e) {
        m_antialiasingEnabled = e;
    }

    bool temporalAntialiasingEnabled() const {
        return m_temporalAntialiasingEnabled;
    }

    void setTemporalAntialiasingEnabled(bool b) {
        m_temporalAntialiasingEnabled = b;
    }

    bool antialiasingEnabled() const {
        return m_antialiasingEnabled;
    }

    float antialiasingFilterRadius() const {
        return m_antialiasingFilterRadius;
    }

    void setAntialiasingHighQuality(bool b) {
        m_antialiasingHighQuality = b;
    }

    bool antialiasingHighQuality() const {
        return m_antialiasingHighQuality;
    }

    /** 0 = FXAA within a pixel. Any larger value blurs taps that are separated from the center by f pixels*/
    void setAntialiasingFilterRadius(float f) {
        m_antialiasingFilterRadius = f;
    }

    void setGamma(float g) {
        m_gamma = g;
    }

    void setSensitivity(float s) {
        m_sensitivity = s;
    }

    void setBloomStrength(float s) {
        m_bloomStrength = s;
    }

    void setBloomRadiusFraction(float f) {
        m_bloomRadiusFraction = f;
    }

    /** If false, skips all processing and just blits to the output. */
    bool effectsEnabled() const {
        return m_effectsEnabled;
    }

    void setEffectsEnabled(bool b) {
        m_effectsEnabled = b;
    }

    /** Adds controls for these settings to the specified GuiPane. */
    void makeGui
    (class GuiPane*, 
     float maxSensitivity = 10.0f, 
     float sliderWidth    = 180, 
     float controlIndent  = 0.0f);

    void setIdentityToneCurve();
    void setCelluloidToneCurve();
    void setSuperboostToneCurve();
    void setSaturateToneCurve();
    void setContrastToneCurve();
    void setBurnoutToneCurve();
    void setNegativeToneCurve();

    /**
    Ensures the GBuffer specification has all the fields needed to render this effect
    \sa GApp::extendGBufferSpecification
    */
    void extendGBufferSpecification(GBuffer::Specification& spec) const;
};

}
