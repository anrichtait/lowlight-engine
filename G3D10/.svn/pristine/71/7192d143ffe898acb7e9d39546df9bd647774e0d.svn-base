/**
  \file G3D-app.lib/include/G3D-app/DefaultRenderer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_DefaultRenderer_h

#include "G3D-base/platform.h"
#include "G3D-app/Renderer.h"
#include "G3D-app/DDGIVolume.h"
#include "G3D-app/GaussianMIPFilter.h"

namespace G3D {

class Camera;
class Texture;
class Args;
class Shader;

/** \brief Supports both traditional forward shading and full-screen deferred shading.

    The basic rendering algorithm is:

\code
Renderer::render(all) {
    visible, requireForward, requireBlended = cullAndSort(all)
    renderGBuffer(visible)
    computeShadowing(all)
    if (deferredShading()) { renderIndirectIllumination(); renderDeferredShading()  }
    renderOpaqueSamples(deferredShading() ? requireForward : visible)
    lighting.updateColorImage() // For the next frame
    renderOpaqueScreenSpaceRefractingSamples(deferredShading() ? requireForward : visible)
    renderBlendedSamples(requireBlended, transparencyMode)
}
\endcode

    The DefaultRenderer::renderDeferredShading() pass uses whatever properties are available in the
    GBuffer, which are controlled by the GBufferSpecification. For most applications,
    it is necessary to enable the lambertian, glossy, camera-space normal,
    and emissive fields to produce good results. If the current GBuffer specification
    does not contain sufficient fields, most of the surfaces will take the fallback
    forward shading pass at reduced performance.

    \sa GApp::m_renderer, G3D::RenderDevice, G3D::Surface
*/
class DefaultRenderer : public Renderer {
protected:

    /** For computing blurred mirror reflections to approximate gathered
        glossy reflections. The mip levels are progressively blurred versions. */
    shared_ptr<Framebuffer>                 m_blurredMIPReflectionFramebuffer;

    /** Only used when half-resolution glossy is in effect. Otherwise, we render straight into
        m_blurredMIPReflectionFramebuffer*/
    shared_ptr<Framebuffer>                 m_shadedMirrorRaysFramebuffer;

    int                                     m_glossyYScale = 2;

    /** If true, ray trace glossy reflections. */
    bool                                    m_traceGlossyReflections = true;
    
    /** Specifies the mip level at which to sample textures
    for irradiance and glossy rays.*/
    int                                     m_diffuseMipLevel = 8;
    int                                     m_glossyMipLevel = 3;

    /** Number of frames to spend initializing probes if any volume has uninitialized probes.
        Triggered on scene load, volume creation, and leap-frogging on a camera-locked volume. */
    int                                     m_numInitializationFrames = 0;
    bool									m_shouldClearUninitializedProbes = false;
    bool                                    m_newlyXProbes = false;

    shared_ptr<GaussianMIPFilter>          m_gaussianMIPFilter;

    shared_ptr<GBuffer>                     m_reflectionGBuffer;

    /** Textures storing ray origins and directions for irradiance probe sampling,
    regenerated every frame and then split between all probes according to a given heuristic. */
    shared_ptr<Texture>                     m_irradianceRayOriginsTexture;
    shared_ptr<Texture>                     m_irradianceRayDirectionsTexture;

    /** Ray textures for reflection rays. */
    shared_ptr<Texture>                     m_reflectionRayOriginsTexture;
    shared_ptr<Texture>                     m_reflectionRayDirectionsTexture;

    shared_ptr<Framebuffer>                 m_irradianceRaysShadedFB;
    shared_ptr<GBuffer>                     m_irradianceRaysGBuffer;


    Table<shared_ptr<Texture>, shared_ptr<GLPixelTransferBuffer>> m_rayOriginsAndDirectionsTable;
    using PBOGBuffer = shared_ptr<GLPixelTransferBuffer>[5];
    PBOGBuffer                              m_pboGBuffer;

