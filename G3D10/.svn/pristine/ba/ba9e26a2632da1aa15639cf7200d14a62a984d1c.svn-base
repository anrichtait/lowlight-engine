/**
  \file G3D-app.lib/source/TriTreeBase.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/AABox.h"
#include "G3D-base/CollisionDetection.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-app/TriTreeBase.h"
#include "G3D-app/Surface.h"
#include "G3D-app/UniversalSurfel.h"
#include "G3D-app/GBuffer.h"
#include "G3D-gfx/Profiler.h"
#include "G3D-app/Camera.h"
#include "G3D-app/Scene.h"
#include "G3D-gfx/RenderDevice.h"

namespace G3D {

void TriTreeBase::clear() {
    m_triArray.fastClear();
    m_vertexArray.clear();
}


TriTreeBase::~TriTreeBase() {
    clear();
}


void TriTreeBase::setContents
   (const shared_ptr<Scene>&            scene, 
    ImageStorage                        newStorage) {
    
    Array< shared_ptr<Surface> > surfaceArray;
    
    BEGIN_PROFILER_EVENT("Scene::onPose");
    scene->onPose(surfaceArray);
    END_PROFILER_EVENT();

    // As written, this is rebuilding the skybox when it doesn't have to.
    // I think it would need to be reenabled for an animated skybox.

    //if (newStorage == ImageStorage::COPY_TO_CPU || newStorage == ImageStorage::MOVE_TO_CPU) {
    //    if (scene->lastVisibleChangeTime() > m_lastBuildTime) {
    //        m_sky = scene->skyboxAsCubeMap();
    //    }
    //}
    //else {
    //    m_sky = nullptr;
    //}

    setContents(surfaceArray, newStorage);
}


void TriTreeBase::setContents
(const Array<shared_ptr<Surface> >& surfaceArray, 
 ImageStorage                       newStorage) {

    const bool computePrevPosition = false;
    clear();
    Surface::getTris(surfaceArray, m_vertexArray, m_triArray, computePrevPosition);
    Surface::setStorage(surfaceArray, newStorage);
    m_sky = nullptr;
    rebuild();
}


void TriTreeBase::setContents
   (const Array<Tri>&                  triArray, 
    const CPUVertexArray&              vertexArray,
    ImageStorage                       newStorage) {

    clear();
    m_triArray = triArray;
    m_vertexArray.copyFrom(vertexArray);
    Tri::setStorage(m_triArray, newStorage);
    m_sky = nullptr;
    rebuild();
}


void TriTreeBase::intersectRays
   (const Array<Ray>&      rays,
    Array<Hit>&            results,
    IntersectRayOptions    options) const {

    results.resize(rays.size());
    runConcurrently(0, rays.size(), [&](int i) { _intersectRay(rays[i], results[i], options); });
}


void TriTreeBase::intersectBox
   (const AABox&           box,
    Array<Tri>&            results) const {

    results.fastClear();
    Spinlock resultLock;
    runConcurrently(0, m_triArray.size(), [&](int t) {
        const Tri& tri = m_triArray[t];
        if ((tri.area() > 0.0f) &&
            CollisionDetection::fixedSolidBoxIntersectsFixedTriangle(box, Triangle(tri.position(m_vertexArray, 0), 
                                                                                   tri.position(m_vertexArray, 1), 
                                                                                   tri.position(m_vertexArray, 2)))) {
            resultLock.lock();
            results.append(tri);
            resultLock.unlock();
        }
    });
}


void TriTreeBase::intersectSphere
   (const Sphere&                      sphere,
    Array<Tri>&                        triArray) const {

    AABox box;
    sphere.getBounds(box);
    intersectBox(box, triArray);

    // Iterate backwards because we're removing
    for (int i = triArray.size() - 1; i >= 0; --i) {
        const Tri& tri = triArray[i];
        if (! CollisionDetection::fixedSolidSphereIntersectsFixedTriangle(sphere, Triangle(tri.position(m_vertexArray, 0), tri.position(m_vertexArray, 1), tri.position(m_vertexArray, 2)))) {
            triArray.fastRemove(i);
        }
    }
}


void TriTreeBase::intersectRays
    (const Array<Ray>&                 rays,
    Array<shared_ptr<Surfel>>&         results,
    IntersectRayOptions                options,
    const Array<float>&                coherence) const {

    Array<Hit> hits;
    results.resize(rays.size());
    intersectRays(rays, hits, options);

    const Hit* pHit = hits.getCArray();
    shared_ptr<Surfel>* pSurfel = results.getCArray();
    const Tri* pTri = m_triArray.getCArray();

    tbb::parallel_for(tbb::blocked_range<size_t>(0, hits.size(), 128), [&](const tbb::blocked_range<size_t>& r) {
        const size_t start = r.begin();
        const size_t end   = r.end();
        for (size_t i = start; i < end; ++i) {
            const Hit& hit = pHit[i];
            if (hit.triIndex != Hit::NONE) {
                // Pass twoSided to determine whether or not to flip normals.
                pTri[hit.triIndex].sample(hit.u, hit.v, hit.triIndex, m_vertexArray, hit.backface, pSurfel[i], pTri[hit.triIndex].twoSided());
            }
            else {
                pSurfel[i] = nullptr;
            }
        }
    });

}


void TriTreeBase::intersectRays
    (const Array<Ray>&                 rays,
    Array<bool>&                       results,
    IntersectRayOptions                options) const {

    Array<Hit> hits;
    results.resize(rays.size());
    intersectRays(rays, hits, options);
    runConcurrently(0, rays.size(), [&](int i) {
        results[i] = (hits[i].triIndex != Hit::NONE);
    });
}


void TriTreeBase::copyToCPU
   (const shared_ptr<GLPixelTransferBuffer>& srcPTB,
    Array<float>&                      rayCoherenceBuffer) {

    // Copy data to CPU
    const int N = srcPTB->width() * srcPTB->height();
    rayCoherenceBuffer.resize(N);

    // Map
    const float* srcPtr = reinterpret_cast<const float*>(srcPTB->mapRead());

    // Copy and interleave
    float* dstPtr = reinterpret_cast<float*>(rayCoherenceBuffer.getCArray());
    System::memcpy(dstPtr, srcPtr, sizeof(float) * N);

    // Unmap
    srcPTB->unmap();
    srcPtr = nullptr;
    dstPtr = nullptr;
}


void TriTreeBase::copyToCPU
   (const shared_ptr<GLPixelTransferBuffer>& originPTB,
    const shared_ptr<GLPixelTransferBuffer>& directionPTB,
    Array<Ray>&                        rayBuffer,
	const int width,
	const int height) {

    alwaysAssertM(notNull(originPTB) && (originPTB->format() == ImageFormat::RGBA32F()), "rayOrigin texture must be in RGBA32F format.");
    alwaysAssertM(notNull(directionPTB) && (directionPTB->format() == ImageFormat::RGBA32F()), "rayDirection texture must be in RGBA32F format.");
    alwaysAssertM(originPTB->width() == directionPTB->width() && originPTB->height() == directionPTB->height(), "rayOrigin and rayDirection must have the same shape");

	int N = 0;

	if (width == -1 || height == -1) {
		N = originPTB->width() * originPTB->height();
	}
	else {
		N = width * height;
	}

    rayBuffer.resize(N);

    // Map
    const Vector4* originPtr = reinterpret_cast<const Vector4*>(originPTB->mapRead());
    const Vector4* directionPtr = reinterpret_cast<const Vector4*>(directionPTB->mapRead());

    // Copy and interleave
    Vector4* rayPtr = reinterpret_cast<Vector4*>(rayBuffer.getCArray());
    runConcurrently(0, N, [&](int i) {
        const int j = i << 1; 
        rayPtr[j + 0] = originPtr[i];
        rayPtr[j + 1] = directionPtr[i];
    });

    // Unmap
    directionPTB->unmap();
    originPTB->unmap();
    originPtr = nullptr;
    directionPtr = nullptr;
}

    /*
static bool ignoreField(GBuffer::Field f) {
    return (f == GBuffer::Field::CS_POSITION_CHANGE) || (f == GBuffer::Field::SS_POSITION_CHANGE) || (f == GBuffer::Field::DEPTH_AND_STENCIL) ||
            (f == GBuffer::Field::SVO_POSITION) || (f == GBuffer::Field::FLAGS) || (f == GBuffer::Field::SVO_COVARIANCE_MAT1) || (f == GBuffer::Field::SVO_COVARIANCE_MAT2);
}
    */

