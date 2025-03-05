/**
  \file G3D-app.lib/source/OptiXTriTree.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifdef _MSC_VER
#include "wave/wave.h"
#include "G3D-base/PixelTransferBuffer.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-app/UniversalMaterial.h"
#include "G3D-app/UniversalSurface.h"
#include "G3D-app/OptiXTriTree.h"
#include "G3D-app/UniversalSurfel.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Profiler.h"

namespace G3D {

OptiXTriTree::OptiXTriTree() {
    m_bvh = new wave::BVH();
}


OptiXTriTree::~OptiXTriTree() {
    delete m_bvh;
    m_bvh = nullptr;
}


bool OptiXTriTree::supported() {
    return m_bvh && m_bvh->valid();
}

void OptiXTriTree::setTimingCallback(wave::TimingCallback* callback, int verbosityLevel) {
    m_bvh->setTimingCallback(callback, verbosityLevel);
}


shared_ptr<Texture> OptiXTriTree::convertToOptiXFormat(const shared_ptr<Texture>& tex) {
    alwaysAssertM((tex->dimension() == Texture::DIM_2D) || (tex->dimension() == Texture::DIM_CUBE_MAP), "We currently only support 2D textures and cubemaps for interop with OptiX");

    // Cache of textures that had to be converted to another
    // opengl format before OptiX interop wrapping
    static Table<GLuint, shared_ptr<Texture>> formatConvertedTextureCache;

    if (! optixSupportsTexture(tex)) {
        // Convert from current format to RGBA32F
        bool needsConversion = false;
        shared_ptr<Texture>& converted = formatConvertedTextureCache.getCreate(tex->openGLID(), needsConversion);
        if (needsConversion) {
            if (tex->dimension() == Texture::DIM_2D) {
                const ImageFormat* fmt = tex->format();
                const ImageFormat* fmtAlpha = ImageFormat::getFormatWithAlpha(fmt);
                const ImageFormat* newFormat = nullptr;

                if ((tex->format()->code == ImageFormat::Code::CODE_SRGBA8) || (tex->format()->code == ImageFormat::Code::CODE_SRGB8)) {
                    // sRGB8 -> RGBA16F or sRGBA8 -> RGBA16F
                    newFormat = ImageFormat::RGBA8();
                } else if (fmtAlpha) {
                    // Add an alpha channel
                    newFormat = fmtAlpha;
                } else {
                    // Nothing worked, just use RGBA32F
                    newFormat = ImageFormat::RGBA32F();
                }
                converted = Texture::createEmpty("Converted0 " + tex->name(), tex->width(), tex->height(),
                    Texture::Encoding(newFormat, tex->encoding().frame, tex->encoding().readMultiplyFirst, tex->encoding().readAddSecond));

                Texture::copy(tex, converted);
            }
        }
        return converted;
    }
    else {
        return tex;
    }
}


void OptiXTriTree::setContents
   (const Array<shared_ptr<Surface> >& surfaceArray,
    ImageStorage                       newStorage) {

    // Assume all IDs we're tracking from previous frames are stale.
    // If we discover it isn't stale, remove it from the list of objects scheduled for deletion.
    // If we discover a new value, add it.
    m_sky = nullptr;

    // all elements were marked as dead in the previous pass
    for (const shared_ptr<Surface>& s : surfaceArray) {        
        const shared_ptr<UniversalSurface>& uniS = dynamic_pointer_cast<UniversalSurface>(s);
        if (isNull(uniS)) {
            // OptiXTriTree only supports UniversalSurface, but fails silently in other cases because the skybox is not one.
            continue;
        }

        const CFrame& frame = uniS->frame();
        const uint64 rigidBodyID = uniS->rigidBodyID();
        
        bool created = false;
        SurfaceCacheElement& e = m_surfaceCache.getCreate(rigidBodyID, created);
        // Mark as live
        e.live = true;

        if (! created) {
            // We've cached the surface, so just update the transform.
            if (uniS->lastChangeTime() > m_lastBuildTime) {
                m_bvh->setTransform(e.geometryIndex, static_cast<const float*>(frame.toMatrix4()));
            }
        } else {

            // Add the surface (and possibly material) to the cache.         
            //Array<int>                  index;
            //Array<Point3>               position;
            //Array<Vector3>              normal;
            //Array<Vector4>              tangentAndFacing;
            //Array<Point2>               texCoord;

            //// Although this call peforms a giant CPU copy, it is needed because the data is stored
            //// in each model interlaced and so is not in a usable format for OptiX
            //uniS->getObjectSpaceGeometry(index, position, normal, tangentAndFacing, texCoord);

            const Array<int>& index = *(uniS->cpuGeom().index);
            const CPUVertexArray* cpuVertexArray = uniS->cpuGeom().vertexArray;

            bool vertexArrayAdded = false;
            VertexCacheElement& vertices = m_vertexCache.getCreate(cpuVertexArray, vertexArrayAdded);
            if (vertexArrayAdded) {
                // Copy the data from the cpuGeom into the VertexCacheElement arrays
                for (const CPUVertexArray::Vertex& v : cpuVertexArray->vertex) {
                    vertices.position.append(v.position);
                    vertices.normal.append(v.normal);
                    vertices.tangent.append(v.tangent);
                    vertices.texCoord.append(v.texCoord0);
                }
            }

            // Assign materials
            const shared_ptr<UniversalMaterial>& material = uniS->material();

            if (notNull(material)) {
                ensureMaterialCached(material);
            } else {
                alwaysAssertM(false, "OptiXTriTree only accepts UniversalMaterial");
            }

            bool newFrameCreated = false;
            int transformID = m_frameCache.getCreate(frame, newFrameCreated);
            if (newFrameCreated) {
                // nextID is always the nextID that will be used, so postincrement
                transformID = m_nextFrameCacheID++;
                m_frameCache[frame] = transformID;
            }

            const Matrix4& M = frame.toMatrix4();

            GeometryIndex idx = m_bvh->createGeometry(
                reinterpret_cast<const float*>(vertices.position.getCArray()),
                reinterpret_cast<const float*>(vertices.normal.getCArray()),
                reinterpret_cast<const float*>(vertices.texCoord.getCArray()),
                reinterpret_cast<const float*>(vertices.tangent.getCArray()),
                (int)vertices.position.size(),
                &index[0],
                m_materialCache[material],
                index.size() / 3,
                static_cast<const float*>(M),
                (uniS->transparencyType() != TransparencyType::NONE) || uniS->gpuGeom()->twoSided,
                // If it can't change, group it under the transform associated
                // with identical CFrames
                uniS->canChange() ? m_nextFrameCacheID++ : transformID,
				uniS->renderMask());

            // Create the wave objects
            e.geometryIndex = idx;
            m_bvh->setTransform(idx, M);
        }
    }

    static Array<uint64> staleArray;
    staleArray.fastClear();
    for (SurfaceCache::Iterator e = m_surfaceCache.begin(); e.hasMore(); ++e) {
        if (! e->value.live) {
            staleArray.push(e->key);
        } else {
            // Reset for the next pass
            e->value.live = false;
        }
    }

    // Remove all stale geometry
    for (const uint64& rigidBodyId : staleArray) {
        SurfaceCacheElement e;
        if (m_surfaceCache.get(rigidBodyId, e)) {
            m_bvh->deleteGeometry(e.geometryIndex);
            m_surfaceCache.remove(rigidBodyId);
        } else {
            alwaysAssertM(false, "Stale ID not found in m_surfaceCache.");
        }
    }

    m_lastBuildTime = System::time();
}


void OptiXTriTree::ensureMaterialCached(const shared_ptr<UniversalMaterial>& material) {

    // Set the ignore texture, if necessary
    if (isNull(m_ignoreTexture)) {
        m_ignoreTexture = Texture::opaqueBlack();
    }
    
    if (m_materialCache.containsKey(material)) {
        return;
    }

    // Tri has a new material, so set the material in the wave::BVH and add it to the table.
    const shared_ptr<Texture>& bump         = notNull(material->bump())         ? convertToOptiXFormat(material->bump()->normalBumpMap()->texture()) : m_ignoreTexture;
    const shared_ptr<Texture>& lambertian   = material->bsdf()->hasLambertian() ? convertToOptiXFormat(material->bsdf()->lambertian().texture())     : m_ignoreTexture;
    const shared_ptr<Texture>& glossy       = material->bsdf()->hasGlossy()     ? convertToOptiXFormat(material->bsdf()->glossy().texture())        : m_ignoreTexture;
    const shared_ptr<Texture>& transmissive = material->hasTransmissive()       ? convertToOptiXFormat(material->bsdf()->transmissive().texture())   : m_ignoreTexture;
    const shared_ptr<Texture>& emissive     = material->hasEmissive()           ? convertToOptiXFormat(material->emissive().texture())               : m_ignoreTexture;

    bump->generateMipMaps();
    lambertian->generateMipMaps();
    glossy->generateMipMaps();
    transmissive->generateMipMaps();
    emissive->generateMipMaps();


    const MaterialIndex optixMaterial = m_bvh->createMaterial(
        material->hasAlpha(),

        bump->openGLID(),
        &bump->encoding().readMultiplyFirst[0],
        &bump->encoding().readAddSecond[0],

        lambertian->openGLID(),
        &lambertian->encoding().readMultiplyFirst[0],
        &lambertian->encoding().readAddSecond[0],

        glossy->openGLID(),
        &glossy->encoding().readMultiplyFirst[0],
        &glossy->encoding().readAddSecond[0],

        transmissive->openGLID(),
        &transmissive->encoding().readMultiplyFirst[0],
        &transmissive->encoding().readAddSecond[0],

        emissive->openGLID(),
        &emissive->encoding().readMultiplyFirst[0],
        &emissive->encoding().readAddSecond[0],

        (material->hasTransmissive()) ? material->bsdf()->etaReflect() / material->bsdf()->etaTransmit() : 1.0f, // etaPos / etaNeg

        material->flags()
    );
    m_materialCache.set(material, optixMaterial);
}

void OptiXTriTree::intersectRays
(const shared_ptr<GLPixelTransferBuffer>&         rayOrigins,
    const shared_ptr<GLPixelTransferBuffer>&         rayDirections,
    const shared_ptr<GLPixelTransferBuffer>&         booleanResults,
    IntersectRayOptions                options) const {

    registerReallocationAndMapHooks(rayOrigins);
    registerReallocationAndMapHooks(rayDirections);

    registerReallocationAndMapHooks(booleanResults);

    alwaysAssertM((options & 1), "Intersect rays with a single output buffer assumes occlusion cast");
    if (!m_bvh->occlusionCast(rayOrigins->glBufferID(), rayDirections->glBufferID(), rayOrigins->width(), rayOrigins->height(), booleanResults->glBufferID(), !(options & 8), !(options & 4), !(options & 2), RenderMask::ALL_GEOMETRY)) {
        alwaysAssertM(false, "Internal wave.lib exception printed to stderr. This is likely an out of memory exception due to copying too many textures into CUDA-acceptable formats.");
    }
}


void OptiXTriTree::copyToRayPBOs(const Array<Ray>& rays) const {
    const int width = 512; const int height = iCeil((float)rays.size() / (float)width);

    // Only reallocate the textures if we need more space
    if (isNull(m_rayOrigins) || (m_rayOrigins->width() * m_rayOrigins->height()) < (width * height)) {
        m_rayOrigins = GLPixelTransferBuffer::create(width, height, ImageFormat::RGBA32F());
        m_rayDirections = GLPixelTransferBuffer::create(width, height, ImageFormat::RGBA32F());
        registerReallocationAndMapHooks(m_rayOrigins);
        registerReallocationAndMapHooks(m_rayDirections);
    }

    // GL mandates that the ray textures and ptbs be of the same dimension. Both
    // may be larger than rays.size() if a previous call to intersect() generated
    // larger buffers.
    Vector4* originPointer    = (Vector4*)m_rayOrigins->mapWrite();
    Vector4* directionPointer = (Vector4*)m_rayDirections->mapWrite();

    // Exploit the fact that we know the exact memory layout of the Ray class,
    // which was designed to match this format.
    runConcurrently(0, rays.size(), [&](int i) {
        const Ray& ray = rays[i];
        originPointer[i]    = *(const Vector4*)(&ray);
        directionPointer[i] = *((const Vector4*)(&ray) + 1);
    });

    m_rayDirections->unmap();
    m_rayOrigins->unmap();
}


void OptiXTriTree::intersectRays
   (const Array<Ray>&                  rays,
    Array<bool>&                       results,
    IntersectRayOptions                options) const {

    alwaysAssertM((options & 1), "Intersect rays with a boolean array assumes occlusion casts");
    results.resize(rays.size());
    if (rays.size() == 0) {
        return;
    }


    copyToRayPBOs(rays);

    static shared_ptr<GLPixelTransferBuffer> hitsBuffer;
    
    if (isNull(hitsBuffer) || (hitsBuffer->width() != m_rayOrigins->width()) || (hitsBuffer->height() != m_rayOrigins->height())) {
        hitsBuffer = GLPixelTransferBuffer::create(m_rayOrigins->width(), m_rayOrigins->height(), ImageFormat::R8());
    }
        
    intersectRays(m_rayOrigins, m_rayDirections, hitsBuffer, options);

    uint8* hitsData = (uint8*)hitsBuffer->mapWrite();
    for (int i = 0; i < rays.size(); ++i) {
        results[i] = (*(hitsData + i) > 0);
    }
    hitsBuffer->unmap();
}


void OptiXTriTree::intersectRays
   (const shared_ptr<GLPixelTransferBuffer>&          rayOrigins,
    const shared_ptr<GLPixelTransferBuffer>&          rayDirections,
    Array<Hit>&                         results,
    IntersectRayOptions                 options) const {

    registerReallocationAndMapHooks(rayOrigins);
    registerReallocationAndMapHooks(rayDirections);

    const int width = rayOrigins->width();
    const int height = rayOrigins->height();
    alwaysAssertM(width > 0 && height > 0, "Width and height of ray textures must be greater than 0.");

    if (width > m_outWidth || height > m_outHeight) {
        m_outPBOArray.resize(9);
        //Re-allocate (or allocate) the PBOs as the dimensions have changed.
        for (int i = 0; i < 9; ++i) {
            m_outPBOArray[Field::MATERIAL0 + i] = (i == 4) ? GLPixelTransferBuffer::create(width, height, ImageFormat::RGBA32UI()) : GLPixelTransferBuffer::create(width, height, ImageFormat::RGBA32F());
            registerReallocationAndMapHooks(m_outPBOArray[Field::MATERIAL0 + i]);
        }
        m_outWidth = width;
        m_outHeight = height;
    }
   
    if (!m_bvh->cast(
        rayOrigins->glBufferID(),
        rayDirections->glBufferID(),
        width,
        height,
        m_outPBOArray[Field::MATERIAL0]->glBufferID(),
        m_outPBOArray[Field::MATERIAL1]->glBufferID(),
        m_outPBOArray[Field::MATERIAL2]->glBufferID(),
        m_outPBOArray[Field::MATERIAL3]->glBufferID(),

        m_outPBOArray[Field::HIT_LOCATION]->glBufferID(),
        m_outPBOArray[Field::SHADING_NORMAL]->glBufferID(),
        m_outPBOArray[Field::POSITION]->glBufferID(),
        m_outPBOArray[Field::GEOMETRIC_NORMAL]->glBufferID(),

        !(options & 8),
        !(options & 4),
        !(options & 2)
    )) {
        alwaysAssertM(false, "Internal wave.lib exception printed to stderr. This is likely an out of memory exception due to copying too many textures into CUDA-acceptable formats.");
    }
    
    // Map the output data
    const Vector4uint16* hitLocation = reinterpret_cast<const Vector4uint16*>(m_outPBOArray[Field::HIT_LOCATION]->mapRead());
    const Vector4* position             = reinterpret_cast<const Vector4*>(m_outPBOArray[Field::POSITION]->mapRead());

    // Fill hit array
    for (int i = 0; i < results.size(); ++i) {
        Hit& h = results[i];
        const Vector4uint16 hitData = hitLocation[i];
        h.triIndex = (int)hitData.x;
        h.u = (float)hitData.y;
        h.v = (float)hitData.z;
        h.backface = (bool)hitData.w;

        // A of positionOut stores hit distance
        h.distance = position[i].w;
    }

    m_outPBOArray[Field::HIT_LOCATION]->unmap(); hitLocation = nullptr;
    m_outPBOArray[Field::POSITION]->unmap(); position = nullptr;
}


void OptiXTriTree::intersectRays
   (const Array<Ray>&                  rays,
    Array<Hit>&                        results,
    IntersectRayOptions                options) const {
    results.resize(rays.size());
    if (rays.size() == 0) {
        return;
    }
    copyToRayPBOs(rays);

    intersectRays(m_rayOrigins, m_rayDirections, results, options);

}

void OptiXTriTree::intersectRays
(const shared_ptr<GLPixelTransferBuffer>&       rayOrigin,
    const shared_ptr<GLPixelTransferBuffer>&    rayDirection,
    const shared_ptr<GLPixelTransferBuffer>     results[5],
    IntersectRayOptions                         options,
    const shared_ptr<GLPixelTransferBuffer>&    rayCone,
    const int baseMipLevel,
	const Vector2int32 wavefrontDimensions,
	const RenderMask mask) const {

    BEGIN_PROFILER_EVENT("Pre-cast");
    registerReallocationAndMapHooks(rayOrigin);
    registerReallocationAndMapHooks(rayDirection);

    for (int i = 0; i < 5; ++i) {
        registerReallocationAndMapHooks(results[i]);
    }
    const int traceWidth = (wavefrontDimensions.x == -1) ? rayOrigin->width() : wavefrontDimensions.x;
    const int traceHeight = (wavefrontDimensions.y == -1) ? rayOrigin->height() : wavefrontDimensions.y;
    END_PROFILER_EVENT();

    BEGIN_PROFILER_EVENT("wave.lib: ray cast");
    if (!m_bvh->cast(
        rayOrigin->glBufferID(),
        rayDirection->glBufferID(),
        traceWidth,
        traceHeight,
        results[2]->glBufferID(), // Lambertian
        results[3]->glBufferID(), // Glossy
        GL_NONE,                  // Transmissive
        results[4]->glBufferID(), // Emissive

        GL_NONE,                  // Hit location
        results[1]->glBufferID(), // Shading Normal
        results[0]->glBufferID(), // Position
        GL_NONE,                  // Geometric normal 

        !(options & TriTree::NO_PARTIAL_COVERAGE_TEST),
        !(options & TriTree::DO_NOT_CULL_BACKFACES),
        baseMipLevel, // materialLOD
        notNull(rayCone) ? rayCone->glBufferID() : GL_NONE,
        mask
    )) {
        alwaysAssertM(false, "Internal wave.lib exception printed to stderr. This is likely an out of memory exception due to copying too many textures into CUDA-acceptable formats.");
    }
    END_PROFILER_EVENT();
}

void OptiXTriTree::intersectRays
   (const Array<Ray>&                  rays,
    Array<shared_ptr<Surfel>>&         results,
    IntersectRayOptions                options,
    const Array<float>&                coneBuffer) const {
    
    results.resize(rays.size());
    if (rays.size() == 0) {
        return;
    }

    copyToRayPBOs(rays);
    
    intersectRays(m_rayOrigins, m_rayDirections, results, options);

    // Trim off any padding added by generateRayTextures
    results.resize(rays.size());

    // Null out the miss surfels
    runConcurrently(0, results.size(), [&](int i) {
        shared_ptr<Surfel>& surfel = results[i];
        if (surfel->shadingNormal.squaredLength() < 0.5f) {
            surfel = nullptr;
        }
    });
}

void OptiXTriTree::intersectRays
   (const shared_ptr<GLPixelTransferBuffer>&         rayOrigins,
    const shared_ptr<GLPixelTransferBuffer>&         rayDirections,
    Array<shared_ptr<Surfel>>&         results,
    IntersectRayOptions                options) const {
    debugAssertGLOk();

    registerReallocationAndMapHooks(rayOrigins);
    registerReallocationAndMapHooks(rayDirections);

    // Set width and height based on actual number of rays.
    // There may be more space allocated in ray textures.
    const int width = rayOrigins->width();
    const int height = rayOrigins->height();

    //results.resize(width * height);

    if ((width > m_outWidth) || (height > m_outHeight)) {
        m_outPBOArray.resize(9);
        // Re-allocate (or allocate) the PBOs as the dimensions have changed.
        for (int i = 0; i < 9; ++i) {
            m_outPBOArray[Field::MATERIAL0 + i] = (i == 4) ? GLPixelTransferBuffer::create(width, height, ImageFormat::RGBA32UI()) : GLPixelTransferBuffer::create(width, height, ImageFormat::RGBA32F());
            registerReallocationAndMapHooks(m_outPBOArray[Field::MATERIAL0 + i]);
        }
        m_outWidth = width;
        m_outHeight = height;
    }

    debugAssertGLOk();

    if (!m_bvh->cast
    (rayOrigins->glBufferID(),
        rayDirections->glBufferID(),
        width,
        height,
        m_outPBOArray[Field::MATERIAL0]->glBufferID(),
        m_outPBOArray[Field::MATERIAL1]->glBufferID(),
        m_outPBOArray[Field::MATERIAL2]->glBufferID(),
        m_outPBOArray[Field::MATERIAL3]->glBufferID(),

        m_outPBOArray[Field::HIT_LOCATION]->glBufferID(),
        m_outPBOArray[Field::SHADING_NORMAL]->glBufferID(),
        m_outPBOArray[Field::POSITION]->glBufferID(),
        m_outPBOArray[Field::GEOMETRIC_NORMAL]->glBufferID(),

        !(options & TriTree::NO_PARTIAL_COVERAGE_TEST),
        !(options & TriTree::DO_NOT_CULL_BACKFACES)
    )) {
        alwaysAssertM(false, "Internal wave.lib exception printed to stderr. This is likely an out of memory exception due to copying too many textures into CUDA-acceptable formats.");
    }

    debugAssertGLOk();

    const Vector4* position =                   reinterpret_cast<const Vector4*>(m_outPBOArray[Field::POSITION]->mapRead());
    const Vector4* geometricNormal =            reinterpret_cast<const Vector4*>(m_outPBOArray[Field::GEOMETRIC_NORMAL]->mapRead());
    const Vector4* shadingNormal =              reinterpret_cast<const Vector4*>(m_outPBOArray[Field::SHADING_NORMAL]->mapRead());
    const Color4* lambertian =                  reinterpret_cast<const Color4*>(m_outPBOArray[Field::MATERIAL0]->mapRead());
    const Color4* glossy =                      reinterpret_cast<const Color4*>(m_outPBOArray[Field::MATERIAL1]->mapRead());
    const Color4* transmissive =                reinterpret_cast<const Color4*>(m_outPBOArray[Field::MATERIAL2]->mapRead());
    const Color4* emissive =                    reinterpret_cast<const Color4*>(m_outPBOArray[Field::MATERIAL3]->mapRead());

    // Copy the PBO data into surfels
    for (int i = 0; i < results.size(); ++i) {
        shared_ptr<UniversalSurfel> surfel = notNull(results[i]) ? dynamic_pointer_cast<UniversalSurfel>(results[i]) : UniversalSurfel::create();
        surfel->position = position[i].xyz();
        surfel->geometricNormal = geometricNormal[i].xyz();
        surfel->shadingNormal = shadingNormal[i].xyz();
        surfel->lambertianReflectivity = lambertian[i].rgb();
        surfel->glossyReflectionCoefficient = glossy[i].rgb();
        surfel->smoothness = glossy[i].a;
        surfel->transmissionCoefficient = transmissive[i].rgb();
        surfel->emission = emissive[i].rgb();
        surfel->flags = (uint8)(geometricNormal[i].w);
        surfel->etaRatio = emissive[i].a;
        results[i] = surfel;
    }

    m_outPBOArray[Field::POSITION]->unmap();            position = nullptr;
    m_outPBOArray[Field::GEOMETRIC_NORMAL]->unmap();    geometricNormal = nullptr;
    m_outPBOArray[Field::SHADING_NORMAL]->unmap();      shadingNormal = nullptr;
                  
    m_outPBOArray[Field::MATERIAL0]->unmap();           lambertian = nullptr;
    m_outPBOArray[Field::MATERIAL1]->unmap();           glossy = nullptr;
    m_outPBOArray[Field::MATERIAL2]->unmap();           transmissive = nullptr;
    m_outPBOArray[Field::MATERIAL3]->unmap();           emissive = nullptr;

}


bool OptiXTriTree::intersectRay
   (const Ray&                         ray,
    Hit&                               hit,
    IntersectRayOptions                options) const {
    
    Array<Ray> rays = { ray };
    Array<Hit> hits = { hit };

    copyToRayPBOs(rays);

    intersectRays(m_rayOrigins, m_rayDirections, hits, options);

    return hits[0].triIndex != Hit::NONE;
}

void OptiXTriTree::registerReallocationAndMapHooks(const shared_ptr<GLPixelTransferBuffer>& t) const {

    std::weak_ptr<GLPixelTransferBuffer> b;
    int id = t->glBufferID();
    if (m_registeredBufferIDs.get(id, b)) {
        if (!b.expired()) {
            return;
        }
    }
    m_registeredBufferIDs.set(id, std::weak_ptr<GLPixelTransferBuffer>(t));
    // Use weak_ptr to reference the OptiXTriTree without incrementing the reference count,  
    // allowing us to collect the tree before the registered textures if we ever need to.
    t->registerReallocationHook([tritree = std::weak_ptr<const OptiXTriTree>(dynamic_pointer_cast<const OptiXTriTree>(shared_from_this()))](GLint glID) {
        const shared_ptr<const OptiXTriTree>& ptr = tritree.lock();
        if (notNull(ptr)) { ptr->m_bvh->unregisterCachedBuffer(glID); };
    });

    t->registerMapHook([tritree = std::weak_ptr<const OptiXTriTree>(dynamic_pointer_cast<const OptiXTriTree>(shared_from_this()))](GLint glID) {
        const shared_ptr<const OptiXTriTree>& ptr = tritree.lock();
        if (notNull(ptr)) { ptr->m_bvh->unmapCachedBuffer(glID); };
    });

}
}
#endif
