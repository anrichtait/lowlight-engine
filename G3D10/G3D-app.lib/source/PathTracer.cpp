/**
  \file G3D-app.lib/source/PathTracer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/PathTracer.h"
#include "G3D-base/Image.h"
#include "G3D-base/CubeMap.h"
#include "G3D-app/Light.h"
#include "G3D-app/Camera.h"
#include "G3D-app/Scene.h"
#include "G3D-app/UniversalSurfel.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"

namespace G3D {

/** Ray that is very unlikely to hit anything ... used for cases where some ray is needed for alignment in
    a buffer, but the result is either ignored or intended to be "miss" */
const Ray PathTracer::s_degenerateRay = Ray(Point3(5000, 5000, 5000), Vector3::unitX(), 0.0f, 0.01f);

PathTracer::PathTracer(const shared_ptr<TriTree>& t) {
    m_triTree = isNull(t) ? TriTree::create(true) : t;
}


shared_ptr<PathTracer> PathTracer::create(shared_ptr<TriTree> t) {
    return createShared<PathTracer>(t);
}


void PathTracer::setScene(const shared_ptr<Scene>& scene) {
    m_scene = scene;
}


Radiance3 PathTracer::skyRadiance(const Vector3& direction) const {
    if (m_skybox) {
        return m_skybox->bilinear(-direction);
    } else {
        // No skybox. Synthesize a reasonable sky gradient
        const float r = 1.0f - powf(max(-direction.y, 0.0f), 0.8f) * 0.7f;
        return Radiance3(r, lerp(r, 1.0f, 0.6f), 1.0f) * (0.4f + max(-direction.y, 0.0f) * 0.3f);
    }
}

// Below this amount of modulation, paths are terminated because their contribution
// is likely to be too low to matter.
static const float minModulation = 0.02f;

void PathTracer::traceImage
   (const shared_ptr<Image>&            radianceImage,
    const shared_ptr<Camera>&           camera,
    const Options&                      options,
    const std::function<void(const String&, float)>& statusCallback) const {
    
    // Visible area lights are handled by indirect rays during
    // recursive ray importance sampling. Point lights and invisible
    // lights can't be hit by indirect rays, so they receive special
    // handling.
    Array<shared_ptr<Light>> directLightArray, indirectLightArray;
    prepare(options, directLightArray, indirectLightArray);

    // Resize all buffers for one sample per pixel
    const int numPixels = radianceImage->width() * radianceImage->height();

    BufferSet buffers;
    // Total contribution, taking individual ray filter footprints into account
    const shared_ptr<Image>& weightSumImage = Image::create(radianceImage->width(), radianceImage->height(), ImageFormat::R32F());

    // All operations act on all pixels in parallel
    radianceImage->setAll(Radiance3::zero());
    for (int rayIndex = 0; rayIndex < options.raysPerPixel; ++rayIndex) {
        buffers.resize(numPixels);
        buffers.modulation.setAll(Color3::one());
        buffers.impulseRay.setAll(true);
        buffers.outputCoord.resize(numPixels);
        
        generateEyeRays(radianceImage->width(), radianceImage->height(), camera, buffers.ray, options.raysPerPixel > 1, buffers.outputCoord, weightSumImage, rayIndex, options.raysPerPixel);
        
        // Visualize eye rays
        // for (Point2int32 P(0, 0); P.y < radianceImage->height(); ++P.y) for (P.x = 0; P.x < radianceImage->width(); ++P.x) radianceImage->set(P, Radiance3(rayBuffer[P.x + P.y * radianceImage->width()].direction() * 0.5f + Vector3::one() * 0.5f)); return;

        traceBufferInternal(buffers, nullptr, radianceImage, nullptr, directLightArray, indirectLightArray, rayIndex);

        if (statusCallback) { statusCallback(format("%d/%d rays/pixel", rayIndex, options.raysPerPixel), float(rayIndex) / float(options.raysPerPixel)); }
    } // for rays per pixel

    // Normalize by the weight per pixel
    runConcurrently(Point2int32(0, 0), Point2int32(radianceImage->width(), radianceImage->height()), [&](Point2int32 pix) {
        debugAssertM(isFinite(weightSumImage->get<Color1>(pix).value), "Infinite/NaN weight");
        radianceImage->set(pix, radianceImage->get<Radiance3>(pix) / max(0.00001f, weightSumImage->get<Color1>(pix).value));
        debugAssertM(radianceImage->get<Color3>(pix).isFinite(), "Infinite/NaN radiance");
    }, ! m_options.multithreaded);
}