void TriTreeBase::intersectRays
   (const shared_ptr<Texture>& rayOrigin,
    const shared_ptr<Texture>& rayDirection,
    const shared_ptr<GBuffer>& results,
    IntersectRayOptions        options,
    const shared_ptr<Texture>& rayCoherence) const {
    const shared_ptr<GLPixelTransferBuffer>& originPTB = rayOrigin->toPixelTransferBuffer();
    const shared_ptr<GLPixelTransferBuffer>& directionPTB = rayDirection->toPixelTransferBuffer();

    const shared_ptr<GLPixelTransferBuffer>& coherencePTB = rayCoherence ? rayCoherence->toPixelTransferBuffer() : nullptr;

    alwaysAssertM(notNull(results), "Result GBuffer must be preallocated");
    results->resize(rayOrigin->width(), rayOrigin->height());

    alwaysAssertM(notNull(results), "Result GBuffer must be preallocated");
    // Set up the G-buffer
    GBuffer::Specification spec;
    spec.encoding[GBuffer::Field::WS_POSITION] = ImageFormat::RGBA32F();
    spec.encoding[GBuffer::Field::WS_NORMAL]   = ImageFormat::RGBA32F();
    spec.encoding[GBuffer::Field::CS_NORMAL]   = nullptr;
    spec.encoding[GBuffer::Field::LAMBERTIAN]  = ImageFormat::RGBA32F();
    spec.encoding[GBuffer::Field::GLOSSY]      = ImageFormat::RGBA32F();
    spec.encoding[GBuffer::Field::EMISSIVE]    = ImageFormat::RGBA32F();

    // Removing the depth buffer forces the deferred shader to read the explicit position buffer
    //spec.encoding[GBuffer::Field::TRANSMISSIVE]= ImageFormat::RGBA32F();
    spec.encoding[GBuffer::Field::DEPTH_AND_STENCIL] = nullptr;
    results->setSpecification(spec);
    results->resize(rayOrigin->width(), rayOrigin->height());
    results->prepare(RenderDevice::current, results->camera(), results->timeOffset(), results->timeOffset(), results->depthGuardBandThickness(), results->colorGuardBandThickness());
    
    // Allocate PTBs for the transfer to the textures
    shared_ptr<GLPixelTransferBuffer> resultPTB[5];
    for (int i = 0; i < 5; ++i) {
        switch (i) {
        case 2:
        case 3:
            resultPTB[i] = GLPixelTransferBuffer::create(rayOrigin->width(), rayOrigin->height(), ImageFormat::RGBA8());
            break;
        default:
            resultPTB[i] = GLPixelTransferBuffer::create(rayOrigin->width(), rayOrigin->height(), ImageFormat::RGBA32F());
        }
    }
        
    intersectRays(originPTB, directionPTB, resultPTB, options, coherencePTB);

    // Map back to GL
    results->texture(GBuffer::Field::WS_POSITION)->update(resultPTB[0]);
    results->texture(GBuffer::Field::WS_NORMAL)->update(resultPTB[1]);
    results->texture(GBuffer::Field::LAMBERTIAN)->update(resultPTB[2]);
    results->texture(GBuffer::Field::GLOSSY)->update(resultPTB[3]);
    results->texture(GBuffer::Field::EMISSIVE)->update(resultPTB[4]);
}


