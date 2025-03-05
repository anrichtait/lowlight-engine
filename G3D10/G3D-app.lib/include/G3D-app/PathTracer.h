/**
  \file G3D-app.lib/include/G3D-app/PathTracer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_PathTracer_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/Array.h"
#include "G3D-base/Ray.h"
#include "G3D-app/TriTree.h"

namespace G3D {

class Camera;
class Light;
class Surfel;
class Image;
class Scene;

class PathTracer : public ReferenceCountedObject {
public:

    class Options {
    public:
        /** Only used for traceImage. Default = 64 in Release mode and 1 in Debug mode.
            Not used by traceBuffer(). */
        int         raysPerPixel = 64;

        /** 1 = direct illumination. Default = 5 in Release mode and 2 in Debug mode. */
        int         maxScatteringEvents = 5;

        /** Prevent very low-probability caustic paths from increasing variance. Increase for more accuracy,
            decrease for less variance.  */
        Radiance    maxIncidentRadiance = 300.0f;

        /** Prevent low-probability glossy paths from receiving high weights. Increase for more accuracy,
            decrease for less variance. */
        float       maxImportanceSamplingWeight = 1.5f;

        /** If true, make the last scattering event a direct lookup against
            the Scene's environment map instead of an actual trace. Path tracing
            with a fixed maximum number of scattering events is biased to produce
            a value that is too dark (because it always misses some light).
            useEnvironmentMapForLastScatteringEvent = true can reduce this bias,
            leading to faster convergence if the environment map is accurate.

            Default = false.
          */
        bool        useEnvironmentMapForLastScatteringEvent = false;

        /** Default = true in Release mode and false in debug mode */
        bool        multithreaded = true;

        /** Huch energy should be sampled via direct illumination/shadow rays ("Next event estimation")
            vs. random indirect rays to emissive surfaces? 
            
            Extremes:
            - 1.0 = pure direct illumination (good for small sources and diffuse surfaces)
            - 0.0 = pure path tracing (good for large area lights and glossy surfaces)
            
            */
        float       areaLightDirectFraction = 0.7f;

        G3D_DECLARE_ENUM_CLASS(LightSamplingMethod,
            UNIFORM_AREA,
            STRATIFIED_AREA,
            LOW_DISCREPANCY_AREA,
            LOW_DISCREPANCY_SOLID_ANGLE
        );
        LightSamplingMethod samplingMethod = LightSamplingMethod::LOW_DISCREPANCY_SOLID_ANGLE;

        Options()
#       ifdef G3D_DEBUG
            : raysPerPixel(1),
            maxScatteringEvents(2),
            multithreaded(false)
#       endif
        {}
    };

