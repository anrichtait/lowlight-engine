/**
  \file G3D-app.lib/include/G3D-app/Component.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_Component_h
#define G3D_Component_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/ImageFormat.h"
#include "G3D-base/Image1.h"
#include "G3D-base/Image3.h"
#include "G3D-base/Image4.h"
#include "G3D-gfx/Texture.h"

namespace G3D {

/** Used by Component */
enum ImageStorage { 
    /** Ensure that all image data is stored exclusively on the CPU. */
    MOVE_TO_CPU = 0, 

    /** Ensure that all image data is stored exclusively on the GPU. */
    MOVE_TO_GPU = 1, 

    /** Ensure that all image data is stored at least on the CPU. */
    COPY_TO_CPU = 2, 

    /** Ensure that all image data is stored at least on the GPU. */
    COPY_TO_GPU = 3,

    /** Do not change image storage */
    IMAGE_STORAGE_CURRENT = 4
};


class ImageUtils {
public:
    /** Returns the equivalent 8-bit version of a float format */
    static const ImageFormat* to8(const ImageFormat* f);
};

#ifdef __clang__
#   define DO_NOT_WARN_IF_UNUSED __attribute__ ((unused))
#else
#   define DO_NOT_WARN_IF_UNUSED
#endif

static DO_NOT_WARN_IF_UNUSED Color4 handleTextureEncoding(const Color4& c, const shared_ptr<Texture>& t) {
    if (notNull(t)) {
        const Texture::Encoding& encoding = t->encoding();
        return c * encoding.readMultiplyFirst + encoding.readAddSecond;
    } else {
        return c;
    }
}


static DO_NOT_WARN_IF_UNUSED Color3 handleTextureEncoding(const Color3& c, const shared_ptr<Texture>& t) {
    if (notNull(t)) {
        const Texture::Encoding& encoding = t->encoding();
        return c * encoding.readMultiplyFirst.rgb() + encoding.readAddSecond.rgb();
    } else {
        return c;
    }
}


/** Manages CPU and GPU versions of image data and performs
    conversions as needed.

    \param Image the CPU floating-point image format to use.  On the GPU, the corresponding uint8 format is used.

    Primarily used by Component. */
template<class Image>
class MapComponent : public ReferenceCountedObject {
protected:
            
#   define MyType class MapComponent<Image>

    shared_ptr<Image>                       m_cpuImage;
    shared_ptr<Texture>                     m_gpuImage;

    /** True when stats are out of date */
    mutable bool                            m_needsForce;
    mutable typename Image::StorageType     m_min;
    mutable typename Image::StorageType     m_max;
    mutable typename Image::ComputeType     m_mean;

    static void getTexture(const shared_ptr<Image>& im, shared_ptr<Texture>& tex) {
            
        Texture::Dimension dim;
        if (isPow2(im->width()) && isPow2(im->height())) {
            dim = Texture::DIM_2D;
        } else {
            dim = Texture::DIM_2D;
        }

        Texture::Encoding e;
        e.format = ImageUtils::to8(im->format());
        tex = Texture::fromMemory
            ("Converted", im->getCArray(), im->format(),
             im->width(), im->height(), 1, 1, e, dim);
    }

    static void convert(const Color4& c, Color3& v) {
        v = c.rgb();
    }

    static void convert(const Color4& c, Color4& v) {
        v = c;
    }

    static void convert(const Color4& c, Color1& v) {
        v.value = c.r;
    }

    MapComponent(const class shared_ptr<Image>& im, const shared_ptr<Texture>& tex) : 
        m_cpuImage(im),
        m_gpuImage(tex),
        m_min(Image::StorageType::one()),
        m_max(Image::StorageType::zero()),
        m_mean(Image::ComputeType::zero()) {

        // Compute min, max, mean
        if (m_gpuImage) {
            m_needsForce = true;
        } else {
            m_needsForce = false;
            computeCPUStats();
        }
    }

