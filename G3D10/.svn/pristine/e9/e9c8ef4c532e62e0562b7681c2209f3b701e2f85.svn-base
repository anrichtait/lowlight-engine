/**
  \file G3D-app.lib/source/UniversalSurfel.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/UniversalSurfel.h"
#include "G3D-app/UniversalBSDF.h"
#include "G3D-app/UniversalMaterial.h"
#include "G3D-gfx/CPUVertexArray.h"
#include "G3D-app/Surface.h"
#include "G3D-app/Tri.h"

namespace G3D {

shared_ptr<UniversalSurfel> UniversalSurfel::createEmissive(const Radiance3 emission, const Point3& position, const Vector3& normal) {
    const shared_ptr<UniversalSurfel>& s = create();
    s->emission = emission;
    s->coverage = 1.0f;
    s->isTransmissive = false;
    s->smoothness = 0.0f;
    s->position = s->prevPosition = position;
    s->geometricNormal = s->shadingNormal = normal;
    s->tangentSpaceNormal = Vector3::unitZ();
    s->material = nullptr;
    s->surface = nullptr;
    return s;
}

    
void UniversalSurfel::sample(const Tri& tri, float u, float v, int triIndex, const CPUVertexArray& vertexArray, bool backside, const UniversalMaterial* universalMaterial, float du, float dv, bool twoSided) {
    // TODO: MIP-map
    source.index = triIndex;
    source.u = u;
    source.v = v;

    const float w = 1.0f - u - v;
    const CPUVertexArray::Vertex* vertexArrayPtr = vertexArray.vertex.getCArray();
    debugAssert((tri.index[0] < (uint32)vertexArray.vertex.size()) &&
                (tri.index[1] < (uint32)vertexArray.vertex.size()) &&
                (tri.index[1] < (uint32)vertexArray.vertex.size()));

    const CPUVertexArray::Vertex& vert0 = vertexArrayPtr[tri.index[0]];
    const CPUVertexArray::Vertex& vert1 = vertexArrayPtr[tri.index[1]];
    const CPUVertexArray::Vertex& vert2 = vertexArrayPtr[tri.index[2]];   

    Vector3 interpolatedNormal =
       (w * vert0.normal + 
        u * vert1.normal +
        v * vert2.normal).direction();

    const Vector3& tangentX  =   
       (w * vert0.tangent.xyz() +
        u * vert1.tangent.xyz() +
        v * vert2.tangent.xyz()).direction();

    const Vector3& tangentY  =  
       ((w * vert0.tangent.w) * vert0.normal.cross(vert0.tangent.xyz()) +
        (u * vert1.tangent.w) * vert1.normal.cross(vert1.tangent.xyz()) +
        (v * vert2.tangent.w) * vert2.normal.cross(vert2.tangent.xyz())).direction();

    const Vector2& texCoord =
        w * vert0.texCoord0 +
        u * vert1.texCoord0 +
        v * vert2.texCoord0;

    geometricNormal = tri.normal(vertexArray);

    if (!interpolatedNormal.isFinite()) {
        // If the vertex normals exactly cancelled, fall back to the geometric normal
        interpolatedNormal = geometricNormal;
    }

    // The cast for material and extracting the surface
    // are very expensive.
    const shared_ptr<UniversalBSDF>& bsdf = universalMaterial->bsdf();    

    if (backside) {

        // If the surface is two-sided, swap the normal direction
        // before we compute values relative to it.
        if (twoSided) {
            interpolatedNormal = -interpolatedNormal;
            geometricNormal = -geometricNormal;
        }

        // Swap sides
        etaRatio = bsdf->etaTransmit() / bsdf->etaReflect();
        kappaPos = bsdf->extinctionTransmit();
        kappaNeg = bsdf->extinctionReflect();
    } else {
        etaRatio = bsdf->etaReflect() / bsdf->etaTransmit();
        kappaPos = bsdf->extinctionReflect();
        kappaNeg = bsdf->extinctionTransmit();
    }    

    const shared_ptr<BumpMap>& bumpMap = universalMaterial->bump();
    // TODO: support other types of bump map besides normal
    if (bumpMap && !tangentX.isNaN() && !tangentY.isNaN()) {
        const Matrix3& tangentSpace = Matrix3::fromColumns(tangentX, tangentY, interpolatedNormal);
        const shared_ptr<Image4>& normalMap = bumpMap->normalBumpMap()->image();
        const Vector2int32 mappedTexCoords(texCoord * Vector2(float(normalMap->width()), float(normalMap->height())));

        tangentSpaceNormal = Vector3(normalMap->get(mappedTexCoords.x, mappedTexCoords.y).rgb()) * 2.0f + Vector3(-1.0f, -1.0f, -1.0f);
        shadingNormal = (tangentSpace * tangentSpaceNormal).direction();

        // "Shading tangents", or at least one tangent, is traditionally used in anisotropic
        // BSDFs. To combine this with bump-mapping we use Graham-Schmidt orthonormalization
        // to perturb the tangents by the bump map in a sensible way. 
        // See: http://developer.amd.com/media/gpu_assets/shaderx_perpixelaniso.pdf 
        // Note that if the bumped normal
        // is parallel to a tangent vector, this will give nonsensical results.
        // TODO: handle the parallel case elegantly
        // TODO: Have G3D support anisotropic direction maps so we can apply this transformation
        // to that instead of these tangents taken from texCoord directions
        // TODO: implement
        shadingTangent1 = tangentX;
        shadingTangent2 = tangentY;

        /*
        Vector3 tangentSpaceTangent1 = Vector3(1,0,0);
        Vector3 tangentSpaceTangent2 = Vector3(0,1,0);
        
        Vector3& t1 = tangentSpaceTangent1;
        Vector3& t2 = tangentSpaceTangent2;
        const Vector3& n = tangentSpaceNormal;
        t1 = (t1 - (n.dot(t1) * n)).direction();
        t2 = (t2 - (n.dot(t2) * n)).direction();

        shadingTangent1 = tangentSpace.normalToWorldSpace(tangentSpaceTangent1).direction();
        shadingTangent2 = tangentSpace.normalToWorldSpace(tangentSpaceTangent2).direction();*/
    } else {
        tangentSpaceNormal = Vector3::unitZ();
        shadingNormal   = interpolatedNormal;
        shadingTangent1 = tangentX;
        shadingTangent2 = tangentY;
    } 

    position = 
       w * vert0.position + 
       u * vert1.position +
       v * vert2.position;
    
    if (vertexArray.hasPrevPosition()) {
        prevPosition = 
            w * vertexArray.prevPosition[tri.index[0]] + 
            u * vertexArray.prevPosition[tri.index[1]] +
            v * vertexArray.prevPosition[tri.index[2]];
    } else {
        prevPosition = position;
    }

    const Color4& lambertianSample = bsdf->lambertian().sample(texCoord);

    lambertianReflectivity = lambertianSample.rgb();
    coverage = lambertianSample.a;

    // Vertex color blending
    if (vertexArray.hasVertexColors) {
        const Color4* colorPtr = vertexArray.vertexColors.getCArray();

        const Color4& interpolatedColor =
            w * colorPtr[tri.index[0]] + 
            u * colorPtr[tri.index[1]] +
            v * colorPtr[tri.index[2]];
        lambertianReflectivity *= interpolatedColor.rgb();
        coverage *= interpolatedColor.a;
    }

    emission = universalMaterial->emissive().sample(texCoord);

    const Color4& packG = bsdf->glossy().sample(texCoord);
    glossyReflectionCoefficient  = packG.rgb();
    smoothness     = packG.a;
    
    transmissionCoefficient = bsdf->transmissive().sample(texCoord);

    isTransmissive = transmissionCoefficient.nonZero() || (coverage < 1.0f);

    material = universalMaterial;
    surface  = dynamic_cast<const Surface*>(tri.m_data.get());
}