protected:
    typedef Point2                              PixelCoord;

    
    /** Per-path data passed between major routines. Configured as a structure of arrays
        instead of an array of structures s othat the ray and surfel buffers can be directly
        passed to G3D::TriTree routines. */
    class BufferSet {
    public:
        Array<Ray>                              ray;

        /** How much the surfaces between the eye and the current path node
            have already modulated the contribution observed due to the BSDF.
            Initialized based on the number of rays per pixel. */
        Array<Color3>                           modulation;

        /** Surfels hit by primary and indirect rays (may be nullptr if each missed) */
        Array<shared_ptr<Surfel>>               surfel;

        /** Scattered radiance due to the selected light (which may be an emissive surface), IF
            it is visible: (B_j * |w_j . n| * f) / p_j
            The actual light position is implicitly encoded in the shadowRay. */
        Array<Radiance3>                        direct;

        /** Shadow rays corresponding to the direct values */
        Array<Ray>                              shadowRay;

        /** False if the light that created the direct value is visible */
        Array<bool>                             lightShadowed;

        /** True if the previous scattering event was an impulse (or primary ray). Light surfaces
            do not contribute to indirect light unless the previous event was an impulse. This
            avoids double-counting the lights. */
        Array<bool>                             impulseRay;
    
        /** Location in the output buffer to write the final radiance to.*/
        Array<int>                              outputIndex;

        /** Location in the output image to write the final radiance to.*/
        Array<PixelCoord>                       outputCoord;

        size_t size() const {
            return ray.size();
        }

        /** Does not resize outputIndex or outputCoord */
        void resize(size_t n) {
            ray.resize(n);
            modulation.resize(n);
            surfel.resize(n);
            direct.resize(n);
            shadowRay.resize(n);
            lightShadowed.resize(n);
            impulseRay.resize(n);
        }

        /** Removes element \a i from all arrays, including either outputIndex or outputCoord. */
        void fastRemove(int i) {
            ray.fastRemove(i);
            modulation.fastRemove(i);
            surfel.fastRemove(i);
            direct.fastRemove(i);
            shadowRay.fastRemove(i);
            lightShadowed.fastRemove(i);
            impulseRay.fastRemove(i);

            if (outputIndex.size() > 0) {
                outputIndex.fastRemove(i);
            } else {
                outputCoord.fastRemove(i);
            }
        }
    };

    mutable shared_ptr<TriTree>                 m_triTree;
    
    /** For the active trace */
    mutable Options                             m_options;
    shared_ptr<Scene>                           m_scene;
    mutable shared_ptr<CubeMap>                 m_skybox;

    /** \see Options:: useEnvironmentMapForLastScatteringEvent */
    mutable shared_ptr<CubeMap>                 m_environmentMap;

    static const Ray                            s_degenerateRay;

    PathTracer(const shared_ptr<TriTree>& t = nullptr);

    Radiance3 skyRadiance(const Vector3& direction) const;

    /**
     Sample a single light and choose a point on it, potentially in a low-discrepancy or importance sampling way.

      \a areaTimesPDFValue Return value that is the area of the light times the differential probability with which
       each point was selected.For uniform selection, this value is just 1.0f.

       \a X the point from which the light will be viewed. Set to Point3::NaN to ignore.
       \a n the surface normal at X. Set to Point3::NaN to ignore.

      \sa Light::lowDiscrepancyPosition
    */
    Point3 sampleOneLight(const shared_ptr<Light>& light, const Point3& X, const Vector3& n, int pixelIndex, int lightIndex, int sampleIndex, int numSamples, float& areaTimesPDFValue) const;

    /** Produces a buffer of eye rays, stored in raster order in the preallocated rayBuffer. 
        \param castThroughCenter When true (for the first ray at each pixel), cast the ray through
               the pixel center to make images look less noisy.
        \param raysPerPixel
        \param rayIndex between 0 and raysPerPixel - 1, used for lens low-discrepancy sampling */
    void generateEyeRays
       (int                                     width,
        int                                     height,
        const shared_ptr<Camera>&               camera,
        Array<Ray>&                             rayBuffer,
        bool                                    randomSubpixelPosition,
        Array<PixelCoord>&                      pixelCoordBuffer,
        const shared_ptr<Image>&                weightSumImage,
        int                                     rayIndex,
        int                                     raysPerPixel) const;

    /** In a properly modeled scene with area lights and no duplicating point lights, we should only count this 
        term on the first bounce. However, we're only going to sample point lights explicitly, so we need emissive
        on every bounce. Scenes like G3D cornell box where there are both point and emissives in the same location
        will get brighter than expected as a result.
        
        If outputBuffer is not null, writes to it using outputCoordBuffer indices, otherwise writes to
        radianceImage using pixelCoordBuffer indices.
        */
    void addEmissive
       (const Array<Ray>&                       rayFromEye,
        const Array<shared_ptr<Surfel>>&        surfelBuffer, 
        const Array<bool>&                      impulseRay,
        const Array<Color3>&                    modulationBuffer,
        Radiance3*                              outputBuffer,
        const Array<int>                        outputCoordBuffer,
        const shared_ptr<Image>&                radianceImage,
        const Array<PixelCoord>&                pixelCoordBuffer) const;

    /** Choose what light surface to sample, storing the corresponding shadow ray and biradiance value */
    void computeDirectIllumination
       (const Array<shared_ptr<Surfel>>&        surfelBuffer,
        const Array<shared_ptr<Light>>&         lightArray,
        const Array<Ray>&                       rayBuffer,
        int                                     currentPathDepth,
        int                                     currentRayIndex,
        const Options&                          options,
        const Array<PixelCoord>&                pixelCoordBuffer,
        const int                               radianceImageWidth,
        Array<Radiance3>&                       directBuffer,
        Array<Ray>&                             shadowRayBuffer) const;

    /** Apply the BSDF for each surfel to the biradiance in the corresponding light (unless shadowed),
        modulate as specified, and add to the image. Emissive light is only added for primary surfaces
        since it is already accounted for by explicit light sampling. 
        
        If outputBuffer is not null, writes to it using outputCoordBuffer indices, otherwise writes to
        radianceImage using pixelCoordBuffer indices.
        */
    virtual void shade
       (const Array<shared_ptr<Surfel>>&        surfelBuffer,
        const Array<Ray>&                       rayFromEye,
        const Array<Ray>&                       rayFromLight,
        const Array<bool>&                      lightShadowedBuffer,
        const Array<Radiance3>&                 directBuffer,
        const Array<Color3>&                    modulationBuffer,
        Radiance3*                              outputBuffer,
        const Array<int>                        outputCoordBuffer,
        const shared_ptr<Image>&                radianceImage,
        const Array<PixelCoord>&                pixelCoordBuffer) const; 

    /** sequenceIndex = (pixelIndex * maxPathDepth) + currentPathDepth)
    
    \param probability Relative probablity mass with which this particular sample was taken relative to other samples
           that were considered.     
    */
    const shared_ptr<Light>& importanceSampleLight
       (const Array<shared_ptr<Light>>&         lightArray,
        const Vector3&                          w_o,
        const shared_ptr<Surfel>&               surfel,
        int                                     sequenceIndex,
        int                                     rayIndex,
        int                                     raysPerPixel,
        Biradiance3&                            biradiance,
        Color3&                                 cosBSDFDivPDF,
        Point3&                                 lightPosition) const;

    /** Compute the next bounce direction by mutating rayBuffer, and then multiply the modulationBuffer by
        the inverse probability density that the direction was taken. Those probabilities are computed across
        three color channels, so modulationBuffer can become "colored" by this. */
    virtual void scatterRays
       (const Array<shared_ptr<Surfel>>&        surfelBuffer, 
        const Array<shared_ptr<Light>>&         indirectLightArray,
        int                                     currentPathDepth,
        int                                     rayIndex,
        int                                     raysPerPixel,
        Array<Ray>&                             rayBuffer,
        Array<Color3>&                          modulationBuffer,
        Array<bool>&                            impulseScatterBuffer) const;

    void prepare
       (const Options&                          options, 
        Array<shared_ptr<Light>>&               directLightArray, 
        Array<shared_ptr<Light>>&               indirectLightArray) const;

    /** Called from traceBuffer after the options are set and scene is processed.
        \param currentRayIndex If you are tracing multiple rays per pixel, this is
        the loop index of these rays.

        If \a output is not null, the output is written there. Otherwise the output is 
        bilinearly blended into the radianceImage and the pixelCoordBuffer is used.

        If \a weight is not null, it is an array of per-output weights.

        If \a distance is not null, the distance to each primary
        hit is written to it (not the "Z" value).
     */
    virtual void traceBufferInternal
       (BufferSet&                              buffers, 
        Radiance3*                              output,
        const shared_ptr<Image>&                radianceImage,
        float*                                  distance,
        const Array<shared_ptr<Light>>&         directLightArray,
        const Array<shared_ptr<Light>>&         indirectLightArray,
        int                                     currentRayIndex) const;

