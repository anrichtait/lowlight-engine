/**
  \file G3D-app.lib/include/G3D-app/TriTree.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once

#include "G3D-base/platform.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/Ray.h"
#include "G3D-base/Array.h"
#include "G3D-gfx/CPUVertexArray.h"
#include "G3D-app/Tri.h"
#ifndef _MSC_VER
#include <stdint.h>
#endif

namespace G3D {
class Surface;
class Surfel;
class Material;
class AABox;
class GBuffer;


/** Interface for ray-casting data structures.
   \sa G3D::TriTreeBase */
class TriTree : public ReferenceCountedObject {
protected:

    shared_ptr<CubeMap>     m_sky;
    Array<Tri>              m_triArray;
    CPUVertexArray          m_vertexArray;
    RealTime                m_lastBuildTime = -1e6;

public:

    /** CPU timing of API conversion overhead for the most recent call to intersectRays */
    mutable RealTime debugConversionOverheadTime = 0;

    /** Options for intersectRays. Default is full intersection with no backface culling optimization
        and partial coverage (alpha) test passing for values over 0.5. */
    typedef unsigned int IntersectRayOptions;

    /** Test for occlusion and do not necessarily return valid triIndex, backfacing, etc. data
        (useful for shadow rays and testing line of sight) */
    static const IntersectRayOptions OCCLUSION_TEST_ONLY = 1;

    /** Do not allow the intersector to perform backface culling as an
        optimization. Backface culling is not required in any case. */
    static const IntersectRayOptions DO_NOT_CULL_BACKFACES = 2;

    /** Only fail the partial coverage (alpha) test on zero coverage. */
    static const IntersectRayOptions PARTIAL_COVERAGE_THRESHOLD_ZERO = 4;

    /** Disable partial coverage (alpha) testing. */
    static const IntersectRayOptions NO_PARTIAL_COVERAGE_TEST = 8;

    /** Make optimizations appropriate for coherent rays (same origin) */
    static const IntersectRayOptions COHERENT_RAY_HINT = 16;

    class Hit {
    public:
        enum { NONE = -1 };
        /** NONE if no hit. For occlusion ray casts, this will be an undefined value not equal to NONE. */
        int         triIndex;
        float       u;
        float       v;
        float       distance;

        /** For occlusion ray casts, this will always be false. */
        bool        backface;

        Hit() : triIndex(NONE), u(0), v(0), distance(0), backface(false) {}
    };
    
    virtual const String& className() const = 0;

    virtual void clear() = 0;

    const Array<Tri>& triArray() const {
        return m_triArray;
    }

    const CPUVertexArray& vertexArray() const {
        return m_vertexArray;
    }

    /** If you mutate this, you must call rebuild() */
    Array<Tri>& triArray() {
        return m_triArray;
    }

    /** If you mutate this, you must call rebuild() */
    CPUVertexArray& vertexArray() {
        return m_vertexArray;
    }

    /** Array access to the stored Tris */
    const Tri& operator[](int i) const {
        debugAssert(i >= 0 && i < m_triArray.size());
        return m_triArray[i];
    }

    int size() const {
        return m_triArray.size();
    }

    /** Time at which setContents() or rebuild() was last invoked */
    RealTime lastBuildTime() const {
        return m_lastBuildTime;
    }

    /** Rebuild the tree after m_triArray or CPUVertexArray have been mutated. Called automatically by setContents() */
    virtual void rebuild() = 0;

    /** Base class implementation populates m_triArray and m_vertexArray and applies the image storage option. */
    virtual void setContents
        (const Array<shared_ptr<Surface>>&  surfaceArray, 
         ImageStorage                       newImageStorage = ImageStorage::COPY_TO_CPU) = 0;

    virtual void setContents
       (const Array<Tri>&                   triArray, 
        const CPUVertexArray&               vertexArray,
        ImageStorage                        newStorage = ImageStorage::COPY_TO_CPU) = 0;

