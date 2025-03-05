/**
  \file G3D-app.lib/include/G3D-app/AmbientOcclusion.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#ifndef G3D_app_AmbientOcclusion_h
#define G3D_app_AmbientOcclusion_h

#include "G3D-base/platform.h"
#include "G3D-base/Array.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/Vector4.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-app/AmbientOcclusionSettings.h"
#include "G3D-app/TemporalFilter.h"

namespace G3D {

class Camera;
class CoordinateFrame;
class Framebuffer;
class RenderDevice;
class Texture;

/**
 \brief Screen-space ambient obscurance.

  Create one instance of AmbientObscurance per viewport or Framebuffer rendered in the frame.  Otherwise
  every update() call will trigger significant texture reallocation.


 \author Morgan McGuire and Michael Mara, NVIDIA and Williams College, 
 http://research.nvidia.com, http://graphics.cs.williams.edu 
*/
class AmbientOcclusion : public ReferenceCountedObject {
protected:  

    class PerViewBuffers {
    public:
        /** Stores camera-space (negative) linear z values at various scales in the MIP levels */
        shared_ptr<Texture>                      cszBuffer;
        
        // buffer[i] is used for MIP level i
        Array< shared_ptr<Framebuffer> >         cszFramebuffers;
        void resizeBuffers(const String& name, shared_ptr<Texture>   texture, const shared_ptr<Texture>&   peeledTexture, const AmbientOcclusionSettings::ZStorage& zStorage);
        static shared_ptr<PerViewBuffers> create();
        PerViewBuffers() {}
    };

    /** Used for debugging and visualization purposes */
    String                                   m_name;

    // These must be initialized where used instead of in the constructor
    // so that subclasses can replace the name
    shared_ptr<Shader>                       m_minifyShader;
    shared_ptr<Shader>                       m_aoShader;

    /** Prefix for the shaders. Default is "AmbientOcclusion_". This is useful when subclassing 
        AmbientOcclusion to avoid a conflict with the default shaders. */
    String                                   m_shaderFilenamePrefix;

    shared_ptr<Framebuffer>                  m_resultFramebuffer;
    shared_ptr<Texture>                      m_resultBuffer;

    /** As of the last call to update.  This is either m_resultBuffer or Texture::white() */
    shared_ptr<Texture>                      m_texture;

    /** For raw and temporally-filtered buffers */
    int                                      m_guardBandSize;

    /** For spatially blurred and output buffers */
    int                                      m_outputGuardBandSize;

    /** For now, can only be 1 or 2 in size */
    Array<shared_ptr<PerViewBuffers> >       m_perViewBuffers;

    /** Has AO in R and depth in G * 256 + B.*/
    shared_ptr<Texture>                      m_rawAOBuffer;
    shared_ptr<Framebuffer>                  m_rawAOFramebuffer;

    /** Has AO in R and depth in G * 256 + B.*/
    shared_ptr<Texture>                      m_temporallyFilteredBuffer;

    /** Has AO in R and depth in G */
    shared_ptr<Texture>                      m_hBlurredBuffer;
    shared_ptr<Framebuffer>                  m_hBlurredFramebuffer;

    /** If normals enabled, RGBA8, RG is CSZ, and BA is normal in Oct16 */
    shared_ptr<Framebuffer>                  m_packedKeyBuffer;

    TemporalFilter                           m_temporalFilter;

    /** Appended to all Args for shader passes run by this class. 
    
        Useful for prototyping minor variations; simply inherit from this class, 
        modify the shaders and add any new uniforms/macros required here.
        Note that because of the inherent slowness of iterating over hash tables,
        such a modification is not as performant as possible.  
        */
    shared_ptr<UniformTable>                 m_uniformTable;

    /** Creates the per view buffers if necessary */
    void initializePerViewBuffers(int size);

    void resizeBuffers(const shared_ptr<Texture>& depthTexture, bool packKey);

