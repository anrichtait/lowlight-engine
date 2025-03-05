/**
  \file G3D-gfx.lib/source/Texture_Visualization.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Args.h"
namespace G3D {

Texture::Visualization::Visualization(Channels c, float g, float mn, float mx, bool m1, bool motVec) : 
    channels(c), documentGamma(g), min(mn), max(mx), invertIntensity(false), layer(0), mipLevel(0), mod1(m1), showMotionVectors(motVec) {
}


const static String name[] = 
    {"RGB", "R", "G", "B", "RasL", "GasL", "BasL", "AasL", "MeanRGBasL", "Luminance", ""};


static String channelsToString(const Texture::Visualization::Channels& c) {
    return name[c];
}


static Texture::Visualization::Channels toChannels(const String& s) {

    for (int i = 0; ! name[i].empty(); ++i) {
        if (s == name[i]) {
            return Texture::Visualization::Channels(i);
        }
    }

    return Texture::Visualization::RGB;
}


Matrix4 Texture::Visualization::colorShiftMatrix() {
    static Matrix4 colorShift[] = {
        // RGB
        Matrix4(1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 0),

        // R
        Matrix4(1, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0),

        // G
        Matrix4(0, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0),

        // B
        Matrix4(0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 0),

        // RasL
        Matrix4(1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
        0, 0, 0, 0),
        // GasL
        Matrix4(0, 1, 0, 0,
        0, 1, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 0),

        // BasL
        Matrix4(0, 0, 1, 0,
        0, 0, 1, 0,
        0, 0, 1, 0,
        0, 0, 0, 0),

        // AasL
        Matrix4(0, 0, 0, 1,
        0, 0, 0, 1,
        0, 0, 0, 1,
        0, 0, 0, 0),

        // MeanRGBasL
        Matrix4(1, 1, 1, 0,
        1, 1, 1, 0,
        1, 1, 1, 0,
        0, 0, 0, 0) * (1.0f / 3.0f),

        // Luminance
        Matrix4(0.2126f, 0.7152f, 0.0722f, 0,
        0.2126f, 0.7152f, 0.0722f, 0,
        0.2126f, 0.7152f, 0.0722f, 0,
        0, 0, 0, 0)
    };
    return colorShift[channels];
}


void Texture::Visualization::setShaderArgs(UniformTable& args) {
    
    args.setUniform("mipLevel", (float)(mipLevel));
    args.setUniform("adjustGamma", documentGamma / 2.2f);
    args.setUniform("bias", -min);
    args.setUniform("mod1", mod1);
    args.setUniform("showMotionFlow", showMotionVectors);
    args.setUniform("motionVectorSpacing", motionVectorSpacing);
    args.setUniform("motionVectorScale", motionVectorScale);
    args.setUniform("scale", 1.0f / (max - min));

    args.setUniform("invertIntensity", invertIntensity);
    args.setUniform("colorShift", colorShiftMatrix());

    args.setMacro("LAYER", layer);
}


Any Texture::Visualization::toAny() const {
    Any a(Any::TABLE, "Texture::Visualization");
    a["channels"] = channelsToString(channels);
    a["documentGamma"] = documentGamma;
    a["min"] = min;
    a["max"] = max;
    a["mod1"] = mod1;
    a["invertIntensity"] = invertIntensity;
    a["layer"] = layer;
    a["mipLevel"] = mipLevel;
    return a;
}


Texture::Visualization::Visualization(const Any& a) {
    *this = Visualization();
    if (a.type() == Any::ARRAY) {
        if (a.nameEquals("bumpInAlpha")) {
            *this = bumpInAlpha();
        } else if (a.nameEquals("defaults")) {
            *this = defaults();
        } else if (a.nameEquals("linearRGB")) {
            *this = linearRGB();
        } else if (a.nameEquals("depthBuffer")) {
            *this = depthBuffer();
        } else if (a.nameEquals("packedUnitVector")) {
            *this = packedUnitVector();
        } else if (a.nameEquals("radiance")) {
            *this = radiance();
        } else if (a.nameEquals("reflectivity")) {
            *this = reflectivity();
        } else if (a.nameEquals("sRGB")) {
            *this = sRGB();
        } else if (a.nameEquals("unitVector")) {
            *this = unitVector();
        } else if (a.nameEquals("textureCoordinates")) {
            *this = textureCoordinates();
        } else {
            a.verify(false, "Unrecognized Visualization factory method");
        }
    } else {
        a.verifyName("Texture::Visualization", "Visualization");

        AnyTableReader r(a);
        String c;

        if (r.getIfPresent("channels", c)) {
            channels = toChannels(c);
        }

        r.getIfPresent("documentGamma", documentGamma);
        r.getIfPresent("invertIntensity", invertIntensity);
        r.getIfPresent("max", max);
        r.getIfPresent("min", min);
        r.getIfPresent("mod1", mod1);
        r.getIfPresent("layer", layer);
        r.getIfPresent("mipLevel", mipLevel);

        r.verifyDone();
    }
}


const Texture::Visualization& Texture::Visualization::sRGB() {
    static const Texture::Visualization s(RGB, 2.2f, 0.0f, 1.0f);
    return s;
}


const Texture::Visualization& Texture::Visualization::unitVector() {
    static const Texture::Visualization s(RGB, 1.0f, -1.0f, 1.0f);
    return s;
}

const Texture::Visualization& Texture::Visualization::textureCoordinates() {
    static const Texture::Visualization s(RGB, 1.0f, 0.0f, 1.0f, true);
    return s;
}


const Texture::Visualization& Texture::Visualization::depthBuffer() {
    static const Texture::Visualization s(RasL, 9.0f, 0.2f, 1.0f);
    return s;
}


const Texture::Visualization& Texture::Visualization::bumpInAlpha() {
    static const Texture::Visualization s(AasL, 1.0f, 0.0f, 1.0f);
    return s;
}


const Texture::Visualization& Texture::Visualization::motionVectors() {
    static const Texture::Visualization s(RGB, 1.0f, 0.0f, 1.0f, false, true);
    return s;
}


const Texture::Visualization& Texture::Visualization::defaults() {
    static const Texture::Visualization s;
    return s;
}

}