void PathTracer::generateEyeRays
(int                                 width, 
 int                                 height,
 const shared_ptr<Camera>&           camera,
 Array<Ray>&                         rayBuffer,
 bool                                randomSubpixelPosition,
 Array<PixelCoord>&                  pixelCoordBuffer,
 const shared_ptr<Image>&            weightSumImage,
 int                                 rayIndex,
 int                                 raysPerPixel) const {

    const Rect2D viewport = Rect2D::xywh(0.0f, 0.0f, float(width), float(height));

    const bool depthOfField = camera->depthOfFieldSettings().enabled() && (camera->depthOfFieldSettings().model() == DepthOfFieldModel::PHYSICAL);

    runConcurrently(Point2int32(0, 0), Point2int32(width, height), [&](Point2int32 point) {
        Vector2 offset(0.5f, 0.5f);
        if (randomSubpixelPosition) {
            Random& rng = Random::threadCommon();
            offset.x = rng.uniform(); offset.y = rng.uniform();
        }
        const int i = point.x + point.y * width;

        const Point2 P(float(point.x) + offset.x, float(point.y) + offset.y);

        if (depthOfField) {
            // Hammersley sequence remapped from a square to a disk
            const uint32_t hash = superFastHash(&point, sizeof(point));
            const Point2 pixelShift((hash >> 16) / float(0xFFFF), (hash & 0xFFFF) / float(0xFFFF));
            const Point2& h = (Point2::hammersleySequence2D(rayIndex, raysPerPixel) + pixelShift).mod1();
            const float angle = 2.0f * h.x * pif();
            const float radius = sqrt(h.y);
            const Point2 lens(cos(angle) * radius, sin(angle) * radius);
            rayBuffer[i] = camera->worldRay(P.x, P.y, lens.x, lens.y, viewport);
        }
        else {
            rayBuffer[i] = camera->worldRay(P.x, P.y, viewport);
        }

        // Camera coords put integers at top left, but image coords put them at pixel centers
        const PixelCoord& pixelCoord = Point2(point) + offset - Point2(0.5f, 0.5f);
        pixelCoordBuffer[i] = pixelCoord;
        }, ! m_options.multithreaded);

    const Color1 one(1.0f);
    for (const PixelCoord& pixelCoord : pixelCoordBuffer) {
        weightSumImage->bilinearIncrement(pixelCoord, one);
    }

}


void PathTracer::addEmissive
(const Array<Ray>&                   rayFromEye,
 const Array<shared_ptr<Surfel>>&    surfelBuffer, 
 const Array<bool>&                  impulseRay,
 const Array<Color3>&                modulationBuffer,
 Radiance3*                          outputBuffer,
 const Array<int>                    outputCoordBuffer,
 const shared_ptr<Image>&            radianceImage,
 const Array<PixelCoord>&            pixelCoordBuffer) const {
    
    runConcurrently(0, rayFromEye.length(), [&](int i) {
        const Surfel* surfel = surfelBuffer[i].get();
        const Vector3& w_o = -rayFromEye[i].direction();
        Radiance3 L_e = notNull(surfel) ? surfel->emittedRadiance(w_o) : skyRadiance(w_o);
        
        if (surfel && ! impulseRay[i] && surfel->isLight()) {
            // Remove the portion of non-impulse sampling of area lights that was already handled by direct illumination
            L_e *= 1.0f - m_options.areaLightDirectFraction;
        }

        debugAssertM(L_e.min() >= -1e-6f, "Negative emission");
        L_e = L_e.max(Color3::zero());

        if (L_e.nonZero()) {
            // Don't incur the memory transaction cost for the common case of no emissive
            debugAssertM(modulationBuffer[i].isFinite(), "Non-finite modulation");
            if (outputBuffer) {
                outputBuffer[outputCoordBuffer[i]] += L_e * modulationBuffer[i];
            } else {
                //radianceImage->increment(Point2int32(pixelCoordBuffer[i]), L_e * modulationBuffer[i]);
                radianceImage->bilinearIncrement(pixelCoordBuffer[i], L_e * modulationBuffer[i]);
            }
        }
    }, ! m_options.multithreaded);
}