void TriTreeBase::intersectRays
   (const shared_ptr<GLPixelTransferBuffer>& rayOrigin,
    const shared_ptr<GLPixelTransferBuffer>& rayDirection,
    const shared_ptr<GLPixelTransferBuffer>  results[5],
    IntersectRayOptions                      options,
    const shared_ptr<GLPixelTransferBuffer>& rayCone,
    const int baseMipLevel,
	const Vector2int32 wavefrontDimensions,
	const RenderMask mask) const {

    debugAssert(isNull(rayCone) || (rayCone->format()->numComponents == 1));
    
    // Copy data to CPU
    Array<Ray> rayBuffer;
    copyToCPU(rayOrigin, rayDirection, rayBuffer, wavefrontDimensions.x, wavefrontDimensions.y);

    Array<float> rayCoherenceBuffer;
    if (notNull(rayCone)) {
        copyToCPU(rayCone, rayCoherenceBuffer);
    }

    // Invoke CPU ray tracer
    Array<shared_ptr<Surfel>> surfelBuffer;
    intersectRays(rayBuffer, surfelBuffer, options, rayCoherenceBuffer);

    void* resultPtr[5];
    for (int i = 0; i < 5; ++i) {
        resultPtr[i] = results[i]->mapWrite();
    }

    // Upload
    // For each pixel
	runConcurrently(0, rayBuffer.size(), [&](int i) {
		const shared_ptr<Surfel>& s = surfelBuffer[i];
		if (isNull(s)) {
			((Vector4*)resultPtr[0])[i] = Vector4(0.0f, 0.0f, 0.0f, finf());
			((Vector4*)resultPtr[1])[i] = Vector4(0.0f, 0.0f, 0.0f, -1.0f);
			((Vector4*)resultPtr[4])[i] = Color4(0.0f, 0.0f, 0.0f, -1.0f);
		}
		else {
			const shared_ptr<UniversalSurfel>& surfel = dynamic_pointer_cast<UniversalSurfel>(surfelBuffer[i]);
			debugAssertM(notNull(surfel), "Unknown surfel type");

			((Vector4*)resultPtr[0])[i] = Vector4(surfel->position, 1.0f);
			((Vector4*)resultPtr[1])[i] = Vector4(surfel->shadingNormal, 0.0f);
			((Vector4uint8*)resultPtr[2])[i] = Vector4uint8(Vector4(255.0f * Color4(surfel->lambertianReflectivity, 1.0f)));
			((Vector4uint8*)resultPtr[3])[i] = Vector4uint8(Vector4(255.0f * Color4(surfel->glossyReflectionCoefficient, surfel->smoothness)));
			((Vector4*)resultPtr[4])[i] = Color4(surfel->emission, 1.0f);
		}
    });

    for (int i = 0; i < 5; ++i) {
        results[i]->unmap();
        resultPtr[i] = nullptr;
    }
}