    /** Ray traced diffuse global illumination using DDGI. */
    bool                                    m_enableDiffuseGI = false;

    /** Renders the glossy pass for primary rays each frame. */
    bool                                    m_enableGlossyGI = false;

    bool                                    m_enableProbeRayRandomRotation = true;


    /** Trace an arbitrary buffer of rays to fill a GBuffer. */
    void sampleArbitraryRays
    (const shared_ptr<Texture>& rayOrigins,
        const shared_ptr<Texture>& rayDirections,
        const shared_ptr<TriTree>& tritree,
        const shared_ptr<GBuffer>& gbuffer,
        const int                                   totalRays,
        const unsigned int							visibilityMask,
        const int mipLevel = 0);

    /** Run the deferred shader on a gbuffer of arbitrary ray data. */
    virtual void shadeArbitraryRays
    (RenderDevice* rd,
        const Array<shared_ptr<Surface>>&   surfaceArray,
        const shared_ptr<Framebuffer>&      targetFramebuffer,
        const LightingEnvironment&          environment,
        const shared_ptr<Texture>&          rayOrigins,
        const shared_ptr<Texture>&          rayDirections,
        const shared_ptr<GBuffer>&          gbuffer,
        const bool                          useProbeIndirect,
        const bool                          glossyToMatte);

    void resizeReflectionRayTextures(const int screenWidth, const int screenHeight);
    void resizeIrradianceRayTextures(const int raysPerProbe = -1);

    /** Generate rays for diffuse irradiance. Uses DDGIVolume_generateRays.glc. */
    void generateIrradianceRays(RenderDevice* rd, const int offset, const shared_ptr<DDGIVolume>& ddgiVolume, int& numGeneratedRays, const int raysPerProbe);

    /** \param numGlossyRays Returns the number of rays generated; needed by generateIrradianceRays */
    void generateMirrorRays(RenderDevice* rd, const shared_ptr<GBuffer>& primaryGBuffer, int& numGlossyRays);

    /** e.g., "DefaultRenderer" used for switching the shaders loaded by subclasses. */
    String                      m_shaderName;

    /** e.g., "G3D::DefaultRenderer::" used for switching the shaders loaded by subclasses. */
    String                      m_textureNamePrefix;

    bool                        m_deferredShading;
    bool                        m_orderIndependentTransparency;

    /**
        Hi-res pixels per low-res pixel, along one dimension.
        (1 is identical resolution, 4 would be quarter-res,
        which is 1/16 the number of pixels).

        Default is 4.

        Set to 1 to disable low resolution OIT.
    */
    int                         m_oitLowResDownsampleFactor;

    /** Default is 1 */
    int                         m_oitUpsampleFilterRadius;

    /** If true, all OIT buffers will be in 32-bit floating point.
        Default is false. */
    bool                        m_oitHighPrecision;

    /** For the transparent surface pass of the OIT algorithm.
        Shares the depth buffer with the main framebuffer. The
        subsequent compositing pass uses the regular framebuffer in 2D mode. 
        
        This framebuffer has several color render targets bound. For details, see:

        McGuire and Mara, A Phenomenological Scattering Model for Order-Independent Transparency, I3D'16
        http://graphics.cs.williams.edu/papers/TransparencyI3D16/

        It shares the depth with the original framebuffer but does not write to it.
    */
    shared_ptr<Framebuffer>     m_oitFramebuffer;
    
    /** A low resolution version of m_oitFramebuffer. */
    shared_ptr<Framebuffer>     m_oitLowResFramebuffer;

    /** Used for resampling normals during computeLowResDepthAndNormals for later upsampling under OIT. Has a single
        RG8_SNORM texture that is camera-space octahedrally encoded normals. */
    shared_ptr<Framebuffer>     m_csOctLowResNormalFramebuffer;

    /** Captured image of the background used for blurs for OIT */ 
    shared_ptr<Framebuffer>     m_backgroundFramebuffer;
    
