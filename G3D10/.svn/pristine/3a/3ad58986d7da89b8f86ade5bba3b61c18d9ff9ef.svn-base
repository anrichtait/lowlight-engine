/**
  \file G3D-app.lib/include/G3D-app/OptiXTriTree.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once

#include "G3D-base/platform.h"

#if defined(G3D_WINDOWS)

#include "G3D-base/HashTrait.h"
#include "G3D-base/Table.h"
#include "G3D-app/Material.h"
#include "G3D-app/TriTreeBase.h"


#ifdef _DEBUG
#    pragma comment(lib, "waved.lib")
#else
#    pragma comment(lib, "wave.lib")
#endif

// Forward declaration
namespace wave { 
    class BVH; 
    using TimingCallback = void(int, const char*, const char*, void*);
}

namespace G3D {

class UniversalMaterial;

class OptiXTriTree : public TriTreeBase {

    typedef int GeometryIndex, MaterialIndex;
    typedef uint64 SurfaceId;

protected:

    wave::BVH* m_bvh;

    G3D_DECLARE_ENUM_CLASS(Field, MATERIAL0, MATERIAL1, MATERIAL2, MATERIAL3, HIT_LOCATION, SHADING_NORMAL, POSITION, GEOMETRIC_NORMAL, HIT);

    struct SurfaceCacheElement {
        GeometryIndex       geometryIndex = 0;
        // Set to false for garbage collection in setContents()
        bool                live = false;
        SurfaceCacheElement() {}
        SurfaceCacheElement(GeometryIndex i) : geometryIndex(i) {}
    };

    struct VertexCacheElement {
        Array<Point3>   position;
        Array<Vector3>  normal;
        Array<Vector4>  tangent;
        Array<Point2>   texCoord;
    };

    using SurfaceCache = Table<SurfaceId, SurfaceCacheElement>;
    using VertexCache = Table<const CPUVertexArray*, VertexCacheElement>;

    mutable int                                             m_outWidth = -1;
    mutable int                                             m_outHeight = -1;
    mutable Array<shared_ptr<GLPixelTransferBuffer>>        m_outPBOArray;
    mutable SurfaceCache                                    m_surfaceCache;
    mutable VertexCache                                     m_vertexCache;
    mutable Table<shared_ptr<Material>, MaterialIndex>      m_materialCache;
    mutable Table<int, weak_ptr<GLPixelTransferBuffer>>     m_registeredBufferIDs;
    
    // Cache for grouping surfaces by CFrame
    using CFrameCache = Table<CFrame, int>;
    mutable int                                             m_nextFrameCacheID = 0;
    mutable CFrameCache                                     m_frameCache;


    // For public methods that take rays in non-texture form, copy the
    // ray data into these member textures, mirroring the caching on wave.lib.
    mutable shared_ptr<GLPixelTransferBuffer>                                     m_rayOrigins;
    mutable shared_ptr<GLPixelTransferBuffer>                                     m_rayDirections;
    
    // Need this ignore texture so that materials without the full set of textures have a texture that doesn't go out of scope.
    shared_ptr<Texture> m_ignoreTexture;

    // The width and height of the *result* textures are returned in width and height
    // Note that the ray textures may be larger than this.
    void copyToRayPBOs(const Array<Ray>& rays) const;
    //shared_ptr<Texture> convertToOptiXFormat(shared_ptr<Texture> t);

    static inline bool optixSupportsTexture(const shared_ptr<Texture>& tex) {
        // http://docs.nvidia.com/gameworks/content/gameworkslibrary/optix/optixapireference/group___open_g_l.html
        // http://docs.nvidia.com/gameworks/content/gameworkslibrary/optix/optixapireference/optix__gl__interop_8h.html#a04e1ab43df38124e9902ea98238bb1b4
        // http://raytracing-docs.nvidia.com/optix/guide/index.html#texture#15001

        const ImageFormat* fmt = tex->format();
        return (fmt->numComponents != 3) && (fmt->colorSpace == ImageFormat::ColorSpace::COLOR_SPACE_RGB);
    }

    void intersectRays
        (const shared_ptr<GLPixelTransferBuffer>&         rayOrigins,
        const shared_ptr<GLPixelTransferBuffer>&         rayDirections,
        Array<shared_ptr<Surfel>>&         results,
        IntersectRayOptions                options) const;

    static shared_ptr<Texture> convertToOptiXFormat(const shared_ptr<Texture>& tex);

    void ensureMaterialCached(const shared_ptr<UniversalMaterial>& material);

    void registerReallocationAndMapHooks(const shared_ptr<GLPixelTransferBuffer>& t) const;

    OptiXTriTree();

public:

    virtual ~OptiXTriTree();

    // TODO: remove, only exposed for wave testing.
    wave::BVH* BVH() {
        return m_bvh;
    }

    static shared_ptr<OptiXTriTree> create() { 
        return createShared<OptiXTriTree>();
    }
    /** If false after construction then OptiXTriTree will not work on this machine and 
        you should switch to EmbreeTriTree. This cannot be static because it has to try
        to construct an OptiX context. */
    bool supported();

    virtual void setContents
        (const Array<Tri>&                  triArray,
        const CPUVertexArray&               vertexArray,
        ImageStorage                        newStorage = ImageStorage::COPY_TO_CPU) {
        alwaysAssertM(false, "Not Implemented! Use OptiXTriTree::setContents(const Array<shared_ptr<Surface> >& surfaceArray, ImageStorage newStorage)");
    }

    virtual void setContents
    (const Array<shared_ptr<Surface> >& surfaceArray,
        ImageStorage                       newStorage = ImageStorage::COPY_TO_CPU) override;

    virtual const String& className() const override { static const String n = "OptiXTriTree"; return n; }

    virtual void rebuild() override {}

    void intersectRays
       (const shared_ptr<GLPixelTransferBuffer>&          rayOrigins,
        const shared_ptr<GLPixelTransferBuffer>&          rayDirections,
        Array<Hit>&                         results,
        IntersectRayOptions                 options) const;

    /** Values in results will be reused if already allocated, which can increase performance*/
    virtual void intersectRays
       (const Array<Ray>&                   rays,
        Array<shared_ptr<Surfel>>&          results,
        IntersectRayOptions                 options = IntersectRayOptions(0),
        const Array<float>&                 coneBuffer = Array<float>()) const override;

    /** Intersect a single ray. Return value is `hit.triIndex != Hit::NONE` for convenience.
    \param filterFunction Set to nullptr to accept any geometric ray-triangle instersection.
    */
    virtual bool intersectRay
       (const Ray&                         ray,
        Hit&                               hit,
        IntersectRayOptions                options = IntersectRayOptions(0)) const override;

    /** \param booleanResults The red channel is nonzero on hit, 0 on miss. Subclasses are free to change the format of the
    \a booleanResults texture to whatever is most convenient for them, so make no assumptions other than that it has
    a red channel. */

    virtual void intersectRays
       (const Array<Ray>&                  rays,
        Array<bool>&                       results,
        IntersectRayOptions                options = IntersectRayOptions(0)) const override;


    /** Batch ray casting. The default implementation calls the single-ray version using
    Thread::runConcurrently. */
    virtual void intersectRays
       (const Array<Ray>&                  rays,
        Array<Hit>&                        results,
        IntersectRayOptions                options = IntersectRayOptions(0)) const override;

    /** \param rayOrigin must be RGBA32F() = XYZ, min distance

    \param rayDirection must be RGBA32F() or RGBA16F() = normalized XYZ, max distance

    \param rayCone must be null or a single-channel (R-only) texture. If not null, each element is the cosine of the
    half-angle of the cone about \a rayDirection that should be used to select a MIP-level at the intersection point.
    The easy way to compute this for primary rays is to pass the dot products of adjacent pixel ray directions.

    The GBuffer and both textures must have the same dimensions.

    Writes the following fields to the GLPixelTransferBuffers in order in the passed in array:

    - GBuffer::Field::WS_POSITION
    - GBuffer::Field::WS_NORMAL
    - GBuffer::Field::LAMBERTIAN
    - GBuffer::Field::GLOSSY
    - GBuffer::Field::EMISSIVE

    WS_NORMAL is zero at pixels where the ray misses

    All other fields are ignored. The GBuffer may be reallocated with textures
    in a different format as well.

    The base class implementation copies all data to the CPU, invokes the
    intersectRays overload that accepts CPU data, and then copies all data
    back to the GPU.
    */
    virtual void intersectRays
    (const shared_ptr<GLPixelTransferBuffer>& rayOrigin,
        const shared_ptr<GLPixelTransferBuffer>& rayDirection,
        const shared_ptr<GLPixelTransferBuffer>  results[5],
        IntersectRayOptions                      options = IntersectRayOptions(0),
        const shared_ptr<GLPixelTransferBuffer>& rayCone = nullptr,
        const int baseMipLevel = 0,
		const Vector2int32 wavefrontDimensions = Vector2int32(-1,-1),
		const RenderMask mask = 0xFF) const override;

    virtual void intersectRays
    (const shared_ptr<GLPixelTransferBuffer>& rayOrigin,
        const shared_ptr<GLPixelTransferBuffer>& rayDirection,
        const shared_ptr<GLPixelTransferBuffer>& booleanResults,
        IntersectRayOptions                      options = IntersectRayOptions(0)) const override;

    /** Set a timing function callback to print timing details from OptiX. */
    void setTimingCallback(wave::TimingCallback* callback, int verbosityLevel);
};

}

#endif
