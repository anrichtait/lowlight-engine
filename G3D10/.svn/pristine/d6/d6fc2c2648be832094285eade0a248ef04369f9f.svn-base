/**
  \file G3D-app.lib/source/UniversalBSDF.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/UniversalBSDF.h"

namespace G3D {

float UniversalBSDF::ignoreFloat;

#define INV_PI  (0.318309886f)
#define INV_8PI (0.0397887358f)

bool UniversalBSDF::conservativelyHasTransparency() const {
    const Texture::Encoding& transmissiveEncoding = m_transmissive.texture()->encoding();

    return m_lambertian.texture()->conservativelyHasNonUnitAlpha() ||
        (transmissiveEncoding.readMultiplyFirst.rgb().max() > 0.0f);
}

shared_ptr<UniversalBSDF> UniversalBSDF::create
(const Component4& lambertian,
 const Component4& glossy,
 const Component3& transmissive,
 float             eta_t,
 const Color3&     extinction_t,
 float             eta_r,
 const Color3&     extinction_r) {

    const shared_ptr<UniversalBSDF>& bsdf = createShared<UniversalBSDF>();

    bsdf->m_lambertian      = lambertian;
    bsdf->m_glossy          = glossy;
    bsdf->m_transmissive    = transmissive;
    bsdf->m_eta_t           = eta_t;
    bsdf->m_extinction_t    = extinction_t;
    bsdf->m_eta_r           = eta_r;
    bsdf->m_extinction_r    = extinction_r;

    return bsdf;
}


void UniversalBSDF::setStorage(ImageStorage s) const {
    m_lambertian.setStorage(s);
    m_transmissive.setStorage(s);
    m_glossy.setStorage(s);
}


bool UniversalBSDF::hasMirror() const {
    const Color4& m = m_glossy.max();
    return (m.a == 1.0f) && ! m.rgb().isZero();
}


bool UniversalBSDF::hasGlossy() const {
    float avg = m_glossy.mean().a;
    return (avg > 0) && (avg < 1) && ! m_glossy.max().rgb().isZero();
}


bool UniversalBSDF::hasLambertian() const {
    return ! m_lambertian.max().rgb().isZero();
}

}