    /** Because subclasses can change the shader filename prefix, we must use
        member variables instead of the static variables created by LAUNCH_SHADER
        to store the shaders. These are loaded just before use. */
    shared_ptr<Shader>          m_deferredShader;
    shared_ptr<Shader>          m_upsampleOITShader;
    shared_ptr<Shader>          m_compositeOITShader;

    /** Loaded by the constructor, but subclasses may replace it in their own constructors.

        The default implementation is Weighted-Blended Order Independent Transparency by
        McGuire and Bavoil. This string can be overwritten to implement alternative algorithms,
        such as Adaptive Transparency. However, new buffers may need to be set by overriding
        renderOrderIndependentBlendedSamples() for certain algorithms.
    */
    String                      m_oitWriteDeclarationGLSLFilename;

    virtual void renderDeferredShading
       (RenderDevice*                       rd, 
        const Array<shared_ptr<Surface> >&  sortedVisibleSurfaceArray, 
        const shared_ptr<GBuffer>&          gbuffer,
        const LightingEnvironment&          environment);

    /** Subclasses that can compute global illumination to deferred shading buffers should override this method,
        which is invoked before renderDeferredShading. */
    virtual void renderIndirectIllumination
    (RenderDevice* rd,
        const Array<shared_ptr<Surface> >& sortedVisibleSurfaceArray,
        const shared_ptr<GBuffer>& gbuffer,
        const LightingEnvironment& environment,
        const shared_ptr<TriTree>& tritree = nullptr);
    
    /** Called by DefaultRenderer::renderDeferredShading to configure the inputs to deferred shading. */
    virtual void setDeferredShadingArgs(
        Args&                               args, 
        const shared_ptr<GBuffer>&          gbuffer, 
        const LightingEnvironment&          environment);

    virtual void renderOpaqueSamples
       (RenderDevice*                       rd, 
        Array<shared_ptr<Surface> >&        surfaceArray, 
        const shared_ptr<GBuffer>&          gbuffer, 
        const LightingEnvironment&          environment);

    virtual void renderOpaqueScreenSpaceRefractingSamples
       (RenderDevice*                       rd, 
        Array<shared_ptr<Surface> >&        surfaceArray, 
        const shared_ptr<GBuffer>&          gbuffer, 
        const LightingEnvironment&          environment);

    virtual void renderSortedBlendedSamples       
        (RenderDevice*                      rd, 
        Array<shared_ptr<Surface> >&        surfaceArray, 
        const shared_ptr<GBuffer>&          gbuffer, 
        const LightingEnvironment&          environment);

    virtual void renderOrderIndependentBlendedSamples       
       (RenderDevice*                       rd, 
        Array<shared_ptr<Surface> >&        surfaceArray, 
        const shared_ptr<GBuffer>&          gbuffer, 
        const LightingEnvironment&          environment);

    virtual void allocateAllOITBuffers
       (RenderDevice*                       rd, 
        bool                                highPrecision = false);

    /** Called once for the high res buffer and once for the low res one from allocateAllOITBuffers.
        \param w Desired width of this framebuffer 
        \param h Desired height of this framebuffer 
     */
    virtual void allocateOITFramebufferAttachments
       (RenderDevice*                       rd, 
        const shared_ptr<Framebuffer>&      oitFramebuffer, 
        int                                 w, 
        int                                 h, 
        bool                                highPrecision = false);

    virtual void clearAndRenderToOITFramebuffer
       (RenderDevice*                       rd,
        const shared_ptr<Framebuffer>&      oitFramebuffer,
        Array <shared_ptr<Surface>>&        surfaceArray,
        const shared_ptr<GBuffer>&          gbuffer,
        const LightingEnvironment&          environment);

    /** For OIT */
    virtual void resizeOITBuffersIfNeeded
       (const int                           width,
        const int                           height, 
        const int                           lowResWidth,
        const int                           lowResHeight);