    void computeCPUStats() {
        bool cpuWasNull = isNull(m_cpuImage);

        if (isNull(m_cpuImage) && m_gpuImage) {
            m_gpuImage->getImage(m_cpuImage);
        }

        if (m_cpuImage) {
            const typename Image::StorageType* ptr = m_cpuImage->getCArray();
            typename Image::ComputeType sum = Image::ComputeType::zero();
            const int N = m_cpuImage->width() * m_cpuImage->height();
            for (int i = 0; i < N; ++i) {
                m_min  = m_min.min(ptr[i]);
                m_max  = m_max.max(ptr[i]);
                sum   += typename Image::ComputeType(ptr[i]);
            }
            m_mean = sum / (float)N;
        }

        if (cpuWasNull) {
            // Throw away the CPU image to conserve memory
            m_cpuImage.reset();
        }
    }

    /** Used to compute min/max/mean from m_gpuImage without
        triggering a Texture::force() at construction time. */
    void forceStats() const {
        if (m_needsForce) {
            if (m_gpuImage && m_gpuImage->min().isFinite()) {
                // Use previously computed data stored in the texture
                convert(m_gpuImage->min(), m_min);
                convert(m_gpuImage->max(), m_max);
                convert(m_gpuImage->mean(), m_mean);
            } else {
                const_cast<MapComponent<Image>*>(this)->computeCPUStats();
            }
            m_needsForce = false;
        }
    }



public:

    /** Returns nullptr if both are nullptr */
    static shared_ptr<MapComponent<Image> > create
    (const class shared_ptr<Image>& im, 
     const shared_ptr<Texture>&                         tex) {

        if (isNull(im) && isNull(tex)) {
            return shared_ptr<MapComponent<Image> >();
        } else {
            return createShared<MapComponent<Image>>(im, tex);
        }
    }

    /** Largest value in each channel of the image */
    const typename Image::StorageType& max() const {
        forceStats();
        return m_max;
    }

    /** Smallest value in each channel of the image */
    const typename Image::StorageType& min() const {
        forceStats();
        return m_min;
    }

    /** Average value in each channel of the image */
    const typename Image::ComputeType& mean() const {
        forceStats();
        return m_mean;
    }
           
    /** Returns the CPU image portion of this component, synthesizing
        it if necessary.  Returns nullptr if there is no GPU data to 
        synthesize from.*/
    const class shared_ptr<Image>& image() const {
        MyType* me = const_cast<MyType*>(this);
        if (isNull(me->m_cpuImage)) {
            debugAssert(me->m_gpuImage);
            // Download from GPU.  This works because C++
            // dispatches the override on the static type,
            // so it doesn't matter that the pointer is nullptr
            // before the call.
            m_gpuImage->getImage(me->m_cpuImage);
        }
                
        return m_cpuImage;
    }
    

    /** Returns the GPU image portion of this component, synthesizing
        it if necessary.  */
    const shared_ptr<Texture>& texture() const {
        MyType* me = const_cast<MyType*>(this);
        if (isNull(me->m_gpuImage)) {
            debugAssert(me->m_cpuImage);
                    
            // Upload from CPU
            getTexture(m_cpuImage, me->m_gpuImage);
        }
                
        return m_gpuImage;
    }

    void setStorage(ImageStorage s) const {
        MyType* me = const_cast<MyType*>(this);
        switch (s) {
        case MOVE_TO_CPU:
            image();
            me->m_gpuImage.reset();
            break;

        case MOVE_TO_GPU:
            texture();
            me->m_cpuImage.reset();
            break;

        case COPY_TO_GPU:
            texture();
            break;

        case COPY_TO_CPU:
            image();
            break;

        case IMAGE_STORAGE_CURRENT:
            // Nothing to do
            break;
        }
    }

#   undef MyType
};


/** \brief Common code for G3D::Component1, G3D::Component3, and G3D::Component4.

    Product of a constant and an image. 

    The image may be stored on either the GPU (G3D::Texture) or
    CPU (G3D::Map2D subclass), and both factors are optional. The
    details of this class are rarely needed to use UniversalMaterial, since
    it provides constructors from all combinations of data
    types.
    
    Supports only floating point image formats because bilinear 
    sampling of them is about 9x faster than sampling int formats.
    */
