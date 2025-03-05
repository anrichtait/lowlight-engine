/**
  \file G3D-app.lib/source/AmbientOcclusionSettings.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/Any.h"
#include "G3D-base/units.h"
#include "G3D-app/AmbientOcclusionSettings.h"
#include "G3D-gfx/GLCaps.h"

namespace G3D {

AmbientOcclusionSettings::AmbientOcclusionSettings() : 
    radius(0.75f * units::meters()),
    bias(0.02f),
    intensity(1.0f),
    numSamples(20),
    edgeSharpness(1.0f),
    blurStepSize(1),
    blurRadius(2),
    useNormalsInBlur(true),
    monotonicallyDecreasingBilateralWeights(false),
    useDepthPeelBuffer(false),
    useNormalBuffer(true),
    depthPeelSeparationHint(0.01f),
    zStorage(ZStorage::HALF),
    highQualityBlur(true),
    packBlurKeys(false),
    temporallyVarySamples(true),
    enabled(true) {

    temporalFilterSettings.hysteresis = 0.9f;
}

    
AmbientOcclusionSettings::AmbientOcclusionSettings(const Any& a) {
    *this = AmbientOcclusionSettings();

    a.verifyName("AmbientOcclusionSettings");

    AnyTableReader r(a);
    r.getIfPresent("enabled", enabled);
    r.getIfPresent("intensity", intensity);
    r.getIfPresent("bias", bias);
    r.getIfPresent("radius", radius);
    r.getIfPresent("numSamples", numSamples);
    r.getIfPresent("samples", numSamples);
    r.getIfPresent("edgeSharpness", edgeSharpness);
    r.getIfPresent("blurStepSize", blurStepSize);
    r.getIfPresent("blurStride", blurStepSize);
    r.getIfPresent("blurRadius", blurRadius);
    r.getIfPresent("useNormalsInBlur", useNormalsInBlur);
    r.getIfPresent("monotonicallyDecreasingBilateralWeights", monotonicallyDecreasingBilateralWeights);
    r.getIfPresent("useDepthPeelBuffer", useDepthPeelBuffer);
    r.getIfPresent("useNormalBuffer", useNormalBuffer);
    r.getIfPresent("depthPeelSeparationHint", depthPeelSeparationHint);
    r.getIfPresent("highQualityBlur", highQualityBlur);
    r.getIfPresent("zStorage", zStorage);
    r.getIfPresent("packBlurKeys", packBlurKeys);
    r.getIfPresent("temporalFilterSettings", temporalFilterSettings);
    r.getIfPresent("temporallyVarySamples", temporallyVarySamples);

    r.verifyDone();
}


Any AmbientOcclusionSettings::toAny() const {
    Any a(Any::TABLE, "AmbientOcclusionSettings");

    a["enabled"]                    = enabled;
    a["intensity"]                  = intensity;
    a["radius"]                     = radius;
    a["bias"]                       = bias;
    a["numSamples"]                 = numSamples;
    a["edgeSharpness"]              = edgeSharpness;
    a["blurStepSize"]               = blurStepSize;
    a["blurRadius"]                 = blurRadius;
    a["useNormalsInBlur"]           = useNormalsInBlur;
    a["monotonicallyDecreasingBilateralWeights"]    = monotonicallyDecreasingBilateralWeights;
    a["useDepthPeelBuffer"]         = useDepthPeelBuffer;
    a["useNormalBuffer"]            = useNormalBuffer;
    a["depthPeelSeparationHint"]    = depthPeelSeparationHint;
    a["highQualityBlur"]            = highQualityBlur;
    a["zStorage"]                   = zStorage; 
    a["packBlurKeys"]               = packBlurKeys;
    a["temporalFilterSettings"]     = temporalFilterSettings;
    a["temporallyVarySamples"]      = temporallyVarySamples;
    return a;
}

void AmbientOcclusionSettings::extendGBufferSpecification(GBuffer::Specification& spec) const {
    if (enabled) {
        if (useNormalBuffer) {
            if (spec.encoding[GBuffer::Field::CS_NORMAL].format == nullptr &&
                spec.encoding[GBuffer::Field::CS_FACE_NORMAL].format == nullptr) {

                const ImageFormat* normalFormat = ImageFormat::RGB10A2();
                spec.encoding[GBuffer::Field::CS_NORMAL] = Texture::Encoding(normalFormat, FrameName::CAMERA, 2.0f, -1.0f);
            }
        }
        if (temporalFilterSettings.hysteresis > 0.0f) {
            if (spec.encoding[GBuffer::Field::SS_POSITION_CHANGE] == nullptr) {
                spec.encoding[GBuffer::Field::SS_POSITION_CHANGE] = Texture::Encoding::lowPrecisionScreenSpaceMotionVector();
            }
        }
    }
}

int AmbientOcclusionSettings::numSpiralTurns() const {
#define NUM_PRECOMPUTED 100

    static int minDiscrepancyArray[NUM_PRECOMPUTED] = {
    //  0   1   2   3   4   5   6   7   8   9
        1,  1,  1,  2,  3,  2,  5,  2,  3,  2,  // 0
        3,  3,  5,  5,  3,  4,  7,  5,  5,  7,  // 1
        9,  8,  5,  5,  7,  7,  7,  8,  5,  8,  // 2
       11, 12,  7, 10, 13,  8, 11,  8,  7, 14,  // 3
       11, 11, 13, 12, 13, 19, 17, 13, 11, 18,  // 4
       19, 11, 11, 14, 17, 21, 15, 16, 17, 18,  // 5
       13, 17, 11, 17, 19, 18, 25, 18, 19, 19,  // 6
       29, 21, 19, 27, 31, 29, 21, 18, 17, 29,  // 7
       31, 31, 23, 18, 25, 26, 25, 23, 19, 34,  // 8
       19, 27, 21, 25, 39, 29, 17, 21, 27, 29}; // 9

    if (numSamples < NUM_PRECOMPUTED) {
        return minDiscrepancyArray[numSamples];
    } else {
        return 5779; // Some large prime. Hope it does alright. It'll at least never degenerate into a perfect line until we have 5779 samples...
    }

#undef NUM_PRECOMPUTED


}


}