    void packBlurKeys
        (RenderDevice* rd,
            const AmbientOcclusionSettings& settings,
            const shared_ptr<Texture>& depthBuffer,
            const Vector3& clipInfo,
        const float farPlaneZ,
            const shared_ptr<Texture>& normalBuffer);

    void computeCSZ
       (RenderDevice*                       rd,   
        const Array<shared_ptr<Framebuffer> >& cszFramebuffers,
        const shared_ptr<Texture>&          csZBuffer,
        const AmbientOcclusionSettings&     settings, 
        const shared_ptr<Texture>&          depthBuffer, 
        const Vector3&                      clipInfo,
        const shared_ptr<Texture>&          peeledDepthBuffer);

    virtual void computeRawAO
       (RenderDevice* rd,         
        const AmbientOcclusionSettings&     settings, 
        const shared_ptr<Texture>&          depthBuffer, 
        const Vector3&                      clipConstant,
        const Vector4&                      projConstant,
        float                               projScale,
        const float                         farPlaneZ,
        const shared_ptr<Texture>&          csZBuffer,
        const shared_ptr<Texture>&          peeledCSZBuffer = shared_ptr<Texture>(),
        const shared_ptr<Texture>&          normalBuffer = shared_ptr<Texture>());

    /** normalBuffer and normalReadScaleBias are only used if settings.useNormalsInBlur is true and normalBuffer is non-null.
        projConstant is only used if settings.useNormalsInBlur is true and normalBuffer is null */
    void blurHorizontal
        (RenderDevice*                      rd, 
        const AmbientOcclusionSettings&     settings, 
        const shared_ptr<Texture>&          depthBuffer,
    const float                         farPlaneZ,
        const Vector4&                      projConstant = Vector4::zero(),
        const shared_ptr<Texture>&          normalBuffer = shared_ptr<Texture>());

    /** normalBuffer and normalReadScaleBias are only used if settings.useNormalsInBlur is true and normalBuffer is non-null.
        projConstant is only used if settings.useNormalsInBlur is true and normalBuffer is null */
    void blurVertical
       (RenderDevice*                       rd, 
        const AmbientOcclusionSettings&     settings, 
        const shared_ptr<Texture>&          depthBuffer,
    const float                         farPlaneZ,
    const Vector4&                      projConstant = Vector4::zero(),
        const shared_ptr<Texture>&          normalBuffer = shared_ptr<Texture>());

    /** Shared code for the vertical and horizontal blur passes */
    void blurOneDirection
       (RenderDevice*                       rd,
        const AmbientOcclusionSettings&     settings,
        const shared_ptr<Texture>&          depthBuffer,
    const float                         farPlaneZ,
        const Vector4&                      projConstant,
        const shared_ptr<Texture>&          normalBuffer,
        const Vector2int16&                 axis,
        const shared_ptr<Framebuffer>&      framebuffer,
        const shared_ptr<Texture>&          source);

