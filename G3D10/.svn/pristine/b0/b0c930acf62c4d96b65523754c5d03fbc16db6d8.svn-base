/**
  \file G3D-base.lib/include/G3D-base/constants.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_constants_h
#define G3D_constants_h

#include "G3D-base/platform.h"
#include "G3D-base/enumclass.h"
#include "G3D-base/Any.h"

namespace G3D {

/** These are defined to have the same value as the equivalent OpenGL
    constant. */
class PrimitiveType {
public:
    enum Value {
        POINTS         = 0x0000,
        LINES          = 0x0001,
        LINE_STRIP     = 0x0003, 
        TRIANGLES      = 0x0004, 
        TRIANGLE_STRIP = 0x0005,
        TRIANGLE_FAN   = 0x0006,
        QUADS          = 0x0007, 
        QUAD_STRIP     = 0x0008,
        PATCHES        = 0x000E
    };

private:
    
    static const char* toString(int i, Value& v);

    Value value;

public:

    G3D_DECLARE_ENUM_CLASS_METHODS(PrimitiveType);
};


/** Values for UniversalSurface::GPUGeom::refractionHint. */
G3D_DECLARE_ENUM_CLASS(RefractionHint,
        /** No refraction; a translucent object will appear as if it had the same index of refraction
            as the surrounding medium and objects will be undistorted in the background. */
        NONE, 

        /** Use a static environment map (cube or paraboloid) for computing transmissivity.*/
        //STATIC_PROBE, 

        /** Use a dynamically rendered 2D environment map; distort the background.  This looks good for many scenes
            but avoids the cost of rendering a cube map for DYNAMIC_ENV. UniversalSurface considers this hint
            to mean that blending is not required, so G3D::Renderer::cullAndSort will categorize
            UniversalSurface with this hint as opaque and a G3D::Renderer will send them to the RenderPassType::UNBLENDED_SCREEN_SPACE_REFRACTION_SAMPLES pass.
            */
        DYNAMIC_FLAT,

        /** Combine DYNAMIC_FLAT mode with order-independent transparency. UniversalSurface considers this hint to
            require blending, so G3D::Renderer::cullAndSort will categorize
            send UniversalSurface with this hint as transparent and a Renderer
            will then send them to the RenderPassType::SINGLE_PASS_UNORDERED_BLENDED_SAMPLES pass. 
            \sa DefaultRenderer::setOrderIndependentTransparency
            \sa DefaultRenderer::cullAndSort
        */
        DYNAMIC_FLAT_OIT,

        /** Use a dynamically rendered 2D environment map that is re-captured per transparent object.  This works well
            for transparent objects that are separated by a significant camera space z distance but overlap in screen space.*/
        //DYNAMIC_FLAT_MULTILAYER,

        /** Render a dynamic environment map */
        //DYNAMIC_PROBE, 

        /** True ray tracing. */
        RAY_TRACE);


/** Values for UniversalSurface::GPUGeom::mirrorHint. */
G3D_DECLARE_ENUM_CLASS(MirrorQuality,
        /** Reflections are black */
        NONE, 
        
        /** Use a static environment map.  This is what most games use */
        STATIC_PROBE,
        
        /** Use a screen-space hack to approximate reflection */
        SCREEN_SPACE,

        /** Render a dynamic environment map. */
        DYNAMIC_PROBE, 
        
        /** True ray tracing. */
        RAY_TRACE);


/** \brief How the alpha value should be interpreted for partial coverage. 

    This must be kept in sync with AlphaFilter.glsl
    \sa UniversalMaterial */
G3D_DECLARE_ENUM_CLASS(AlphaFilter,
    /** Used by UniversalMaterial to specify a default behavior:
        
        - use BLEND for surfaces with some 0.0 < alpha < 1.0
        - use ONE for surfaces with all alpha = 1.0. 
        - use BINARY for surfaces with both alpha = 0 and alpha = 1 at locations, but no intermediate values
    */
    DETECT,

    /** Treat the surface as completely covered, forcing alpha = 1.0 everywhere */
    ONE,

    /** Alpha is rounded per sample before being applied. Alpha >= 0.5 is rendered, alpha < 0.5 is discarded. 
        This enables faster deferred shading and more accurate post-processing because the surface can be exactly represented in the 
        GBuffer. It creates some aliasing on surface edges, however. */
    BINARY,

    /** Convert alpha to coverage values using
        <code>glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB)</code>.
        Requires a MSAA framebuffer to be bound.

        Not currently supported by UniversalSurface.
           
        See http://www.dhpoware.com/samples/glMultiSampleAntiAliasing.html for an example.*/
    COVERAGE_MASK,
        
    /** Render surfaces with partial coverage from back to front,
        using Porter and Duff&apos;s OVER operator.  This leaves the
        depth buffer inconsistent with the color buffer and
        requires a sort, but often gives the best appearance. 
            
        Even surfaces with a binary alpha cutout can benefit from
        blending because the transition under minification or magnification
        between alpha = 0 and alpha = 1 will create fractional values.*/
    BLEND
);