static bool visibleAreaLight(const shared_ptr<Light>& light) {
    return (light->type() == Light::Type::AREA) && light->visible();
}


Point3 PathTracer::sampleOneLight(const shared_ptr<Light>& light, const Point3& X, const Vector3& n, int pixelIndex, int lightIndex, int sampleIndex, int numSamples, float& areaTimesPDFValue) const {
    areaTimesPDFValue = 1.0f;

   switch (m_options.samplingMethod) {
   case Options::LightSamplingMethod::UNIFORM_AREA:
       return light->uniformAreaPosition();
   case Options::LightSamplingMethod::STRATIFIED_AREA:
       return light->stratifiedAreaPosition(pixelIndex, sampleIndex, numSamples);
   case Options::LightSamplingMethod::LOW_DISCREPANCY_AREA:
       return light->lowDiscrepancyAreaPosition(pixelIndex, lightIndex, sampleIndex, numSamples);
   case Options::LightSamplingMethod::LOW_DISCREPANCY_SOLID_ANGLE:
   default:
       return light->lowDiscrepancySolidAnglePosition(pixelIndex, lightIndex, sampleIndex, numSamples, X, areaTimesPDFValue);
   }

}


const shared_ptr<Light>& PathTracer::importanceSampleLight
(const Array<shared_ptr<Light>>&             lightArray,
 const Vector3&                              w_o,
 const shared_ptr<Surfel>&                   surfel,
 int                                         sequenceIndex,
 int                                         rayIndex,
 int                                         raysPerPixel,
 Biradiance3&                                biradiance,
 Color3&                                     cosBSDFDivPDF,
 Point3&                                     lightPosition) const {

    const Point3&  X = surfel->position;
    const Vector3& n = surfel->shadingNormal;
    
    const shared_ptr<Light>& light0 = lightArray[0];
    if (lightArray.size() == 1) {
        // There is only one light, so of course we will sample it
        float areaTimesPDFValue;

        const Point3& Y = sampleOneLight(light0, X, n, sequenceIndex, 0, rayIndex, raysPerPixel, areaTimesPDFValue).xyz();

        lightPosition = Y;
        const Vector3& w_i = (lightPosition - X).direction();  
        biradiance = light0->biradiance(X, lightPosition);

        // To get correct brightness, we need to divide by the light area and the pdf with which we sampled.
        // However, if and only if biradiance is 0, the pdfValue is allowed to be 0, so don't divide by it 
        // unless we're sure that we have non-zero pdf;
        if (areaTimesPDFValue != 0.0f) {
            biradiance /= areaTimesPDFValue;
        }
        else {
            // Ensure that biradiance is 0 if the pdf was 0.
            debugAssertM(biradiance == Biradiance3(0.0f), "pdf value of zero with non-zero biradiance");
        }
        
        const Color3& f = surfel->finiteScatteringDensity(w_i, w_o);
        debugAssertM(biradiance.min() >= 0.0f, "Negative biradiance for light");
        debugAssertM(f.min() >= 0.0f, "Negative finiteScatteringDensity");

        if (visibleAreaLight(light0)) {
            biradiance *= m_options.areaLightDirectFraction;
        }

        cosBSDFDivPDF = f * fabsf(w_i.dot(n));
        return light0;

    } else {

        // Compute the biradiance for each light. Assume a small number of lights (e.g., 10)
        SmallArray<Biradiance3, 12> biradianceForLight;

        // cos * f() for the given light (cosine is different for the incoming vector for each light)
        SmallArray<Color3, 12> cosBRDFForLight;
        SmallArray<Point3, 12> lightPositionArray;
        biradianceForLight.resize(lightArray.size());
        cosBRDFForLight.resize(biradianceForLight.size());
        lightPositionArray.resize(biradianceForLight.size());
        Radiance totalRadiance = 0.0f;
        
        // Compute the CDF for importance resampling to choose a good light to cast
        // a shadow ray to:
        for (int j = 0; j < lightArray.size(); ++j) {
            const shared_ptr<Light>& light = lightArray[j];
            
            // Add : return areaTimesPDFValue
            float areaTimesPDFValue;
            const Point3& Y = sampleOneLight(light, X, n, sequenceIndex, j, rayIndex, raysPerPixel, areaTimesPDFValue).xyz();
            lightPositionArray[j] = Y;
            const Vector3& w_i = (Y - X).direction();

            // Add: divide by areaTimesPDFValue
            Biradiance3 B = light->biradiance(X, Y) / areaTimesPDFValue;

            // If the light casts nonzero energy towards this point across all color channels...
            if (B.sum() > 0.0f) {
                const Color3& f = surfel->finiteScatteringDensity(w_i, w_o);
                debugAssertM(f.isFinite(), "Infinite/NaN finiteScatteringDensity");
                debugAssertM(f.min() >= 0.0f, "Negative finiteScatteringDensity");

                if (visibleAreaLight(light)) {
                    B *= m_options.areaLightDirectFraction;
                }

                cosBRDFForLight[j] = f * fabsf(w_i.dot(n));
                biradianceForLight[j] = B;

                // update the monochrome total radiance that will normalize the CDF
                totalRadiance += (cosBRDFForLight[j] * biradianceForLight[j]).sum();
            } else {
                cosBRDFForLight[j] = biradianceForLight[j] = Biradiance3::zero();
            }
        } // lights

        // Sample the CDF to choose a light.
        // Choose light index j proportional to the light's relative biradiance. Note that 
        // we always select the last light if we slightly overshot due to roundoff. In scenes
        // with only one light, we always choose that light, of course.
        int j = 0;
        Random& rng = Random::threadCommon();
        Color3 cosBSDF;
        Radiance Lsum;
        for (float r = rng.uniform(0, totalRadiance); j < lightArray.size(); ++j) {
            biradiance = biradianceForLight[j];
            cosBSDF = cosBRDFForLight[j];
            Lsum = (biradiance * cosBSDF).sum();
            r -= Lsum;
            if (r <= 0.0f) { break; }
        }

        j = min(j, biradianceForLight.size() - 1);

        // Net radiance = L / (probability of choosing this light)
        cosBSDFDivPDF = cosBSDF * (totalRadiance / max(Lsum, 0.0001f));

        // Contribution this light will make...if it is not shadowed. The shadow ray
        // must still be cast before adding this in another parallel pass.
        biradiance = biradianceForLight[j];

        debugAssertM(cosBSDFDivPDF.isFinite(), "Infinite/NaN BSDF");
        debugAssertM(biradiance.isFinite(), "Infinite/NaN biradiance");
        const shared_ptr<Light>& light = lightArray[j];
        lightPosition = lightPositionArray[j];
        return light;
    }
}


