/**
  \file G3D-app.lib/include/G3D-app/MotionBlurSettings.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef GLG3D_MotionBlurSettings_h
#define GLG3D_MotionBlurSettings_h

#include "G3D-base/platform.h"
#include "G3D-app/GBuffer.h"
namespace G3D {

class Any;

/** \see Camera, MotionBlur, DepthOfFieldSettings */
class MotionBlurSettings {
private:
    bool                        m_enabled;
    float                       m_exposureFraction;
    float                       m_maxBlurDiameterFraction;
    int                         m_numSamples;

public:

    MotionBlurSettings();

    MotionBlurSettings(const Any&);

    Any toAny() const;

    /** Fraction of the frame interval during which the shutter is
        open.  Default is 0.75 (75%).  Larger values create more motion blur, 
        smaller values create less blur.*/
    float exposureFraction() const {
        return m_exposureFraction;
    }

    void setExposureFraction(float f) {
        m_exposureFraction = f;
    }

    /** Objects are blurred over at most this distance as a fraction
        of the screen dimension along the dominant field of view
        axis. Default is 0.10 (10%).*/
    float maxBlurDiameterFraction() const {
        return m_maxBlurDiameterFraction;
    }

    void setMaxBlurDiameterFraction(float f) {
        m_maxBlurDiameterFraction = f;
    }


    bool enabled() const {
        return m_enabled;
    }

    void setEnabled(bool e) {
        m_enabled = e;
    }

    /** Number of samples taken per pixel when computing motion blur. Larger numbers are slower.  The 
        actual number of samples used by the current implementation must be odd, so this is always
        rounded to the odd number greater than or equal to it.

        Default = 11.
     */
    int numSamples() const {
        return m_numSamples;
    }

    void setNumSamples(int n) {
        m_numSamples = n;
    }

    void extendGBufferSpecification(GBuffer::Specification& spec) const;

};

}

#endif // G3D_MotionBlurSettings_h