    virtual void setContents
       (const shared_ptr<class Scene>&      scene, 
        ImageStorage                        newStorage = ImageStorage::COPY_TO_CPU) = 0;

    /** Intersect a single ray. Return value is `hit.triIndex != Hit::NONE` for convenience. 
      */
    virtual bool intersectRay
        (const Ray&                         ray,
         Hit&                               hit,
         IntersectRayOptions                options         = IntersectRayOptions(0)) const = 0;

protected:

    // Needed to make C++ resolve the overloaded/virtual/optional argument polymorphism 
    // pileup on intersectRay() when compiling TriTreeBase

    bool _intersectRay
        (const Ray&                         ray,
         Hit&                               hit,
         IntersectRayOptions                options) const {
        return intersectRay(ray, hit, options);
    }

public:
    
    /** Batch ray casting. The default implementation calls the single-ray version using
        Thread::runConcurrently. */
    virtual void intersectRays
        (const Array<Ray>&                  rays,
         Array<Hit>&                        results,
         IntersectRayOptions                options         = IntersectRayOptions(0)) const = 0;

    /** Values in results will be reused if already allocated, which can increase performance*/
    virtual void intersectRays
        (const Array<Ray>&                  rays,
         Array<shared_ptr<Surfel>>&         results,
         IntersectRayOptions                options         = IntersectRayOptions(0),
         const Array<float>&                coneBuffer      = Array<float>()) const = 0;

    /** \param rayOrigin must be RGBA32F() = XYZ, min distance
        
        \param rayDirection must be RGBA32F() or RGBA16F() = normalized XYZ, max distance
        
        \param rayCone must be null or a single-channel (R-only) texture. If not null, each element is the cosine of the
         half-angle of the cone about \a rayDirection that should be used to select a MIP-level at the intersection point.
         The easy way to compute this for primary rays is to pass the dot products of adjacent pixel ray directions.

        The GBuffer and both textures must have the same dimensions.

        Reconfigures the GBuffer and writes the following fields:

        - GBuffer::Field::WS_POSITION
        - GBuffer::Field::WS_NORMAL
        - GBuffer::Field::LAMBERTIAN
        - GBuffer::Field::GLOSSY
        - GBuffer::Field::EMISSIVE
        - GBuffer::Field::TRANSMISSIVE

        WS_NORMAL is zero at pixels where the ray misses

        All other fields are ignored. The GBuffer may be reallocated with textures
        in a different format as well.

        The base class implementation copies all data to the CPU, invokes the 
        intersectRays overload that accepts CPU data, and then copies all data
        back to the GPU.
     */
    virtual void intersectRays
        (const shared_ptr<Texture>&         rayOrigin,
         const shared_ptr<Texture>&         rayDirection,
         const shared_ptr<GBuffer>&         results,
         IntersectRayOptions                options         = IntersectRayOptions(0),
         const shared_ptr<Texture>&         rayCone         = nullptr) const = 0;

    
    /** \param rayOrigin must be RGBA32F() = XYZ, min distance
        
        \param rayDirection must be RGBA32F() or RGBA16F() = normalized XYZ, max distance
        
        \param rayCone must be null or a single-channel (R-only) texture. If not null, each element is the cosine of the
         half-angle of the cone about \a rayDirection that should be used to select a MIP-level at the intersection point.
         The easy way to approximate this for primary rays is to pass:
         `sqrt(dot(rayDir, adjacentRayDir) * 0.5 + 0.5)`. That is only exact for "square" rays, though, and given 
         the number of approximations involved in both MIP maps and approximating a square pixel footprint with a cone,
         dropping the square root is also reasonable: `dot(rayDir, adjacentRayDir) * 0.5 + 0.5`.

        The GBuffer and all buffers must have the same dimensions.

        The GBuffer array must have G3D::GLPixelTransferBuffers with exactly the following
        semantics and format:

        - 0: GBuffer::Field::WS_POSITION, ImageFormat::RGB32F()
        - 1: GBuffer::Field::WS_NORMAL, ImageFormat::RGB32F()
        - 2: GBuffer::Field::LAMBERTIAN, ImageFormat::RGB32F()
        - 3: GBuffer::Field::GLOSSY, ImageFormat::RGBA32F()
        - 4: GBuffer::Field::EMISSIVE, ImageFormat::RGB32F()

        WS_NORMAL is zero at pixels where the ray misses

        All other fields are ignored. 

        The base class implementation copies all data to the CPU, invokes the 
        intersectRays overload that accepts CPU data, and then copies all data
        back to the GPU.

        <b>This is the fastest overload for OptiXTriTree</b>

        Only supports the first two bits (0b11 = 3) of renderMask.
        Reports hits where (@a renderMask & surface->renderMask() & 3) != 0
     */
    virtual void intersectRays
        (const shared_ptr<GLPixelTransferBuffer>& rayOrigin,
         const shared_ptr<GLPixelTransferBuffer>& rayDirection,
         const shared_ptr<GLPixelTransferBuffer>  results[5],
         IntersectRayOptions                      options         = IntersectRayOptions(0),
         const shared_ptr<GLPixelTransferBuffer>& rayCone         = nullptr,
         const int baseMipLevel = 0,
		 const Vector2int32 wavefrontDimensions = Vector2int32(-1,-1),
		 const RenderMask renderMask = 0xFFFFFFFF) const = 0;

