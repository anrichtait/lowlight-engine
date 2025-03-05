/**
  \file G3D-app.lib/source/BumpMap.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/BumpMap.h"
#include "G3D-base/Any.h"
#include "G3D-base/CPUPixelTransferBuffer.h"

namespace G3D {

bool BumpMap::Specification::operator==(const Specification& other) const {
    return (texture == other.texture) && (settings == other.settings);
}


BumpMap::Specification::Specification(const Any& any) {
    if (any.type() == Any::STRING) {
        // Treat as a filename
        texture.filename = any.resolveStringAsFilename();
        texture.preprocess = Texture::Preprocess::normalMap();
    } else {
        AnyTableReader r(any);
        Any a;
        r.getIfPresent("texture", a);
        if (a.type() == Any::STRING) {
            texture.filename = a.resolveStringAsFilename();
            texture.preprocess = Texture::Preprocess::normalMap();
        } else {
            texture = Texture::Specification(a);
        }
        r.getIfPresent("settings", settings);
        r.verifyDone();
    }
}


BumpMap::BumpMap(const shared_ptr<MapComponent<Image4>>& normalBump, const Settings& settings) : 
    m_normalBump(normalBump), m_settings(settings) {
}


shared_ptr<BumpMap> BumpMap::create(const shared_ptr<MapComponent<Image4>>& normalBump, const Settings& settings) {
    return createShared<BumpMap>(normalBump, settings);
}


shared_ptr<BumpMap> BumpMap::create(const Specification& spec) {
    return BumpMap::create(MapComponent<Image4>::create(nullptr, Texture::create(spec.texture)), spec.settings);    
}


///////////////////////////////////////////////////////////

BumpMap::Settings::Settings(const Any& any) {
    *this = Settings();
    AnyTableReader r(any);
    any.verifyName("BumpMap::Settings");
    r.getIfPresent("iterations", iterations);
    iterations = max(0, iterations);
    r.getIfPresent("scale", scale);
    r.getIfPresent("bias", bias);
    r.verifyDone();
}


Any BumpMap::Settings::toAny() const {
    Any any(Any::TABLE, "BumpMap::Settings");
    static const Settings defaults;

    if (scale != defaults.scale) { any["scale"] = scale; }
    if (bias != defaults.bias) { any["bias"] = bias; }
    if (iterations != defaults.iterations) { any["iterations"] = iterations; }
    return any;
}


bool BumpMap::Settings::operator==(const Settings& other) const {
    return 
        (scale == other.scale) &&
        (bias == other.bias) &&
        (iterations == other.iterations);
}


shared_ptr<PixelTransferBuffer> BumpMap::computeNormalMap
(int                 width,
 int                 height,
 int                 channels,
 const unorm8*       src,
 const BumpMapPreprocess& preprocess) {

    const shared_ptr<CPUPixelTransferBuffer>& normal = CPUPixelTransferBuffer::create(width, height, ImageFormat::RGBA8());
    float       whiteHeightInPixels  = preprocess.zExtentPixels;
    const bool  lowPassBump          = preprocess.lowPassFilter;
    const bool  scaleHeightByNz      = preprocess.scaleZByNz;
    
    if (whiteHeightInPixels < 0.0f) {
        // Default setting scales so that a gradient ramp
        // over the whole image becomes a ~10-degree angle
        
        // Account for potentially non-square aspect ratios
        whiteHeightInPixels = max(width, height) * -whiteHeightInPixels * 0.15f;
    }

    debugAssert(whiteHeightInPixels >= 0);

    const int w = width;
    const int h = height;
    const int stride = channels;
    
    const unorm8* const B = src;
    Color4unorm8* const N = static_cast<Color4unorm8*>(normal->buffer());

    // 1/s for the scale factor that each ELEVATION should be multiplied by.
    // We avoid actually multiplying by this and instead just divide it out of z.
    float elevationInvScale = 255.0f / whiteHeightInPixels;

    runConcurrently(Point2int32(0, 0), Point2int32(w, h), [&](const Point2int32& P) {
        const int x = P.x;
        const int y = P.y;
        // Index into normal map pixel
        int i = x + y * w;

        // Index into bump map *byte*
        int j = stride * i;

        Vector3 delta;

        // Get a value from B (with wrapping lookup) relative to (x, y)
        // and divide by 255
#       define ELEVATION(DX, DY)  ((int)(B[(((DX + x + w) % w) +               \
                                    ((DY + y + h) % h) * w) * stride].bits()))

        // Sobel filter to compute the normal.  
        //
        // Y Filter (X filter is the transpose)
        //  [ -1 -2 -1 ]
        //  [  0  0  0 ]
        //  [  1  2  1 ]

        // Write the Y value directly into the x-component so we don't have
        // to explicitly compute a cross product at the end.  Does not 
        // go out of bounds because the above is computed mod (width, height)
        delta.y = float(-( ELEVATION(-1, -1) * 1 +  ELEVATION( 0, -1) * 2 +  ELEVATION( 1, -1) * 1 +
                    -ELEVATION(-1,  1) * 1 + -ELEVATION( 0,  1) * 2 + -ELEVATION( 1,  1) * 1));

        delta.x = float(-(-ELEVATION(-1, -1) * 1 + ELEVATION( 1, -1) * 1 + 
                    -ELEVATION(-1,  0) * 2 + ELEVATION( 1,  0) * 2 + 
                    -ELEVATION(-1,  1) * 1 + ELEVATION( 1,  1) * 1));

        // The scale of each filter row is 4, the filter width is two pixels,
        // and the "normal" range is 0-255.
        delta.z = 4 * 2 * elevationInvScale;

        // Delta is now scaled in pixels; normalize 
        delta = delta.direction();

        // Copy over the bump value into the alpha channel.
        float H = B[j];

        if (lowPassBump) {
            H = (ELEVATION(-1, -1) + ELEVATION( 0, -1) + ELEVATION(1, -1) +
                ELEVATION(-1,  0) + ELEVATION( 0,  0) + ELEVATION(1,  0) +
                ELEVATION(-1,  1) + ELEVATION( 0,  1) + ELEVATION(1,  1)) / (255.0f * 9.0f);
        }
#       undef ELEVATION

        if (scaleHeightByNz) {
            // delta.z can't possibly be negative, so we avoid actually
            // computing the absolute value.
            H *= delta.z;
        }

        N[i].a = unorm8(H);

        // Pack into byte range
        delta = delta * 0.5f + Vector3(0.5f, 0.5f, 0.5f);
        N[i].r = unorm8(delta.x);
        N[i].g = unorm8(delta.y);
        N[i].b = unorm8(delta.z);
    });

    return normal;
}


shared_ptr<PixelTransferBuffer> BumpMap::computeBumpMap
(const shared_ptr<PixelTransferBuffer>& _normalMap,
 float signConvention) {

    const shared_ptr<Image>& normalMap = Image::fromPixelTransferBuffer(_normalMap);

    const int w = normalMap->width();
    const int h = normalMap->height();

    // Compute the laplacian once; it never changes
    shared_ptr<Image> laplacian = Image::create(w, h, ImageFormat::R32F());

    runConcurrently(Point2int32(0, 0), Point2int32(w, h), [&](const Point2int32& P) {
        const int x = P.x;
        const int y = P.y;
        float ddx = normalMap->get<Color3>(x + 1, y, WrapMode::TILE).r - normalMap->get<Color3>(x - 1, y, WrapMode::TILE).r;
        float ddy = normalMap->get<Color3>(x, y + 1, WrapMode::TILE).g - normalMap->get<Color3>(x, y - 1, WrapMode::TILE).g;
        laplacian->set(x, y, Color1(ddx + signConvention * ddy) * 0.5f);
    });

    // Ping-pong buffers
    shared_ptr<Image> src = Image::create(w, h, ImageFormat::R32F());
    shared_ptr<Image> dst = Image::create(w, h, ImageFormat::R32F());

    dst->setAll(Color1(0.5f));

    // Number of Poisson iterations
    const int N = 100;
    for (int i = 0; i < N; ++i) {
        // Swap buffers
        std::swap(src, dst);

        runConcurrently(Point2int32(0, 0), Point2int32(w, h), [&](const Point2int32& P) {
            const int x = P.x;
            const int y = P.y;
            dst->set(x, y,
                        Color1((src->get<Color1>(x - 1, y, WrapMode::TILE).value + src->get<Color1>(x, y - 1, WrapMode::TILE).value +
                                src->get<Color1>(x + 1, y, WrapMode::TILE).value + src->get<Color1>(x, y + 1, WrapMode::TILE).value + 
                                laplacian->get<Color1>(x, y).value) * 0.25f));
        });
        debugPrintf("On pass %d/%d\n", i, N);
    }

    float lo = finf(), hi = -finf();
    runConcurrently(Point2int32(0, 0), Point2int32(w, h), [&](const Point2int32& P) {
        const float v = dst->get<Color1>(P).value;
        lo = min(lo, v);
        hi = max(hi, v);
    });

    const shared_ptr<Image>& final = Image::create(w, h, ImageFormat::RGB8());
    runConcurrently(Point2int32(0, 0), Point2int32(w, h), [&](const Point2int32& P) {
        final->set(P, Color1((dst->get<Color1>(P).value - lo) / (hi - lo)));
    });

    return final->toPixelTransferBuffer();
}


void BumpMap::detectNormalBumpFormat(const unorm8* bytes, int numComponents, int numPixels, bool& hasBump, bool& hasNormal, bool& bumpInRed) {
    if (numComponents < 3) {
        // This *cannot* be a normal map because there aren't enough channels
        hasNormal = false;
        bumpInRed = true;
        hasBump = true;
        return;
    }

    debugAssert(numComponents <= 4);

    // Initial assumption
    hasBump   = true;
    hasNormal = true;
    bumpInRed = true;
    bool bumpInAlpha = false;

    // Use the same random number generator each time so that detection is deterministic
    Random rng(1000000, false);

    // Sample many pixels
    for (int i = 0; (i < 25) && (hasNormal || hasBump); ++i) {
        const int p = rng.integer(0, numPixels - 1);
        if (numComponents == 3) {
            const Color3unorm8& c = *reinterpret_cast<const Color3unorm8*>(bytes + (numComponents * p));

            // Does this look like a normal?
            const Vector3 v = Vector3(Color3(c)) * 2.0f - Vector3::one();
            float len = v.squaredLength();

            if (hasBump && max(abs(v.x - v.y), abs(v.x - v.z)) > 0.05f) {
                // The color channels vary a lot
                hasBump = false;
                bumpInRed = false;
            }

            hasNormal = hasNormal && ((len > 0.9f) && (len < 1.1f) && (v.z > 0.0f));
        } else {
            // 4 components
            const Color4unorm8& c = *reinterpret_cast<const Color4unorm8*>(bytes + (numComponents * p));

            // Does this look like a normal?
            const Vector3 v = Vector3(Color3(c.rgb())) * 2.0f - Vector3::one();
            float len = v.squaredLength();

            bumpInRed = bumpInRed && (max(abs(v.x - v.y), abs(v.x - v.z)) > 0.05f);
            hasNormal = hasNormal && ((len > 0.9f) && (len < 1.1f) && (v.z > 0.0f));
            bumpInAlpha = bumpInAlpha || (float(c.a) < 1.0f);
        }
    }

    if (numComponents == 4) {
        if (bumpInRed && bumpInAlpha) {
            if (hasNormal) {
                bumpInRed = false;
            } else {
                bumpInAlpha = false;
            }
        }
        hasBump = bumpInRed || bumpInAlpha;
    }

    if (((numComponents == 3) || bumpInRed) && hasBump && hasNormal) {
        // An all-gray texture will read as both...force it to be treated as a bump map,
        // since that is an extremely unlikely normal map
        hasNormal = false;
    }
}

} // G3D