    /**
     \brief Render the obscurance constant at each pixel to the currently-bound framebuffer.

     \param rd The rendering device/graphics context.  The currently-bound framebuffer must
     match the dimensions of \a depthBuffer.

     \param settings See G3D::AmbientOcclusionSettings.

     \param depthBuffer Standard hyperbolic depth buffer.  Can
      be from either an infinite or finite far plane
      depending on the values in projConstant and clipConstant.

     \param clipConstant Constants based on clipping planes:
     \code
        const double width  = rd->width();
        const double height = rd->height();
        const double z_f    = camera->farPlaneZ();
        const double z_n    = camera->nearPlaneZ();

        const Vector3& clipConstant = 
            (z_f == -inf()) ? 
                Vector3(float(z_n), -1.0f, 1.0f) : 
                Vector3(float(z_n * z_f),  float(z_n - z_f),  float(z_f));
     \endcode

     \param projConstant Constants based on the projection matrix:
     \code
        Matrix4 P;
        camera->getProjectUnitMatrix(rd->viewport(), P);
        const Vector4 projConstant
            (float(-2.0 / (width * P[0][0])), 
             float(-2.0 / (height * P[1][1])),
             float((1.0 - (double)P[0][2]) / P[0][0]), 
             float((1.0 + (double)P[1][2]) / P[1][1]));
     \endcode

     \param projScale Pixels-per-meter at z=-1, e.g., computed by:
     
      \code
       -height / (2.0 * tan(verticalFieldOfView * 0.5))
      \endcode

     This is usually around 500.


     \param peeledDepthBuffer An optional peeled depth texture, rendered from the same viewpoint as the depthBuffer,
      but not necessarily with the same resolution.
     */
    void compute
    (RenderDevice*                   rd,
     const AmbientOcclusionSettings& settings,
     const shared_ptr<Texture>&      depthBuffer, 
     const Vector3&                  clipConstant,
     const Vector4&                  projConstant,
     float                           projScale,
     const float                     farPlaneZ,
     const CoordinateFrame&          currentCameraFrame,
     const CoordinateFrame&          prevCameraFrame,
     const shared_ptr<Texture>&      peeledDepthBuffer = nullptr,
     const shared_ptr<Texture>&      normalBuffer      = nullptr,
     const shared_ptr<Texture>&      ssVelocityBuffer  = nullptr);

    /** \brief Convenience wrapper for the full version of compute().

        \param camera The camera that the scene was rendered with.
    */
    void compute
       (RenderDevice*                   rd,
        const AmbientOcclusionSettings& settings,
        const shared_ptr<Texture>&      depthBuffer, 
        const shared_ptr<Camera>&       camera,
        const shared_ptr<Texture>&      peeledDepthBuffer = nullptr,
        const shared_ptr<Texture>&      normalBuffer      = nullptr,
        const shared_ptr<Texture>&      ssVelocityBuffer  = nullptr);

    AmbientOcclusion(const String& name) : m_name(name), m_shaderFilenamePrefix("AmbientOcclusion_") {}

public:

    /** For debugging and visualization purposes */
    const String& name() const {
        return m_name;
    }

    /** \brief Create a new AmbientOcclusion instance. 
    
        Only one is ever needed, but if you are rendering to differently-sized
        framebuffers it is faster to create one instance per resolution than to
        constantly force AmbientOcclusion to resize its internal buffers. */
    static shared_ptr<AmbientOcclusion> create(const String& name = "G3D::AmbientOcclusion");

    /** Convenience method for resizing the AO texture from aoFramebuffer to match the size of its depth buffer and then
        computing AO from the depth buffer.  
        
        \param guardBandSize Required to be the same in both dimensions and non-negative
        \see texture */
    void update
    (RenderDevice*                   rd, 
     const AmbientOcclusionSettings& settings, 
     const shared_ptr<Camera>&       camera, 
     const shared_ptr<Texture>&      depthTexture,
     const shared_ptr<Texture>&      peeledDepthBuffer   = nullptr,  
     const shared_ptr<Texture>&      normalBuffer        = nullptr,
     const shared_ptr<Texture>&      ssVelocityBuffer    = nullptr,
     const Vector2int16              guardBandSize       = Vector2int16(0, 0));   

    /**
       Returns the ao buffer texture, or Texture::white() if AO is disabled or 
       supported on this GPU. Modulate indirect illumination by this */
    shared_ptr<Texture> texture() const {
        return m_texture;
    }

    /**
        Binds:
            sampler2D   <prefix>##buffer;
            ivec2       <prefix>##offset;
            #define     <prefix>##notNull 1;
    to \a args.
    */
    void setShaderArgs(UniformTable& args, const String& prefix = "ambientOcclusion_", const Sampler& sampler = Sampler::buffer());

    /** Returns false if this graphics card is known to perform AmbientOcclusion abnormally slowly */
    static bool supported();

};

} // namespace GLG3D

#endif // AmbientOcclusion_h