    /** \param booleanResults The red channel is nonzero on hit, 0 on miss. Subclasses are free to change the format of the 
        \a booleanResults texture to whatever is most convenient for them, so make no assumptions other than that it has
        a red channel. 
        */
    virtual void intersectRays
        (const shared_ptr<Texture>&         rayOrigin,
         const shared_ptr<Texture>&         rayDirection,
         const shared_ptr<Texture>&         booleanResults,
         IntersectRayOptions                options         = IntersectRayOptions(0)) const = 0;

    /** \param booleanResults The red channel is nonzero on hit, 0 on miss. Subclasses are free to change the format of the 
        \a booleanResults texture to whatever is most convenient for them, so make no assumptions other than that it has
        a red channel. 
        <b>This is the fastest overload for OptiXTriTree</b>
        */
    virtual void intersectRays
        (const shared_ptr<GLPixelTransferBuffer>& rayOrigin,
         const shared_ptr<GLPixelTransferBuffer>& rayDirection,
         const shared_ptr<GLPixelTransferBuffer>& booleanResults,
         IntersectRayOptions                options         = IntersectRayOptions(0)) const = 0;

    virtual void intersectRays
        (const Array<Ray>&                  rays,
         Array<bool>&                       results,
         IntersectRayOptions                options         = IntersectRayOptions(0)) const = 0;

    /** Returns all triangles that lie within the box. Default implementation
        tests each triangle in turn (linear time). */
    virtual void intersectBox
        (const AABox&                       box,
         Array<Tri>&                        results) const = 0;

    /** Returns all triangles that intersect or are contained within
        the sphere (technically, this is a ball intersection).

        Default implementation calls intersectBox and then filters the results for the sphere.
     */
    virtual void intersectSphere
        (const Sphere&                      sphere,
         Array<Tri>&                        triArray) const = 0;

    void sample(const Hit& hit, shared_ptr<Surfel>& surfel) const;

    /** Create an instance of whatever is the fastest implementation subclass for this machine.
        \param preferGPUData If true, use an implementation that is fast for ray buffers already on the GPU. */
    static shared_ptr<TriTree> create(bool preferGPUData = true, bool preferVulkan = false);

    static shared_ptr<TriTree> create(const shared_ptr<Scene>& scene, ImageStorage newImageStorage);

    /** Special single-ray CPU function for simplicity. This guarantees a hit...it will synthesize a skybox
        surfel on a miss if the TriTree was created from a Scene, or return a gray skybox surfel otherwise. 
        This is the absolute slowest way to use a TriTree. */
    shared_ptr<Surfel> intersectRay(const Ray& ray) const;
};

} // G3D
