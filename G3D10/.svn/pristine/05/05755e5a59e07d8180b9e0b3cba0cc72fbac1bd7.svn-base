/**
  \file G3D-gfx.lib/source/Texture_Preprocess.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-gfx/Texture.h"
#include "G3D-base/Any.h"

namespace G3D {

Any Texture::Preprocess::toAny() const {
    Any a(Any::TABLE, "Texture::Preprocess");
    a["modulate"] = modulate;
    a["gammaAdjust"] = gammaAdjust;
    a["computeMinMaxMean"] = computeMinMaxMean;
    a["bumpMapPreprocess"] = bumpMapPreprocess;
    a["convertToPremultipliedAlpha"] = convertToPremultipliedAlpha;
    return a;
}


bool Texture::Preprocess::operator==(const Preprocess& other) const {
    return 
        (modulate == other.modulate) &&
        (gammaAdjust == other.gammaAdjust) &&
        (computeMinMaxMean == other.computeMinMaxMean) &&
        (bumpMapPreprocess == other.bumpMapPreprocess) &&
        (convertToPremultipliedAlpha == other.convertToPremultipliedAlpha);
}


Texture::Preprocess::Preprocess(const Any& any) {
    *this = Preprocess::defaults();
    any.verifyNameBeginsWith("Texture::Preprocess");
    if (any.type() == Any::TABLE) {
        for (Any::AnyTable::Iterator it = any.table().begin(); it.isValid(); ++it) {
            const String& key = it->key;
            if (key == "modulate") {
                modulate = Color4(it->value);
            } else if (key == "gammaAdjust") {
                gammaAdjust = it->value;
            } else if (key == "computeMinMaxMean") {
                computeMinMaxMean = it->value;
            } else if (key == "convertToPremultipliedAlpha") {
                convertToPremultipliedAlpha = it->value;
            } else if (key == "bumpMapPreprocess") {
                bumpMapPreprocess = it->value;
            } else {
                any.verify(false, "Illegal key in Texture::PreProcess: " + it->key);
            }
        }
    } else {
        const String& n = any.name();
        if (n == "Texture::Preprocess::defaults") {
            any.verifySize(0);
        } else if (n == "Texture::Preprocess::gamma") {
            any.verifySize(1);
            *this = Texture::Preprocess::gamma(any[0]);
        } else if (n == "Texture::preprocess::none") {
            any.verifySize(0);
            *this = Texture::Preprocess::none();
        } else if (n == "Texture::Preprocess::quake") {
            any.verifySize(0);
            *this = Texture::Preprocess::quake();
        } else if (n == "Texture::Preprocess::normalMap") {
            any.verifySize(0);
            *this = Texture::Preprocess::normalMap();
        } else {
            any.verify(false, "Unrecognized name for Texture::Preprocess constructor or factory method.");
        }
    }
}


const Texture::Preprocess& Texture::Preprocess::defaults() {
    static const Texture::Preprocess p;
    return p;
}


Texture::Preprocess Texture::Preprocess::gamma(float g) {
    Texture::Preprocess p;
    p.gammaAdjust = g;
    return p;
}


const Texture::Preprocess& Texture::Preprocess::none() {
    static Texture::Preprocess p;
    p.computeMinMaxMean = false;
    return p;
}


const Texture::Preprocess& Texture::Preprocess::quake() {
    static Texture::Preprocess p;
    p.modulate = Color4(2, 2, 2, 1);
    p.gammaAdjust = 1.6f;
    return p;
}


const Texture::Preprocess& Texture::Preprocess::normalMap() {
    static bool initialized = false;
    static Texture::Preprocess p;
    if (! initialized) {
        p.bumpMapPreprocess.mode = BumpMapPreprocess::Mode::AUTODETECT_TO_AUTODETECT;
        initialized = true;
    }

    return p;
}


void Texture::Preprocess::modulateOffsetAndGammaAdjustImage(ImageFormat::Code fmt, void* _src, void* _dst, int n) const {
    debugAssertM(
        (fmt == ImageFormat::CODE_RGB8) ||
        (fmt == ImageFormat::CODE_RGBA8) ||
        (fmt == ImageFormat::CODE_R8) ||
        (fmt == ImageFormat::CODE_L8),
        "Texture modulate and offset only implemented for 1, 3, and 4 channel images with 8 bits per channel.");

    uint8* byte = static_cast<uint8*>(_src);
    uint8* dst  = static_cast<uint8*>(_dst);

    // Make a lookup table
    uint8 adjust[4][256];
    for (int c = 0; c < 3; ++c) {
        for (int i = 0; i < 256; ++i) {
            const float s = float(pow(float(i) * modulate[c] / 255.0f + offset[c], gammaAdjust) * 255.0f);
            adjust[c][i] = iClamp(iRound(s), 0, 255);
        }
    }

    // no gamma on alpha
    for (int i = 0; i < 256; ++i) {
        const float s = ((float(i) * modulate[3] / 255.0f + offset[3]) * 255.0f);
        adjust[3][i] = iClamp(iRound(s), 0, 255);
    }

    switch (fmt) {
    case ImageFormat::CODE_RGBA8:
        alwaysAssertM(n % 4 == 0, "RGBA8 images must have a multiple of 4 bytes");
        if (! convertToPremultipliedAlpha) {
            for (int i = 0; i < n; ) {
                for (int c = 0; c < 4; ++c, ++i) {
                    dst[i] = adjust[c][byte[i]];
                }
            }
        } else {
            for (int i = 0; i < n; i += 4) {
                const int a = adjust[3][byte[i + 3]];
                for (int c = 0; c < 3; ++c) {
                    dst[i + c] = (int(adjust[c][byte[i + c]]) * a) / 255;
                }
                dst[i + 3] = a;
            }
        }
        break;

    case ImageFormat::CODE_RGB8:
        alwaysAssertM(n % 3 == 0, "RGB8 images must have a multiple of 3 bytes");
        for (int i = 0; i < n; ) {
            for (int c = 0; c < 3; ++c, ++i) {
                dst[i] = adjust[c][byte[i]];
            }
        }
        break;

    case ImageFormat::CODE_R8:
    case ImageFormat::CODE_L8:
        for (int i = 0; i < n; ++i) {
            dst[i] = adjust[0][byte[i]];
        }
        break;

    default:;
    }
}

}