public:

    static shared_ptr<PathTracer> create(shared_ptr<TriTree> t = nullptr);

    /** Replaces the previous scene.*/
    void setScene(const shared_ptr<Scene>& scene);

    const shared_ptr<TriTree>& triTree() const {
        return m_triTree;
    }

    /** Call on the main thread if you wish to force GPU->CPU conversion and
        tree building to happen right now. */
    void prepare(const Options& options) const {
        Array<shared_ptr<Light>> directLightArray, indirectLightArray;
        prepare(options, directLightArray, indirectLightArray);
    }

    /** Assumes that the scene has been previously set. Only rebuilds the tree
        if the scene has changed. 

        \param statusCallback Function called periodically to update the GUI with the rendering progress. Arguments are percentage (between 0 and 1) and an arbitrary message string.
      */
    void traceImage(const shared_ptr<Image>& radianceImage, const shared_ptr<Camera>& camera, const Options& options, const std::function<void(const String&, float)>& statusCallback = nullptr) const;

    /** 
     \param output Must be allocated to at least the size of rayBuffer. This may be uncached, memory mapped memory.
     \param weight if not null, each output is scaled by the corresponding weight. 

     The rayBuffer will be modified. Make a copy if you wish to preserve the initial values.

     \param distance If not null, this is filled with the hit distance to the primary surface for each ray. Misses are set to infinity.
     \param lightEmissiveOnFirstHit Should the first hit be treated as a primary/impulse hit and include the emissive term from a light source surface?

     \param primaryWSNormalBuffer If not null, the normal at the primary hit point for each ray. NaN if no hit.
     \param primaryAlbedoBuffer If not null, a color at the primary hit point.
    */
    void traceBuffer
       (Array<Ray>&         rayBuffer,
        Radiance3*          output,
        const Options&      options,
        bool                lightEmissiveOnFirstHit,
        const float*        weight = nullptr,
        float*              distance = nullptr,
        Vector3*            primaryWSNormalBuffer = nullptr,
        Color3*             primaryAlbedoBuffer = nullptr
    ) const;
};

} // namespace