/**
Only used for depth writing purposes by G3D::Surface.

- REJECT_TRANSPARENCY The surface must discard alpha less than 1. Same as `glAlphaFunc(GL_GEQUAL, 1.0f)`. Used for depth prepass, and Williams shadow maps when SVSM is enabled.
- STOCHASTIC The surface may perform stochastic alpha testing or use any threshold value that it wishes. Used for Williams shadow maps when SVSM is disabled.
- STOCHASTIC_REJECT_NONTRANSPARENT The surface may perform stochastic alpha test but must discard on alpha = 1. Used for SVSM generation.
*/
G3D_DECLARE_ENUM_CLASS(TransparencyTestMode,
    REJECT_TRANSPARENCY,
    STOCHASTIC,
    STOCHASTIC_REJECT_NONTRANSPARENT);

/**
    "Transparency" in G3D = transmission + alpha coverage

    - NONE:    alphaFilter(alpha) * (1 - transmission) is always 1
    - BINARY:  alphaFilter(alpha) * (1 - transmission) is always 1 or 0
    - SOME:    alphaFilter(alpha) * (1 - transmission) is less than one somewhere in this surface and 1 somewhere else.
    - ALL:     alphaFilter(alpha) * (1 - transmission) is always less than 1 (there are no nontransparent texels in this surface)

    Used by G3D::Surface.
*/
G3D_DECLARE_ENUM_CLASS(TransparencyType,
    NONE,
    BINARY,
    SOME,
    ALL);


/**
  \brief Used by G3D::Surface and G3D::Renderer to specify the kind of rendering pass.
*/
G3D_DECLARE_ENUM_CLASS(
    RenderPassType, 

    /** Write to the depth buffer, only render 100% coverage, non-transmission samples, no blending allowed. */
    OPAQUE_SAMPLES,

    /** Samples that require screen-space refraction information, and so must be rendered after the usual
        opaque pass. This pass is only for non-OIT refraction. */
    UNBLENDED_SCREEN_SPACE_REFRACTION_SAMPLES,

    /** Do not write to the depth buffer. Only blended samples allowed. Use RenderDevice::DEPTH_LESS 
        to prevent writing to samples from the same surface that were opaque and already colored by previous 
        passes.

        Only a single pass per surface is allowed. Do not modify the current blend mode on the RenderDevice,
        which has been configured to work with a specific output macro. Surfaces need not be submitted in order.        
        
        \sa RefractionHint::DYNAMIC_FLAT_OIT
        \sa DefaultRenderer::setOrderIndependentTransparency
        \sa DefaultRenderer::cullAndSort
        \sa GApp::Settings::RendererSettings::orderIndependentTransparency
     */
    SINGLE_PASS_UNORDERED_BLENDED_SAMPLES,
    
    /** Do not write to the depth buffer. Only blended samples allowed. Use RenderDevice::DEPTH_LESS 
        to prevent writing to samples from the same surface that were opaque and already colored by previous 
        passes. 
        
        Multiple passes over each surface are allowed, for example, to execute colored transmission.
        Surfaces (and ideally, triangles within them) should be submitted in back-to-front order.

        */
    MULTIPASS_BLENDED_SAMPLES,

    /* Generic shadow map pass */
    SHADOW_MAP,

    /* Only render opaque surfaces to the shadow map */
    OPAQUE_SHADOW_MAP,

    /* Only render transparent surfaces (stochastically) to the shadow map */
    TRANSPARENT_SHADOW_MAP,

    /* Render transparent surfaces as if they were opaque. Useful for depth peeling or primary 
       ray optimizations */
    TRANSPARENT_AS_OPAQUE
    );

/** Values for FilmSettings::upscaleFilter and FilmSettings::downscaleFilter */
G3D_DECLARE_ENUM_CLASS(ResampleFilter,
    NEAREST,
    BILINEAR,
    BICUBIC,
    BICUBIC_SHARPER);

} // namespace G3D

G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::PrimitiveType)
G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::RefractionHint)
G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::MirrorQuality)
G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::AlphaFilter)
G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::ResampleFilter)

/** Per-surface mask allowing it to be ignored by some ray types.
In OptiX, this is limited to 8 bits. We use the first two for
static and dynamic objects and reserve the rest for future use.
*/
struct RenderMask {
	uint32_t          value = 0;
	static const uint32_t STATIC_GEOMETRY = 1;
	static const uint32_t DYNAMIC_GEOMETRY = 2;
	static const uint32_t ALL_GEOMETRY = 0xFFFFFFFF;
	RenderMask(const uint32_t value = 0) : value(value) {}
	operator uint32_t() const {
		return value;
	}
};

#endif