template<class Color, class Image>
class Component {
private:

    mutable bool                        m_needsComputeStats;
    mutable Color                       m_max;
    mutable Color                       m_min;
    mutable Color                       m_mean;

    /** nullptr if there is no map. This is a reference so that
        multiple Components may share a texture and jointly
        move it to and from the GPU.*/
    shared_ptr<MapComponent<Image> >  m_map;

    void init() {
        m_max  = Color::nan();
        m_min  = Color::nan();
        m_mean = Color::nan();
        m_needsComputeStats = true;
    }

    void computeStats() const {
        if (m_needsComputeStats) {
            if (notNull(m_map)) {
                m_max  = Color(m_map->max());
                m_min  = Color(m_map->min());
                m_mean = Color(m_map->mean()); 
            }
            m_needsComputeStats = false;
        }
    }

    static float alpha(const Color1& c) {
        return 1.0;
    }
    static float alpha(const Color3& c) {
        return 1.0;
    }
    static float alpha(const Color4& c) {
        return c.a;
    }


public:
        
    /** Assumes a map of nullptr if not specified */
    Component
    (const shared_ptr<MapComponent<Image> >& map = shared_ptr<MapComponent<Image> >()) :
        m_map(map) {
        init();
    }

    Component
    (const shared_ptr<Image>& map) :
        m_map(MapComponent<Image>::create(map, shared_ptr<Texture>())) {
        init();
    }

    Component
    (const shared_ptr<Texture>&             map) :
        m_map(MapComponent<Image>::create(shared_ptr<Image>(), map)) {
        init();
    }
    
 

    bool operator==(const Component<Color, Image>& other) const {
        return (m_map == other.m_map);
    }
        
    /** Return map sampled at \param pos.  Optimized to only perform as many
        operations as needed.

        If the component contains a texture map that has not been
        converted to a CPU image, that conversion is
        performed. Because that process is not threadsafe, when using
        sample() in a multithreaded environment, first invoke setStorage(COPY_TO_CPU)
        on every Component from a single thread to prime the CPU data
        structures.

        Coordinates are normalized; will be scaled by the image width and height
        automatically.
    */
    Color sample(const Vector2& pos) const {
        if (isNull(m_map)) {
            return Color::zero();
        }
        debugAssertM(notNull(m_map), "Tried to sample a component without a map");
        const shared_ptr<Image>& im = m_map->image();
        return handleTextureEncoding(im->bilinear(pos * Vector2(float(im->width()), float(im->height()))), m_map->texture());
    }

    /** Largest value per color channel */
    inline const Color& max() const {
        computeStats();
        return m_max;
    }

    /** Smallest value per color channel */
    inline const Color& min() const {
        computeStats();
        return m_min;
    }

    /** Average value per color channel */
    inline const Color& mean() const {
        computeStats();
        return m_mean;
    }

    /** Causes the image to be created by downloading from GPU if necessary. */
    inline const shared_ptr<Image>& image() const {
        static const shared_ptr<Image> nullTexture;
        return notNull(m_map) ? m_map->image() : nullTexture;
    }

    /** Causes the texture to be created by uploading from CPU if necessary. */
    inline const shared_ptr<Texture>& texture() const {
        static const shared_ptr<Texture> nullTexture;
        return notNull(m_map) ? m_map->texture() : nullTexture;
    }

    /** Does not change storage if the map is nullptr */
    inline void setStorage(ImageStorage s) const {
        if (notNull(m_map)) { m_map->setStorage(s); }
    }

    /** Says nothing about the alpha channel */
    inline bool notBlack() const {
        return ! isBlack();
    }

    /** Returns true if there is non-unit alpha. */
    inline bool nonUnitAlpha() const {
        return (alpha(this->min()) != 1.0f);
    }

    /** Says nothing about the alpha channel */
    inline bool isBlack() const {
        return this->max().rgb().max() == 0.0f;
    }
};

typedef Component<Color1, Image1> Component1;

typedef Component<Color3, Image3> Component3;

typedef Component<Color4, Image4> Component4;

}

#endif