void PathTracer::computeDirectIllumination
(const Array<shared_ptr<Surfel>>&    surfelBuffer, 
 const Array<shared_ptr<Light>>&     lightArray,
 const Array<Ray>&                   rayBuffer,
 int                                 currentPathDepth,
 int                                 currentRayIndex,
 const Options&                      options,
 const Array<PixelCoord>&            pixelCoordBuffer,
 const int                           radianceImageWidth,
 Array<Radiance3>&                   directBuffer,
 Array<Ray>&                         shadowRayBuffer) const {

    const float epsilon = 1e-3f;

    runConcurrently(0, surfelBuffer.size(), [&](int i) {
        const shared_ptr<Surfel>& surfel = surfelBuffer[i];

        debugAssert(notNull(surfel));

        Point3      lightPosition;
        Radiance3&  L_sd = directBuffer[i];
        Biradiance3 biradiance;
        Color3      cosBSDFDivPDF;

        Point2 pixelCoord = pixelCoordBuffer[i];
        int surfelIndex = int(pixelCoord.x + pixelCoord.y * radianceImageWidth);
        // Compute the surfel index before surfel compaction to ensure the low
        // discrepancy samples are not accidentally correlated.
        const shared_ptr<Light>& light = importanceSampleLight(lightArray, -rayBuffer[i].direction(), surfel, surfelIndex * options.maxScatteringEvents + currentPathDepth, currentRayIndex, options.raysPerPixel, biradiance, cosBSDFDivPDF, lightPosition);
        L_sd = biradiance * cosBSDFDivPDF;

        // Cast shadow rays from the light to the surface for more coherence in scenes
        // with few lights (i.e., where many pixels are casting from the same lights)
        //
        // ...but cast from the surface to the light to be consistent with single-sided
        // objects viewed from the camera.
        static const bool castTowardsLight = false;
        if (L_sd.nonZero()) {
            debugAssertM(L_sd.min() >= 0.0f, "Negative direct light");

            if (light->shadowsEnabled()) {
                // Generate shadow ray
                const Point3& overSurface = surfel->position + surfel->geometricNormal * epsilon;
                const Vector3& delta = overSurface - lightPosition;
                const float distance = delta.length();
                if (castTowardsLight) {
                    shadowRayBuffer[i] = Ray::fromOriginAndDirection(overSurface, delta * (-1.0f / distance), epsilon, distance - epsilon * 2.0f);
                } else {
                    shadowRayBuffer[i] = Ray::fromOriginAndDirection(lightPosition, delta * (1.0f / distance), epsilon, distance - epsilon * 2.0f);
                }
            } else {
                // Non-shadow casting light. Use a degenerate ray that is likely to miss everything to fake "no shadow"
                shadowRayBuffer[i] = s_degenerateRay;
            }
        } else {
            // Backface: create a degenerate ray
            shadowRayBuffer[i] = s_degenerateRay;
            directBuffer[i] = Radiance3::zero();
        }
    }, ! m_options.multithreaded);
}