void TriTreeBase::intersectRays
   (const shared_ptr<GLPixelTransferBuffer>& originPTB,
    const shared_ptr<GLPixelTransferBuffer>& directionPTB,
    const shared_ptr<GLPixelTransferBuffer>& hitPTB,
    IntersectRayOptions                options) const {

    // Copy data to CPU
    Array<Ray> rayBuffer;
    copyToCPU(originPTB, directionPTB, rayBuffer);

    // Invoke CPU ray tracer
    Array<bool> hitBuffer;
    intersectRays(rayBuffer, hitBuffer, options);

    uint8* hitPtr = reinterpret_cast<uint8*>(hitPTB->mapWrite());

    runConcurrently(0, hitBuffer.size(), [&](int i) {
        hitPtr[i] = hitBuffer[i] ? 255 : 0;
    });

    hitPTB->unmap();
    hitPtr = nullptr;
}


void TriTreeBase::intersectRays
   (const shared_ptr<Texture>&         rayOrigin,
    const shared_ptr<Texture>&         rayDirection,
    const shared_ptr<Texture>&         booleanResults,
    IntersectRayOptions                options) const {

    alwaysAssertM(notNull(booleanResults), "Result texture must be preallocated");
    const shared_ptr<GLPixelTransferBuffer>& originPTB = rayOrigin->toPixelTransferBuffer();
    const shared_ptr<GLPixelTransferBuffer>& directionPTB = rayDirection->toPixelTransferBuffer();
    const shared_ptr<GLPixelTransferBuffer>& hitPTB = GLPixelTransferBuffer::create(originPTB->width(), originPTB->height(), ImageFormat::R8());
    intersectRays(originPTB, directionPTB, hitPTB, options);
    booleanResults->update(hitPTB);
}

} // G3D