    /**
      For OIT
      \param csNormalTexture May be nullptr
    */
    virtual void computeLowResDepthAndNormals
       (RenderDevice*                       rd, 
        const shared_ptr<Texture>&          csHighResNormalTexture);
    
    DefaultRenderer(const String& className = "DefaultRenderer", const String& namespacePrefix = "G3D::");

public:

    // Named constants for the number of frames to initialize.
    static const int CAMERA_TRACK = 2;
    static const int SCENE_INIT = 5;

    /** For creating the diffuse irradiance probe volume(s).
        Volumes at varying grid resolutions store irradiance (RGB10A2) and
        mean distance/squared-distance (RG16F). When enabled, these volumes are
        updated using raytracing and queried during shading (for both raytracing
        and rasterization) for duffise global illumination. Details in:

        Majercik et al., Dynamic Diffuse Global Illumination with Ray-Traced Irradiance Fields, JCGT'19
        http://jcgt.org/published/0008/02/01/
    */
    virtual void createProbeVolumes(const AABox& sceneBounds, LightingEnvironment& environment, const shared_ptr<Camera>& camera);

    /** Resolve probe states and update the irradiance probe volume. */
    virtual void updateDiffuseGI(RenderDevice* rd,
        const shared_ptr<Scene>& scene,
        const shared_ptr<GBuffer>& primaryGBuffer,
        const shared_ptr<Camera>& camera);

    /** Update the probe data structure. Called multiple times from updateDiffuseGI
        to intialize different sets of probes in different states. */
    void traceAndUpdateProbes(
        RenderDevice* rd,
        const Array<shared_ptr<Surface>>& surfaceArray,
        const shared_ptr<TriTree>& tritree,
        const shared_ptr<GBuffer>& primaryGBuffer,
        const LightingEnvironment& environment,
        const int                              raysPerProbe,
        const unsigned int					   visibilityMask);

    /** Trace half-res rays to resolve glossy ilumination. If diffuse GI is enabled, 
        uses irradiance volume for second-order glossy reflections. */
    void traceGlossyIndirectIllumination
    (RenderDevice* rd,
        const Array<shared_ptr<Surface> >& sortedVisibleSurfaceArray,
        const shared_ptr<GBuffer>& gbuffer,
        const LightingEnvironment& environment,
        const shared_ptr<TriTree>& tritree = nullptr);

    /** If there are any probes in the uninitialized state, converge them. */
    void convergeUninitializedProbes(RenderDevice* rd, 
        const Array<shared_ptr<Surface>>& surfaceArray, 
        const shared_ptr<TriTree>& tritree, 
        const shared_ptr<GBuffer>& gbuffer, 
        const LightingEnvironment& lightingEnv);

    /** Compute bounds on dynamic objects to wake up sleeping probes. */
    void getDynamicObjectBounds(const Array<shared_ptr<Entity>>& sceneEntities, Array<AABox>& dynamicBounds);

    const shared_ptr<GBuffer>& irradianceGBuffer() {
        return m_irradianceRaysGBuffer;
    }

    const shared_ptr<GBuffer>& reflectionGBuffer() {
        return m_reflectionGBuffer;
    }

    AABox                                   m_bounds;

    Array<shared_ptr<DDGIVolume>>			m_ddgiVolumeArray;
    Array<bool>                             m_showProbeLocations;

    /** How much should the probes count when shading *themselves*? 1.0 preserves
    energy perfectly. Lower numbers compensate for small leaks/precision by avoiding
    recursive energy explosion. */
    float                                   m_energyPreservation = 0.95f;

    int									    m_raysThisFrame = 0;

    shared_ptr<Framebuffer>                 m_glossyGIFramebuffer;

    void setReflectionTexture(const shared_ptr<GLPixelTransferBuffer>& pbo) {
        // Takes a pbo to avoid the const_cast
        m_blurredMIPReflectionFramebuffer->texture(0)->update(pbo);
    }

    const shared_ptr<Texture>& reflectionTexture() {
        return m_blurredMIPReflectionFramebuffer->texture(0);
    }

