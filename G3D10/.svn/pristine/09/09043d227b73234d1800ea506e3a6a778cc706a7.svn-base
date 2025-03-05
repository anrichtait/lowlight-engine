/**
  \file G3D-app.lib/source/MotionBlurSettings.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/MotionBlurSettings.h"
#include "G3D-base/Any.h"
#include "G3D-gfx/GLCaps.h"

namespace G3D {

MotionBlurSettings::MotionBlurSettings() : 
    m_enabled(false), 
    m_exposureFraction(0.75f), 
    m_maxBlurDiameterFraction(0.10f),
    m_numSamples(27) {}


MotionBlurSettings::MotionBlurSettings(const Any& any) {
    *this = MotionBlurSettings();

    AnyTableReader reader("MotionBlurSettings", any);

    reader.getIfPresent("enabled",                  m_enabled);
    reader.getIfPresent("exposureFraction",         m_exposureFraction);
    reader.getIfPresent("maxBlurDiameterFraction",  m_maxBlurDiameterFraction);
    reader.getIfPresent("numSamples",               m_numSamples);
    reader.verifyDone();

    m_exposureFraction      = clamp(m_exposureFraction, 0.0f, 2.0f);
}


Any MotionBlurSettings::toAny() const {

    Any any(Any::TABLE, "MotionBlurSettings");

    any["enabled"]                  = m_enabled;
    any["exposureFraction"]         = m_exposureFraction;
    any["maxBlurDiameterFraction"]  = m_maxBlurDiameterFraction;
    any["numSamples"]               = m_numSamples;

    return any;
}

void MotionBlurSettings::extendGBufferSpecification(GBuffer::Specification& spec) const {
    if (m_enabled) {
        if (isNull(spec.encoding[GBuffer::Field::SS_POSITION_CHANGE].format)) {
            spec.encoding[GBuffer::Field::SS_POSITION_CHANGE] = Texture::Encoding::lowPrecisionScreenSpaceMotionVector();
        }
    }
}

}
