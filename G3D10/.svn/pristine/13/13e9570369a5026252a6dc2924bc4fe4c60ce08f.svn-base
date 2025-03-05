/**
  \file G3D-app.lib/include/G3D-app/UniversalSurfel.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once

#include "G3D-base/platform.h"
#include "G3D-app/Tri.h"
#include "G3D-app/Surfel.h"

namespace G3D {

class CPUVertexArray;

/** 
 \brief A Surfel for a surface patch described by a UniversalMaterial.

 Computes the Surfel::ExpressiveParameters::boost solely from the lambertianReflectivity coefficient.

 \sa UniversalMaterial
*/
class UniversalSurfel : public Surfel {
protected:

    virtual void sampleFiniteDirectionPDF
    (PathDirection      pathDirection,
     const Vector3&     w_o,
     Random&            rng,
     const ExpressiveParameters& expressiveParameters,
     Vector3&           w_i,
     float&             pdfValue) const override;

public:

    /** \f$ \rho_\mathrm{L} \f$
    */
    Color3          lambertianReflectivity;

    /** F0, the Fresnel reflection coefficient at normal incidence */
    Color3          glossyReflectionCoefficient;
        
    Color3          transmissionCoefficient;

    /* 
        Post-normal-mapped normal in the coordinate frame determined
        by the pre-normal mapped interpolated normal and tangents
        (i.e. tangent space).

        Always (0,0,1) when there is no normal map. 

        This is useful for sampling Source-style Radiosity Normal
        Maps" http://www2.ati.com/developer/gdc/D3DTutorial10_Half-Life2_Shading.pdf
     */
    Vector3         tangentSpaceNormal;

    Radiance3       emission;

    /** ``alpha'' */
    float           coverage;

    // TODO: Make this a field of the base class instead of a method,
    // and then protect all fields with accessors to allow precomputation
    bool            isTransmissive;

    /** Zero = very rough, 1.0 = perfectly smooth (mirror). Equivalent the
      the \f$1 - \alpha\f$ parameter of the Generalized Trowbridge-Reitz
      microfacet BSDF (GTR/GGX).

      Transmission is always perfectly smooth 

      \sa blinnPhongExponent
    */
    float           smoothness;

    /** An approximate glossy exponent in the Blinn-Phong BSDF for this BSDF. */
    float blinnPhongExponent() const;

    /** Allocates with System::malloc to avoid the performance
        overhead of creating lots of small heap objects using
        the standard library %<code>malloc</code>. 

        This is actually not used by make_shared, which is the common case.
      */
    static void* operator new(size_t size) {
        return System::malloc(size);
    }

    static void operator delete(void* p) {
        System::free(p);
    }

    UniversalSurfel() : coverage(1.0f), isTransmissive(false), smoothness(0.0f) {}

    static shared_ptr<UniversalSurfel> create() {
        return std::make_shared<UniversalSurfel>();
    }

    /** Primarily useful for constructing skybox surfels */
    static shared_ptr<UniversalSurfel> createEmissive(const Radiance3 emission, const Point3& position, const Vector3& normal);

    void sample(const Tri& tri, float u, float v, int triIndex, const CPUVertexArray& vertexArray, bool backside, const class UniversalMaterial* universalMaterial, float du = 0, float dv = 0, bool twoSided = true);

    UniversalSurfel(const Tri& tri, float u, float v, int triIndex, const CPUVertexArray& vertexArray, bool backside, float du = 0, float dv = 0) {
        sample(tri, u, v, triIndex, vertexArray, backside, dynamic_pointer_cast<UniversalMaterial>(tri.material()).get(), du, dv);
    }

    virtual Radiance3 emittedRadiance(const Vector3& wo) const override;
    
    virtual bool transmissive() const override;

    virtual Color3 finiteScatteringDensity
    (const Vector3&    wi, 
     const Vector3&    wo,
     const ExpressiveParameters& expressiveParameters = ExpressiveParameters()) const override;
    
    virtual void getImpulses
    (PathDirection     direction,
     const Vector3&    wi,
     ImpulseArray&     impulseArray,
     const ExpressiveParameters& expressiveParameters = ExpressiveParameters()) const override;

    virtual Color3 reflectivity(Random& rng,
     const ExpressiveParameters& expressiveParameters = ExpressiveParameters()) const override;

    virtual bool nonZeroFiniteScattering() const override {
        return ((smoothness < 1.0f) && glossyReflectionCoefficient.nonZero()) || lambertianReflectivity.nonZero();
    }

    /** 
        Optimized to avoid numerical integration for lambertian-only and perfectly black surfaces.
     */
    virtual Color3 probabilityOfScattering
    (PathDirection pathDirection, const Vector3& w, Random& rng,
     const ExpressiveParameters& expressiveParameters = ExpressiveParameters()) const override;
    

    /**
      Useful for computing separate wide- and narrow-lobe scattering.

      Scatters one lambertian (cosine distributed about the shading normal) ray and one
      glossy (importance sampled by cos * glossy term) ray.

      Example of use:
      \code
      surfel->scatterSeparate(PathDirection::EYE_TO_LIGHT, w_in, rng, w_L, lambertianColor, w_G, glossyColor, glossyWeight);
      
      Irradiance3 E_inL = rayTrace(surfel.position, w_L);
      Radiance3   L_inG = rayTrace(surfel.position, w_G) * glossyWeight;

      Radiance3 L_indirectOut = E_L * lambertianColor + L_G * glossyColor;
      \endcode

      \param lambertianColorPerSteradian Has units of per-steradian. Lambertian lobe term from the BSDF * cosine angle of incidence.
      \param glossyWeight Unitless. This is the PDF adjustment for suboptimal importance sampling. Use it to modulate the glossy ray's contribution as incoming light.
      \param glossyColor Unitless. Glossy lobe term from the  BSDF * cosine angle of incidence.

       The current implementation does not handle transmission, so it is named "reflectSeparate"
      instead of "scatterSeparate"
     */
    virtual void reflectSeparate
    (PathDirection    pathDirection,
     const Vector3&   w_before,
     Random&          rng,
     Vector3&         w_lambertian,
     Color3&          lambertianColorPerSteradian,
     Vector3&         w_glossy,
     Color3&          glossyColor,
     Color3&          glossyWeight) const {}

};

} // namespace G3D
