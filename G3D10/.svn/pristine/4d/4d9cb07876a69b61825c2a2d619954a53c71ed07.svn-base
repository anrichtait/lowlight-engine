/**
  \file G3D-base.lib/source/Image3unorm8.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/Image1unorm8.h"
#include "G3D-base/Image3unorm8.h"
#include "G3D-base/Image3.h"
#include "G3D-base/Image4.h"
#include "G3D-base/Image.h"
#include "G3D-base/Color1.h"
#include "G3D-base/Color1unorm8.h"
#include "G3D-base/Color4.h"
#include "G3D-base/Color4unorm8.h"
#include "G3D-base/ImageFormat.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/PixelTransferBuffer.h"
#include "G3D-base/CPUPixelTransferBuffer.h"


namespace G3D {

shared_ptr<Image3unorm8> Image3unorm8::fromImage1unorm8(const shared_ptr<class Image1unorm8>& im) {
    return fromArray(im->getCArray(), im->width(), im->height(), im->wrapMode());
}


Image3unorm8::Image3unorm8(int w, int h, WrapMode wrap) : Map2D<Color3unorm8, Color3>(w, h, wrap) {
    setAll(Color3unorm8(unorm8::zero(), unorm8::zero(), unorm8::zero()));
}


shared_ptr<Image3unorm8> Image3unorm8::fromImage3(const shared_ptr<Image3>& im) {
    return fromArray(im->getCArray(), im->width(), im->height(), static_cast<WrapMode>(im->wrapMode()));
}


shared_ptr<Image3unorm8> Image3unorm8::fromImage4(const shared_ptr<Image4>& im) {
    return fromArray(im->getCArray(), im->width(), im->height(), static_cast<WrapMode>(im->wrapMode()));
}



shared_ptr<Image3unorm8> Image3unorm8::createEmpty(int width, int height, WrapMode wrap) {
    return shared_ptr<Image3unorm8>(new Type(width, height, wrap));
}


shared_ptr<Image3unorm8> Image3unorm8::createEmpty(WrapMode wrap) {
    return createEmpty(0, 0, wrap);
}


shared_ptr<Image3unorm8> Image3unorm8::fromFile(const String& filename, WrapMode wrap) {
    shared_ptr<Image3unorm8> out = createEmpty(wrap);
    out->load(filename);
    return out;
}


shared_ptr<Image3unorm8> Image3unorm8::fromArray(const class Color3unorm8* ptr, int w, int h, WrapMode wrap) {
    shared_ptr<Image3unorm8> out = createEmpty(wrap);
    out->copyArray(ptr, w, h);
    return out;
}


shared_ptr<Image3unorm8> Image3unorm8::fromArray(const class Color1* ptr, int w, int h, WrapMode wrap) {
    shared_ptr<Image3unorm8> out = createEmpty(wrap);
    out->copyArray(ptr, w, h);
    return out;
}


shared_ptr<Image3unorm8> Image3unorm8::fromArray(const class Color1unorm8* ptr, int w, int h, WrapMode wrap) {
    shared_ptr<Image3unorm8> out = createEmpty(wrap);
    out->copyArray(ptr, w, h);
    return out;
}


shared_ptr<Image3unorm8> Image3unorm8::fromArray(const class Color3* ptr, int w, int h, WrapMode wrap) {
    shared_ptr<Image3unorm8> out = createEmpty(wrap);
    out->copyArray(ptr, w, h);
    return out;
}


shared_ptr<Image3unorm8> Image3unorm8::fromArray(const class Color4unorm8* ptr, int w, int h, WrapMode wrap) {
    shared_ptr<Image3unorm8> out = createEmpty(wrap);
    out->copyArray(ptr, w, h);
    return out;
}


shared_ptr<Image3unorm8> Image3unorm8::fromArray(const class Color4* ptr, int w, int h, WrapMode wrap) {
    shared_ptr<Image3unorm8> out = createEmpty(wrap);
    out->copyArray(ptr, w, h);
    return out;
}


void Image3unorm8::load(const String& filename) {
    shared_ptr<Image> image = Image::fromFile(filename);
    if (image->format() != ImageFormat::RGB8()) {
        image->convertToRGB8();
    }

    switch (image->format()->code)
    {
        case ImageFormat::CODE_L8:
            copyArray(static_cast<const Color1unorm8*>(dynamic_pointer_cast<CPUPixelTransferBuffer>(image->toPixelTransferBuffer())->buffer()), image->width(), image->height());
            break;
        case ImageFormat::CODE_L32F:
            copyArray(static_cast<const Color1*>(dynamic_pointer_cast<CPUPixelTransferBuffer>(image->toPixelTransferBuffer())->buffer()), image->width(), image->height());
            break;
        case ImageFormat::CODE_RGB8:
            copyArray(static_cast<const Color3unorm8*>(dynamic_pointer_cast<CPUPixelTransferBuffer>(image->toPixelTransferBuffer())->buffer()), image->width(), image->height());
            break;
        case ImageFormat::CODE_RGB32F:
            copyArray(static_cast<const Color3*>(dynamic_pointer_cast<CPUPixelTransferBuffer>(image->toPixelTransferBuffer())->buffer()), image->width(), image->height());
            break;
        case ImageFormat::CODE_RGBA8:
            copyArray(static_cast<const Color4unorm8*>(dynamic_pointer_cast<CPUPixelTransferBuffer>(image->toPixelTransferBuffer())->buffer()), image->width(), image->height());
            break;
        case ImageFormat::CODE_RGBA32F:
            copyArray(static_cast<const Color4*>(dynamic_pointer_cast<CPUPixelTransferBuffer>(image->toPixelTransferBuffer())->buffer()), image->width(), image->height());
            break;
        default:
            debugAssertM(false, "Trying to load unsupported image format");
            break;
    }

    setChanged(true);
}


void Image3unorm8::copyArray(const Color1unorm8* src, int w, int h) {
    resize(w, h);
    int N = w * h;

    Color3unorm8* dst = getCArray();
    for (int i = 0; i < N; ++i) {
        dst[i].r = dst[i].g = dst[i].b = src[i].value;
    }
}


void Image3unorm8::copyArray(const Color1* src, int w, int h) {
    resize(w, h);
    int N = w * h;

    Color3unorm8* dst = getCArray();
    for (int i = 0; i < N; ++i) {
        dst[i].r = dst[i].g = dst[i].b = Color1unorm8(src[i]).value;
    }
}


void Image3unorm8::copyArray(const Color3unorm8* ptr, int w, int h) {
    resize(w, h);
    System::memcpy(getCArray(), ptr, w * h * 3);
}


void Image3unorm8::copyArray(const Color3* src, int w, int h) {
    resize(w, h);
    int N = w * h;

    Color3unorm8* dst = getCArray();
    for (int i = 0; i < N; ++i) {
        dst[i] = Color3unorm8(src[i]);
    }
}


void Image3unorm8::copyArray(const Color4unorm8* ptr, int w, int h) {
    resize(w, h);
    
    int N = w * h;

    Color3unorm8* dst = getCArray();
    for (int i = 0; i < N; ++i) {
        dst[i] = Color3unorm8(ptr[i].r, ptr[i].g, ptr[i].b);
    }
}


void Image3unorm8::copyArray(const Color4* src, int w, int h) {
    resize(w, h);
    int N = w * h;

    Color3unorm8* dst = getCArray();
    for (int i = 0; i < N; ++i) {
        dst[i] = Color3unorm8(src[i].rgb());
    }
}


/** Saves in any of the formats supported by G3D::GImage. */
void Image3unorm8::save(const String& filename) {
    shared_ptr<CPUPixelTransferBuffer> buffer = CPUPixelTransferBuffer::create(width(), height(),  format(), MemoryManager::create());
    System::memcpy(buffer->buffer(), getCArray(), (size_t)width() * height() * format()->cpuBitsPerPixel / 8);
    shared_ptr<Image> image = Image::fromPixelTransferBuffer(buffer);
    image->save(filename);
}


shared_ptr<class Image1unorm8> Image3unorm8::getChannel(int c) const {
    debugAssert(c >= 0 && c <= 2);

    shared_ptr<Image1unorm8> dst = Image1unorm8::createEmpty(width(), height(), wrapMode());
    const Color3unorm8* srcArray = getCArray();
    Color1unorm8* dstArray = dst->getCArray();

    const int N = width() * height();
    for (int i = 0; i < N; ++i) {
        dstArray[i] = Color1unorm8(srcArray[i][c]);
    }

    return dst;
}


const ImageFormat* Image3unorm8::format() const {
    return ImageFormat::RGB8();
}

} // G3D