void PathTracer::shade
(const Array<shared_ptr<Surfel>>&        surfelBuffer,
 const Array<Ray>&                       rayFromEye,
 const Array<Ray>&                       rayFromLight,
 const Array<bool>&                      lightShadowedBuffer,
 const Array<Radiance3>&                 directBuffer,
 const Array<Color3>&                    modulationBuffer,
 Radiance3*                              outputBuffer,
 const Array<int>                        outputCoordBuffer,
 const shared_ptr<Image>&                radianceImage,
 const Array<PixelCoord>&                pixelCoordBuffer) const {

    runConcurrently(0, surfelBuffer.size(), [&](int i) {
        // L_sd = outgoing scattered direct radiance
        Radiance3 L_sd = directBuffer[i];
        if (L_sd.nonZero() && ! lightShadowedBuffer[i]) {
            const float m = L_sd.max();
            if (m > m_options.maxIncidentRadiance) { L_sd *= m_options.maxIncidentRadiance / m; }

#           if 1 // Regular implementation
               const Radiance3& L = L_sd;
#           elif 0 // Visualize lambertian term
               const shared_ptr<Surfel>& surfel = surfelBuffer[i];
               debugAssertM(notNull(surfel), "Null surfels should have been compacted before shading"); 

               const Vector3& w_i = -rayFromLight[i].direction();
               const Vector3& w_o = -rayFromEye[i].direction();
               const Vector3& n   = surfel->shadingNormal;
               const shared_ptr<UniversalSurfel>& us = dynamic_pointer_cast<UniversalSurfel>(surfel);
               const Radiance3& L = us->lambertianReflectivity / pif();
#           elif 0 // Visualize hit points
               const Radiance3& L = Radiance3(surfel->position * 0.3f + Point3::one() * 0.5f));
#           else // Visualize normals
               const Radiance3& L = Radiance3(surfel->geometricNormal * 0.5f + Point3::one() * 0.5f));
#           endif

            debugAssertM(modulationBuffer[i].isFinite(), "Non-finite modulation");
            debugAssertM(modulationBuffer[i].min() >= 0.0f, "Negative modulation");
            debugAssertM(L.isFinite(), "Non-finite radiance");
            debugAssertM(L.min() >= 0.0f, "Negative radiance");
            if (outputBuffer) {
                outputBuffer[outputCoordBuffer[i]] += L * modulationBuffer[i];
            } else {
                //radianceImage->increment(Point2int32(pixelCoordBuffer[i]), L * modulationBuffer[i]);
                radianceImage->bilinearIncrement(pixelCoordBuffer[i], L * modulationBuffer[i]);
            }
        }
    });
}