float UniversalSurfel::blinnPhongExponent() const {
    return UniversalBSDF::smoothnessToBlinnPhongExponent(smoothness);
}


Radiance3 UniversalSurfel::emittedRadiance(const Vector3& wo) const {
    return emission;
}


bool UniversalSurfel::transmissive() const {
    return isTransmissive;
}


Color3 UniversalSurfel::finiteScatteringDensity
(const Vector3&    w_i, 
 const Vector3&    w_o,
 const ExpressiveParameters& expressiveParameters) const {

    // Surface normal
    const Vector3& n = shadingNormal;
    debugAssertM(n.isUnit() && n.isFinite(), "Non-unit normal in finiteScatteringDensity");

    if (w_i.dot(n) <= 0.0) { 
        // Transmission is specular-only, so never appears
        // in finiteScatteringDensity for this BSDF.
        return Color3::zero();
    }

    // Fresnel reflection at normal incidence
    const Color3& F_0 = glossyReflectionCoefficient;

    // Lambertian reflectivity (conditioned on not glossy reflected)
    const Color3& p_L = lambertianReflectivity * expressiveParameters.boost(lambertianReflectivity);

    // Half vector
    const Vector3& w_h = (w_i + w_o).directionOrZero();

    // Frensel reflection coefficient for this angle. Ignore fresnel
    // on surfaces that are magically set to zero reflectance.
    const Color3& F =
        F_0.nonZero() ?
        UniversalBSDF::schlickFresnel(F_0, max(0.0f, w_h.dot(w_i)), smoothness) : Color3::zero();

    // Lambertian term
    Color3 result = (Color3::one() - F) * (Color3::one() - F) * p_L * (1.0f / pif());
    debugAssertM(result.isFinite(), "Infinite or NaN matte lobe result in finiteScatteringDensity");

    // Ignore mirror impulse's contribution, which is handled in getImpulses().
    if (smoothness != 1.0f) {
        // Normalized Blinn-Phong lobe
        const float m = UniversalBSDF::smoothnessToBlinnPhongExponent(smoothness);
        // Avoid 0^0
        const float glossyLobe = pow(max(w_h.dot(n), 0.0f), max(m, 1e-6f)) *
            (8.0f + m) / (8.0f * pif() * square(max(w_i.dot(n), w_o.dot(n))));
        result += F * glossyLobe;
        debugAssertM(result.isFinite(), "Infinite or NaN glossy lobe result in finiteScatteringDensity");
    }

    return result;
}


