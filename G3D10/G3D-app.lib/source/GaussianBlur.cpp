/**
  \file G3D-app.lib/source/GaussianBlur.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/filter.h"
#include "G3D-base/Rect2D.h"
#include "G3D-app/GaussianBlur.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/Draw.h"
#include "G3D-gfx/GLCaps.h"


namespace G3D {
struct PreambleKey {
    int N;
    bool unitArea;
    float stddevMultiplier;
    PreambleKey() {}
    PreambleKey(int N, bool unitArea, float stddevMultiplier) :
        N(N), unitArea(unitArea), stddevMultiplier(stddevMultiplier){}
    bool operator==(const PreambleKey& o) const {
        return N == o.N && unitArea == o.unitArea && stddevMultiplier == o.stddevMultiplier;
    }
    size_t hashCode() const {
        return (N*2) ^ int(unitArea) ^ int(stddevMultiplier * 1000.0f);
    }
};

}

template <> struct HashTrait<G3D::PreambleKey> {
  static size_t hashCode(const G3D::PreambleKey& key) { return key.hashCode(); }
};




namespace G3D {

void GaussianBlur::apply(RenderDevice* rd, const shared_ptr<Texture>& source, const Vector2& direction, int N) {
    apply(rd, source, direction, N, source->vector2Bounds(), true);
}


String GaussianBlur::getPreamble(int N, bool unitArea, float stddevMultiplier) {
    static Table<PreambleKey, String> preambles;
    PreambleKey preambleKey(N, unitArea, stddevMultiplier);

    bool created;
    String& preamble = preambles.getCreate(preambleKey, created);

    if (created) {
        Array<float> coeff;
        float stddev = N * 0.16f * stddevMultiplier;
        gaussian1D(coeff, N, stddev);

        if (! unitArea) {
            // Make the center tap have magnitude 1
            for (int i = 0; i < N; ++i) {
                coeff[i] /= coeff[(N - 1) / 2];
            }
        }
        preamble += format("#define KERNEL_RADIUS %d\n", N / 2 + 1);
        //this terrible hack was made neccesary by NVIDIA driver update 361.43. In this driver NVIDIA fails to support array initialization.
        if (GLCaps::enumVendor() == GLCaps::Vendor::NVIDIA) {
            preamble += "float gaussCoef[KERNEL_RADIUS];\n";
            preamble += "#define NVIDIA_361_WORKAROUND() ";
            for (int i = 0; i <= N / 2; ++i) {
                preamble += format("gaussCoef[%i] = %10.8f; ", i, coeff[i]);
            }
        } else {
            // Using const results in a massive slowdown for large kernels
            preamble += "float gaussCoef[KERNEL_RADIUS] = float[KERNEL_RADIUS]( ";
            for (int i = 0; i < N / 2; ++i) {
                preamble += format("%10.8f, ", coeff[i]);
            }
            // Last one has a closing paren and semicolon
            preamble += format("%10.8f);", coeff[N / 2]);
        }
    }
    return preamble;
}


void GaussianBlur::apply(RenderDevice* rd, const shared_ptr<Texture>& source, const Vector2& direction, int N, 
                         const Vector2& destSize, bool clear, bool unitArea, float stddevMultiplier, float computeFraction) {
    debugAssertM(isOdd(N), "Requires an odd filter width");
    
    Args args;

    // Make a string of the coefficients to insert into the shader
    args.appendToPreamble(getPreamble(N, unitArea, stddevMultiplier));

    const Rect2D& dest = Rect2D::xywh(Vector2(0, 0), destSize);
    rd->push2D(dest); {
        if (clear) { 
            rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);
            rd->clear(true, false, false); 
        }

        {
            const int s = iRound(log2(source->width() / destSize.x));
           
            args.setMacro("LOG_DOWNSAMPLE_X", s);
        }
        {
            const int s = iRound(log2(source->height() / destSize.y));
           
            args.setMacro("LOG_DOWNSAMPLE_Y", s);
        }
        args.setUniform("source", source, Sampler::video());
        args.setUniform("direction", Vector2int32(direction));
        args.setRect(dest);

        args.setMacro("COMPUTE_PERCENT", iRound(computeFraction * 100));

        LAUNCH_SHADER("GaussianBlur_apply.*", args);
    } rd->pop2D();
}
    
}
