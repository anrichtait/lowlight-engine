/**
  \file G3D-app.lib/source/Surfel.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/Random.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/Any.h"
#include "G3D-app/Surfel.h"
#include "G3D-app/Material.h"
#include "G3D-app/Surface.h"

namespace G3D {

float Surfel::ExpressiveParameters::boost(const Color3& diffuseReflectivity) const {
    // Avoid computing the HSV transform in the common case
    if (unsaturatedMaterialBoost == saturatedMaterialBoost) {
        return unsaturatedMaterialBoost;
    }

    const float m = diffuseReflectivity.max();
    const float saturation = (m == 0.0f) ? 0.0f : ((m - diffuseReflectivity.min()) / m);

    return lerp(unsaturatedMaterialBoost, saturatedMaterialBoost, saturation);
}


Surfel::ExpressiveParameters::ExpressiveParameters(const Any& a) {
    *this = ExpressiveParameters();
    AnyTableReader r(a);

    r.get("unsaturatedMaterialBoost", unsaturatedMaterialBoost);
    r.get("saturatedMaterialBoost", saturatedMaterialBoost);
    r.verifyDone();
}


Any Surfel::ExpressiveParameters::toAny() const {
    Any a(Any::TABLE, "Surfel::ExpressiveParameters");

    a["unsaturatedMaterialBoost"] = unsaturatedMaterialBoost;
    a["saturatedMaterialBoost"] = saturatedMaterialBoost;

    return a;
}


Surfel::Surfel
(const Point3&     _pos,
 const Point3&     _prevPos,
 const Vector3&    geometricNormal,
 const Vector3&    shadingNormal,
 const Vector3&    shadingTangent1,
 const Vector3&    shadingTangent2,
 const float       etaPos,
 const Color3&     kappaPos,
 const float       etaNeg,
 const Color3&     kappaNeg,
 const uint8       flags,
 const Source&     source,
 const Material*   material,
 const Surface*    surface) :
    position(_pos),
    prevPosition(_prevPos),
    geometricNormal(geometricNormal),
    shadingNormal(shadingNormal),
    shadingTangent1(shadingTangent1),
    shadingTangent2(shadingTangent2),
    etaRatio(etaPos / etaNeg), 
    kappaPos(kappaPos),
    kappaNeg(kappaNeg),
    material(material), 
    surface(surface),
    flags(flags),
    source(source) {
}


void Surfel::transformToWorldSpace(const CFrame& xform) {
    position = xform.pointToWorldSpace(position);
    geometricNormal = xform.vectorToWorldSpace(geometricNormal);
    shadingNormal   = xform.vectorToWorldSpace(shadingNormal);
    shadingTangent1 = xform.vectorToWorldSpace(shadingTangent1);
    shadingTangent2 = xform.vectorToWorldSpace(shadingTangent2);
}


Color3 Surfel::finiteScatteringDensity
   (PathDirection pathDirection,
    const Vector3&    wFrom, 
    const Vector3&    wTo,
    const ExpressiveParameters& expressiveParameters) const {

    if (pathDirection == PathDirection::SOURCE_TO_EYE) {
        return finiteScatteringDensity(wFrom, wTo, expressiveParameters);
    } else {
        return finiteScatteringDensity(wTo, wFrom, expressiveParameters);
    }
}


void Surfel::sampleFiniteDirectionPDF
   (PathDirection      pathDirection,
    const Vector3&     w_o,
    Random&            rng,
    const ExpressiveParameters& expressiveParameters,
    Vector3&           w_i,
    float&             pdfValue) const {

    if (transmissive()) {
        Vector3::cosSphereRandom(shadingNormal, rng, w_i, pdfValue);
    } else {
        Vector3::cosHemiRandom(shadingNormal, rng, w_i, pdfValue);
    }
}


float Surfel::ignore = 0.0;
bool Surfel::ignoreBool;
    
bool Surfel::scatter
   (PathDirection    pathDirection,
    const Vector3&   w_o,
    bool             russianRoulette,
    Random&          rng,
    Color3&          weight,
    Vector3&         w_i,
    bool&            wasImpulse,
    float&           probabilityHint,
    const ExpressiveParameters& expressiveParameters) const {

    // Russian Roulette rescaling
    float rrWeight = 1.0f;
    
    if (russianRoulette) {
        // Net probability of scattering
        const Color3& prob3 = probabilityOfScattering(pathDirection, w_i, rng, expressiveParameters);

        // Apply Russian Roulette with the sqrt probability, following the Arnold renderer's
        // observation that this works better in practice. This increases the chance of following
        // a path a little further.
        const float prob = sqrt(prob3.average());
        const float epsilon = 0.000001f;
        
        // If the total probability is zero, we always want to absorb regardless
        // of the russianRoulette flag value
        if (rng.uniform(epsilon, 1) > prob) {
            w_i = Vector3::nan();
            weight = Color3::zero();
            return false;
        } else {
            rrWeight = 1.0f / prob;
        }
    }

    Surfel::ImpulseArray impulseArray;
    getImpulses(pathDirection, w_o, impulseArray, expressiveParameters);

    float impulseMagnitudeSum = 0.0f;
    float r = rng.uniform();
    for (int i = 0; i < impulseArray.size(); ++i) {
        const Surfel::Impulse& impulse = impulseArray[i];
        const float probabilityOfThisImpulse = impulse.magnitude.average();
        r -= probabilityOfThisImpulse;
        impulseMagnitudeSum += probabilityOfThisImpulse;
        if (r <= 0.0f) {
            w_i    = impulse.direction;
            weight = rrWeight * impulse.magnitude / probabilityOfThisImpulse;
            probabilityHint = probabilityOfThisImpulse;
            wasImpulse = true;
            return weight.nonZero();
        }
    }

    wasImpulse = false;

    // Choose a direction according to the finite portion of the BSDF, conditioned on not having already chosen an impulse.
    float pdfValue;
    sampleFiniteDirectionPDF(pathDirection, w_o, rng, expressiveParameters, w_i, pdfValue);
    
    // Debugging code: choose uniformly on the hemisphere
    //   w_i = Vector3::hemiRandom(shadingNormal, rng); pdfValue = 1.0f / (2.0f * pif());

    // We took this branch with probability (1 - impulseMagnitudeSum), so account for that 
    // in the net probability. Now pdfValue is no longer conditioned on not taking an impulse.
    pdfValue *= 1.0f - impulseMagnitudeSum;
    debugAssert(isFinite(pdfValue) && pdfValue >= 0.0f);
    if (pdfValue > 0.0f) {
        // Common case. Evaluate the actual BSDF in the direction chosen so that we can account for any discrepancy between
        // the pdf sampling and the actual function. Also scale by the cosine of the angle of incidence, which
        // sampleFiniteDirectionPDF does not consider.
        const float cos_i = std::max(w_i.dot(shadingNormal), 0.0f);
        const Color3& bsdf = finiteScatteringDensity(pathDirection, w_o, w_i, expressiveParameters);
        weight = rrWeight * bsdf * (cos_i / std::min(1e8f, pdfValue));
        // Debugging code: white lambertian BSDF
        //  weight = (Color3::one() / pif()) * std::max(w_i.dot(shadingNormal), 0.0f) / std::min(1e8f, pdfValue);
    } else {
        // This case should only very rarely occur: we sampled according to the BSDF...and it was then zero at that direction
        weight = Color3::zero();
    }
    debugAssert(weight.isFinite());
    probabilityHint = float(weight.average() * 1e-3f);
    return weight.nonZero();
}


#if 0 // old code with russianRoulette support

    // Net probability of scattering
    const Color3& prob3 = probabilityOfScattering(pathDirection, wi, rng, expressiveParameters);
    const float prob = prob3.average();

    probabilityHint = 1.0f;

    const float epsilon = 0.000001f;
    {
        // If the total probability is zero, we always want to absorb regardless
        // of the russianRoulette flag value
        const float thresholdProbability = 
            russianRoulette ? rng.uniform(0, 1) : 0.0f;
        
        if (prob <= thresholdProbability) {
            // There is no possible outgoing scattering direction for
            // this incoming direction.
            weight = Color3(0.0f);
            wo = Vector3::nan();
            return;
        }
    }

    ImpulseArray impulseArray;
    getImpulses(pathDirection, wi, impulseArray, expressiveParameters);

    // Importance sample against impulses vs. finite scattering
    float r = rng.uniform(0, prob);
    float totalIprob = 0.0f;

    for (int i = 0; i < impulseArray.size(); ++i) {
        // Impulses don't have projected area terms
        const Impulse& I     = impulseArray[i];
        const float    Iprob = I.magnitude.average();

        alwaysAssertM(Iprob > 0, "Zero-probability impulse");

        r -= Iprob;
        totalIprob += Iprob;
        if (r <= epsilon) {
            // Scatter along this impulse.  Divide the weight by
            // the average because we just importance sampled
            // among the impulses...but did so along a single
            // color channel.
              
            const float mag = I.magnitude.average();
            weight = I.magnitude / mag;
            if (russianRoulette) {
                weight /= prob;
            }

            wo = I.direction;
            probabilityHint = mag;
            return;
        }
    }

    alwaysAssertM(prob >= totalIprob,
                    "BSDF impulses accounted for more than "
                    "100% of scattering probability");

    // If we make it to here, the finite portion of the BSDF must
    // be non-zero because the BSDF itself is non-zero and
    // none of the impulses triggered sampling (note that r was
    // chosen on [0, prob], not [0, 1] above.
    
    sampleFinite(pathDirection, wi, rng, expressiveParameters, weight, wo);

    if (russianRoulette) {
        // For Russian roulette, if we successfully made it to this point without
        // rejecting the photon, normalize by the a priori
        // scattering probability, unless it is greater than 1.0 due to
        // non-physical boost (if it is greater than 1.0, there was zero
        // chance of absorption). 
        if (prob < 1.0f) {
            weight /= prob;
        }
    }

    probabilityHint = float(weight.average() * 1e-3);
}
#endif


Color3 Surfel::probabilityOfScattering
(PathDirection  pathDirection, 
 const Vector3& wi, 
 Random&        rng,
 const ExpressiveParameters& expressiveParameters) const {

    Color3 prob;

    // Sum the impulses (no cosine; principle of virtual images)
    ImpulseArray impulseArray;
    getImpulses(pathDirection, wi, impulseArray);
    for (int i = 0; i < impulseArray.size(); ++i) {
        prob += impulseArray[i].magnitude;
    }

    // This is uniform random sampling; would some kind of
    // striated or jittered sampling might produce a
    // lower-variance result.

    // Sample the finite portion.  Note the implicit cosine
    // weighting in the importance sampling of the sphere.
    const int N = 32;

    if (transmissive()) {
        // Check the entire sphere

        // Measure of each sample on a cosine-weighted sphere 
        // (the area of a cosine-weighted sphere is 2.0)
        const float dw = 2.0f * pif() / float(N);
        for (int i = 0; i < N; ++i) {
            const Vector3& wo = Vector3::cosSphereRandom(shadingNormal, rng);
            prob += finiteScatteringDensity(pathDirection, wi, wo, expressiveParameters) * dw;
        }
    } else {
        // Check only the positive hemisphere since the other hemisphere must be all zeros
        // Measure of each sample on a cosine-weighted hemisphere
        const float dw = 1.0f * pif() / float(N);
        for (int i = 0; i < N; ++i) {
            const Vector3& wo = Vector3::cosHemiRandom(shadingNormal, rng);
            prob += finiteScatteringDensity(pathDirection, wi, wo, expressiveParameters) * dw;
        }
    }

    return prob;
}


Color3 Surfel::reflectivity(Random& rng, const ExpressiveParameters& expressiveParameters) const {
    Color3 c;

    const int N = 10;
    for (int i = 0; i < N; ++i) {
        c += probabilityOfScattering(PathDirection::EYE_TO_SOURCE, Vector3::cosHemiRandom(shadingNormal, rng), rng, expressiveParameters) / float(N);
    }

    return c;
}


} // namespace G3D