void PathTracer::scatterRays
   (const Array<shared_ptr<Surfel>>&        surfelBuffer,
    const Array<shared_ptr<Light>>&         indirectLightArray,
    int                                     currentPathDepth,
    int                                     rayIndex,
    int                                     raysPerPixel,
    Array<Ray>&                             rayBuffer,
    Array<Color3>&                          modulationBuffer,
    Array<bool>&                            impulseRay) const {
    
    static const float epsilon = 1e-4f;

    runConcurrently(0, surfelBuffer.size(), [&](int i) {
        const shared_ptr<Surfel>& surfel = surfelBuffer[i];
        debugAssertM(notNull(surfel), "Null surfels should have been compacted before scattering");

        // Direction that the light went OUT, eventually towards the eye
        const Vector3& w_o = -rayBuffer[i].direction();

        // sample the pdf of (BRDF * cos)

        // Let p(w) be the PDF we're actually sampling to produce an output vector
        // Let g(w) = f(w, w') |w . n|   [Note: g() is *not* a PDF; just the arbitrary integrand]
        //
        // w = sample with respect to p(w)
        // weight = g(w) / p(w)
        Color3 weight;

        // Direction that light came in, being sampled
        Vector3 w_i;

#       if 1 // Surfel scattering
            surfel->scatter(PathDirection::EYE_TO_SOURCE, w_o, false, Random::threadCommon(), weight, w_i, impulseRay[i]);
#       else // Replace the BSDF for specific experiments.
            // scatterDBRDF
            // scatterDisney
            // scatterPeteCone
            // scatterBlinnPhong
            // scatterHackedBlinnPhong
            SimpleBSDF::scatter(dynamic_pointer_cast<UniversalSurfel>(surfel), w_o, Random::threadCommon(), w_i, weight);
#       endif

        if ((modulationBuffer[i].sum() < minModulation) || w_i.isNaN() || weight.isZero()) {
            // This ray didn't scatter; it will be culled after the next pass, so avoid
            // the cost of a real ray cast on it
            rayBuffer[i] = s_degenerateRay;
        } else {
            debugAssertM(weight.isFinite(), "Nonfinite weight");
            debugAssertM(weight.min() >= 0.0f, "Negative weight");
            if (! weight.isFinite()) {
                weight = Color3::zero();
            }

            // Clamp the maximum weight; inverses of really tiny numbers are 
            // likely to have high error and produce bright speckles. We instead
            // bias the results to use a maximum weight of no more than 1,
            // making the image too dark but less speckled.
            const float m = weight.max();
            if (m > m_options.maxImportanceSamplingWeight) {
                weight *= m_options.maxImportanceSamplingWeight / m;
            }


            modulationBuffer[i] *= weight;
            rayBuffer[i] = Ray::fromOriginAndDirection(surfel->position + surfel->geometricNormal * epsilon * sign(w_i.dot(surfel->geometricNormal)), w_i);
        }
    });
}
 