void UniversalSurfel::getImpulses
(PathDirection     direction,
 const Vector3&    w_o,
 ImpulseArray&     impulseArray,
 const ExpressiveParameters& expressiveParameters) const {

    impulseArray.clear();

    // Fresnel reflection at normal incidence
    const Color3& F_0 = glossyReflectionCoefficient;

    // Lambertian reflectivity (conditioned on not glossy reflected)
    const Color3& p_L = lambertianReflectivity;

    // Transmission (conditioned on not glossy or lambertian reflected)
    const Color3& T = transmissionCoefficient;

    // Surface normal
    const Vector3& n = shadingNormal;

    // The half-vector IS the normal for mirror reflection purposes.
    // Frensel reflection coefficient for this angle. Ignore fresnel
    // on surfaces that are magically set to zero reflectance.
    const Color3& F = F_0.nonZero() ? UniversalBSDF::schlickFresnel(F_0, max(0.0f, n.dot(w_o)), smoothness) : Color3::zero();

    // Mirror reflection
    if ((smoothness == 1.0f) && F_0.nonZero()) {
        Surfel::Impulse& impulse = impulseArray.next();
        impulse.direction = w_o.reflectAbout(n);
        impulse.magnitude = F;
    }

    // Transmission
    const Color3& transmissionMagnitude = T * (Color3::one() - F) * (Color3::one() - (Color3::one() - F) * p_L);
    if (transmissionMagnitude.nonZero()) {
        const Vector3& transmissionDirection = (-w_o).refractionDirection(n, etaRatio);

        // Test for total internal reflection before applying this impulse
        if (transmissionDirection.nonZero()) {
            Surfel::Impulse& impulse = impulseArray.next();
            impulse.direction = transmissionDirection;
            impulse.magnitude = transmissionMagnitude;
        }
    }
}


Color3 UniversalSurfel::reflectivity(Random& rng,
 const ExpressiveParameters& expressiveParameters) const {

    // Base boost solely off Lambertian term
    const float boost = expressiveParameters.boost(lambertianReflectivity);

    // Only promises to be an approximation
    return lambertianReflectivity * boost + glossyReflectionCoefficient;
}


Color3 UniversalSurfel::probabilityOfScattering
   (PathDirection   pathDirection,
    const Vector3&  w,
    Random&         rng,
    const ExpressiveParameters& expressiveParameters) const {

    if (glossyReflectionCoefficient.isZero() && transmissionCoefficient.isZero()) {
        // No Fresnel term, so trivial to compute
        const float boost = expressiveParameters.boost(lambertianReflectivity);
        return lambertianReflectivity * boost;
    } else {
        // Compute numerically
        return Surfel::probabilityOfScattering(pathDirection, w, rng, expressiveParameters);
    }
}


void UniversalSurfel::sampleFiniteDirectionPDF
   (PathDirection      pathDirection,
    const Vector3&     _w_o,
    Random&            rng,
    const ExpressiveParameters& expressiveParameters,
    Vector3&           w_i,
    float&             pdfValue) const {

    // Surface normal
    const Vector3& n = shadingNormal;

    Vector3 w_o = _w_o;
    const float d = w_o.dot(n);
    if (d <= 0.0f) {
        // The outgoing vector is on the wrong side of the surface
        // (probably due to an interpolated shading normal that is
        // very far from the geometric normal). Clamp to the closest
        // legal vector at a glancing angle. Push d a little more negative
        // to avoid underflowing back into the same hemisphere
        w_o = (_w_o - n * (d - 1e-7f)).direction();
    }

    // Fresnel reflection at normal incidence
    const Color3& F_0 = glossyReflectionCoefficient;

    // Estimate the fresnel term coarsely, assuming mirror reflection. This is only used
    // for estimating the relativeGlossyProbability for the pdf; error will only lead to
    // noise, not bias in the result.
    const Color3& F = F_0.nonZero() ? UniversalBSDF::schlickFresnel(F_0, max(0.0f, n.dot(w_o)), smoothness) : Color3::zero();

    // Lambertian reflectivity (conditioned on not glossy reflected)
    const Color3& p_L = lambertianReflectivity;

    // Exponent for the cosine power lobe in the PDF that we're sampling.
    const float m = UniversalBSDF::smoothnessToBlinnPhongExponent(smoothness);

    debugAssert(m >= 0.0);

    // relativeGlossyProbability = prob(glossy reflection) / prob(any kind of reflection)
    const float relativeGlossyProbability = F_0.nonZero() ? F.average() / (F + (Color3::one() - F) * p_L).average() : 0.0f;

    Vector3::cosHemiPlusCosPowHemiHemiRandom(w_o.reflectAbout(n), n, m, relativeGlossyProbability, rng, w_i, pdfValue);
    debugAssert(isFinite(pdfValue));
}

} // namespace