    void setEnableDiffuseGI(bool b) {
        m_enableDiffuseGI = b;
    }

    bool enableDiffuseGI() {
        return m_enableDiffuseGI;
    }
    void setEnableGlossyGI(bool b) {
        m_enableGlossyGI = b;
    }

    bool enableGlossyGI() {
        return m_enableGlossyGI;
    }

    void setEnableProbeRayRandomRotation(bool b) {
        m_enableProbeRayRandomRotation = b;
    }

    bool enableProbeRayRandomRotation() {
        return m_enableProbeRayRandomRotation;
    }

    bool traceGlossyReflections() {
        return m_traceGlossyReflections;
    }

    /** When everything is enabled, assuming looking at a fully glossy frame */
    float gRaysPerFrame() const {
        return float(m_raysThisFrame) * 1e-9f;
    }

    /** Fraction of rays cast that were for diffuse shading */
    float diffuseRayFraction() const {
        float numDiffuseRays = float(m_irradianceRayOriginsTexture->width() * m_irradianceRayOriginsTexture->height());
        return numDiffuseRays /
            (numDiffuseRays + m_reflectionRayOriginsTexture->width() * m_reflectionRayOriginsTexture->height());
    }

    void addVolumeInitializationFrames(int numInitializationFrames) {
        // This is the union of initialization requirements: it must be additive.
        m_numInitializationFrames += numInitializationFrames;
        m_shouldClearUninitializedProbes = true;
    }

    int initializationFrames() {
        return m_numInitializationFrames;
    }

    void setTraceGlossyReflections(bool b) {
        m_traceGlossyReflections = b;
    }

    void setGlossyYScale(int i) {
        m_glossyYScale = i;
    }

    int glossyYScale() const {
        return m_glossyYScale;
    }

    int diffuseMipLevel() const {
        return m_diffuseMipLevel;
    }
    void setDiffuseMipLevel(int i) {
        m_diffuseMipLevel = i;
    }
    int glossyMipLevel() const {
        return m_glossyMipLevel;
    }
    void setGlossyMipLevel(int i) {
        m_glossyMipLevel = i;
    }

    static shared_ptr<Renderer> create() {
        return createShared<DefaultRenderer>();
    }

    /** If true, use deferred shading on all surfaces that can be represented by the GBuffer.
        Default is false.
      */
    void setDeferredShading(bool b) {
        m_deferredShading = b;
    }

    bool deferredShading() const {
        return m_deferredShading;
    }

    /** If true, uses OIT.
        Default is false.
        
        The current implementation is based on:
        
        McGuire and Bavoil, Weighted Blended Order-Independent Transparency, Journal of Computer Graphics Techniques (JCGT), vol. 2, no. 2, 122--141, 2013
        Available online http://jcgt.org/published/0002/02/09/

        This can be turned on in both forward and deferred shading modes.

        This algorithm improves the quality of overlapping transparent surfaces for many scenes, eliminating popping and confusing appearance that can arise
        from imperfect sorting. It is especially helpful in scenes with lots of particles. This technique has relatively low overhead compared to alternative methods.
    */
    void setOrderIndependentTransparency(bool b) {
        m_orderIndependentTransparency = b;
    }

    bool orderIndependentTransparency() const {
        return m_orderIndependentTransparency;
    }

    virtual const String& className() const override{
        static const String n = "DefaultRenderer";
        return n;
    }

    virtual void render
       (RenderDevice*                                       rd,
        const shared_ptr<Camera>&                           camera,
        const shared_ptr<Framebuffer>&                      framebuffer,
        const shared_ptr<Framebuffer>&                      depthPeelFramebuffer,
        LightingEnvironment&                                lightingEnvironment,
        const shared_ptr<GBuffer>&                          gbuffer, 
        const Array<shared_ptr<Surface>>&                   allSurfaces,
        const  std::function<const shared_ptr<TriTree>& ()>& tritreeFunction = nullptr) override;
};

} // namespace