void PathTracer::traceBuffer
   (Array<Ray>&                         rayBuffer, 
    Radiance3*                          output,     
    const Options&                      options,
    bool                                lightEmissiveOnFirstHit,
    const float*                        weight,
    float*                              distance,
    Vector3*                            primaryWSNormalBuffer,
    Color3*                             primaryAlbedoBuffer) const {

    if (rayBuffer.size() == 0) { return; }

    alwaysAssertM(notNull(output), "Output must not be null");

    Array<shared_ptr<Light>> directLightArray, indirectLightArray;
    prepare(options, directLightArray, indirectLightArray);

    BufferSet buffers;
    buffers.resize(rayBuffer.size());
    buffers.ray = rayBuffer;
    buffers.outputIndex.resize(buffers.size());

    // Writing to output buffer
    runConcurrently(size_t(0), buffers.size(), [&](size_t i) {
        buffers.outputIndex[i] = int(i);
    });

    if (notNull(weight)) {
        runConcurrently(size_t(0), buffers.size(), [&](size_t i) {
            buffers.modulation[i] = Color3(weight[i]);
        });
    } else {
        buffers.modulation.setAll(Color3::one());
    }
    buffers.impulseRay.setAll(lightEmissiveOnFirstHit);
    
    // Zero the output
    System::memset(output, 0, sizeof(Radiance3) * buffers.size());

    // Trace
    traceBufferInternal(buffers, output, nullptr, distance, directLightArray, indirectLightArray, 1);
}


void PathTracer::prepare
   (const Options&                      options,
    Array<shared_ptr<Light>>&           directLightArray,
    Array<shared_ptr<Light>>&           indirectLightArray) const {
    
    // Save the options to avoid passing them through every method and obscuring the algorithm
    m_options = options;

    debugAssert(notNull(m_scene));
    if (max(m_scene->lastEditingTime(), m_scene->lastStructuralChangeTime(), m_scene->lastVisibleChangeTime()) > m_triTree->lastBuildTime()) {
        // Reset the tree
        debugPrintf("Rebuilding TriTree\n");
        m_triTree->setContents(m_scene);
        m_skybox = m_scene->skyboxAsCubeMap();
        m_environmentMap = nullptr;
    }

    // Another app may have rebuilt the TriTree, in which case we 
    // will miss the skybox assignment above, so set it here if necessary.
    if (isNull(m_skybox)) {
        m_skybox = m_scene->skyboxAsCubeMap();
    }

    directLightArray.fastClear(); indirectLightArray.fastClear();
    for (const shared_ptr<Light>& light : m_scene->lightingEnvironment().lightArray) {
        if (light->enabled()) {
            if ((m_options.areaLightDirectFraction == 0.0f) && light->visible() && (light->type() == Light::Type::AREA)) {
                indirectLightArray.append(light);
            } else {
                directLightArray.append(light);
            }
        }
    }

    if (m_options.useEnvironmentMapForLastScatteringEvent && isNull(m_environmentMap)) {
        m_environmentMap = m_scene->environmentMapAsCubeMap();
    }

}


void PathTracer::traceBufferInternal
   (BufferSet&                          buffers,
    Radiance3*                          output,
    const shared_ptr<Image>&            radianceImage, 
    float*                              distance,
    const Array<shared_ptr<Light>>&     directLightArray,
    const Array<shared_ptr<Light>>&     indirectLightArray,
    int                                 currentRayIndex) const {

    const int numRays = buffers.ray.size();
    if (numRays == 0) { return; }

    //alwaysAssertM((buffers.outputIndex.size() > 0) != notNull(radianceImage), "Exactly one of bufferSet.outputIndex and radianceImage may be specified");
    alwaysAssertM(isNull(radianceImage) || (numRays == buffers.outputCoord.size()), "Must be one ray per pixel coord");   
    debugAssertM(! distance || output, "Cannot specify distance buffer without an output buffer");
    
    const int numTraceIterations = m_options.maxScatteringEvents - (m_options.useEnvironmentMapForLastScatteringEvent ?  1 : 0);

    int radianceImageWidth = radianceImage->width();

    for (int scatteringEvents = 0; (scatteringEvents < numTraceIterations) && (buffers.surfel.size() > 0); ++scatteringEvents) {

        m_triTree->intersectRays(buffers.ray, buffers.surfel, (scatteringEvents == 0) ? TriTree::COHERENT_RAY_HINT : 0);

        if (notNull(distance) && (scatteringEvents == 0)) {
            // Write to the distance buffer.
            // On the first hit, outputCoordBuffer[i] == i, so no need for indirection on indices.
            runConcurrently(0, numRays, [&](int i) {
                const shared_ptr<Surfel>& surfel = buffers.surfel[i];
                distance[i] = surfel ? (surfel->position - buffers.ray[i].origin()).length() : finf();
            });
        }

        addEmissive(buffers.ray, buffers.surfel, buffers.impulseRay, buffers.modulation, output, buffers.outputIndex, radianceImage, buffers.outputCoord);

        // Compact buffers by removing paths that terminated (missed the entire scene)
        // This must be done serially.
        for (int i = 0; i < buffers.surfel.size(); ++i) {
            if (isNull(buffers.surfel[i]) || (buffers.modulation[i].sum() < minModulation)) {
                buffers.fastRemove(i);
                --i;
            }
        } // for i

        // Direct lighting
        if (directLightArray.size() > 0) {
            computeDirectIllumination(buffers.surfel, directLightArray, buffers.ray, scatteringEvents, currentRayIndex, m_options, buffers.outputCoord, radianceImageWidth, buffers.direct, buffers.shadowRay);
            m_triTree->intersectRays(buffers.shadowRay, buffers.lightShadowed, TriTree::COHERENT_RAY_HINT | TriTree::DO_NOT_CULL_BACKFACES | TriTree::OCCLUSION_TEST_ONLY);
            shade(buffers.surfel, buffers.ray, buffers.shadowRay, buffers.lightShadowed, buffers.direct, buffers.modulation, output, buffers.outputIndex, radianceImage, buffers.outputCoord);
        }

        // Indirect lighting rays (don't compute on the last scattering event)
        if (scatteringEvents < m_options.maxScatteringEvents - 1) {
            scatterRays(buffers.surfel, indirectLightArray, scatteringEvents, currentRayIndex, m_options.raysPerPixel, buffers.ray, buffers.modulation, buffers.impulseRay);
        }
    } // for scattering events

    if (m_options.useEnvironmentMapForLastScatteringEvent && notNull(m_environmentMap)) {
        // Perform an environment map lookup on the last ray. If m_options.useEnvironmentMapForLastScatteringEvent == 1,
        // this is actually the *first* ray, too, and the viewer will just see the environment map.

        runConcurrently(0, buffers.ray.size(), [&](int i) {
            const Radiance3& L = m_environmentMap->bilinear(buffers.ray[i].direction()) * buffers.modulation[i] * pif();
            if (output) {
                output[buffers.outputIndex[i]] += L;
            } else {
                //radianceImage->increment(Point2int32(buffers.outputCoord[i]), L);
                radianceImage->bilinearIncrement(buffers.outputCoord[i], L);
            }
        });
    }
}

}
