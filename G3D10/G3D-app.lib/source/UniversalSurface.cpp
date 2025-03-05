/**
  \file G3D-app.lib/source/UniversalSurface.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/Log.h"
#include "G3D-base/constants.h"
#include "G3D-base/fileutils.h"
#include "G3D-app/UniversalSurface.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/CPUVertexArray.h"
#include "G3D-app/Tri.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-app/Light.h"
#include "G3D-app/ShadowMap.h"
#include "G3D-app/AmbientOcclusion.h"
#include "G3D-gfx/Shader.h"
#include "G3D-app/LightingEnvironment.h"
#include "G3D-app/SVO.h"
#include "G3D-base/AreaMemoryManager.h"

namespace G3D {

Array<shared_ptr<Texture>> UniversalSurface::GPUGeom::s_boneTextureBufferPool;
uint64 UniversalSurface::debugTrianglesSubmitted = 0;
float  UniversalSurface::debugSubmitFraction = 1.0f;

void UniversalSurface::setStorage(ImageStorage newStorage) {
    m_material->setStorage(newStorage);
}


TransparencyType UniversalSurface::transparencyType() const {
    if ((m_material->bsdf()->lambertian().max().a < 1.0f) && (m_material->alphaFilter() == AlphaFilter::BLEND)) {
        // Because the max alpha is less than one, this surface has no fully nontransparent texels
        return TransparencyType::ALL;
    } else if (hasTransmission()) {
        if (((m_material->alphaFilter() == AlphaFilter::ONE) || (m_material->bsdf()->lambertian().min().a == 1.0f)) &&                    
            (m_material->bsdf()->transmissive().min() == Color3::zero())) {
            // There may be some nontransparent pixel in here, because there is at least
            // one black pixel in the transmissive texture
            return TransparencyType::SOME;
        } else {
            return TransparencyType::ALL;
        }
    } else if ((m_material->alphaFilter() == AlphaFilter::ONE) || ! m_material->bsdf()->lambertian().nonUnitAlpha()) {
        return TransparencyType::NONE;
    } else if (m_material->alphaFilter() == AlphaFilter::BINARY) {
        return TransparencyType::BINARY;
    } else {
        // Fractional alpha
        debugAssertM(m_material->alphaFilter() != AlphaFilter::COVERAGE_MASK, "Unsupported");
        if (m_material->bsdf()->lambertian().max().a < 1.0f) {
            return TransparencyType::ALL;
        } else {
            return TransparencyType::SOME;
        }
    }
}


bool UniversalSurface::canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const {
    debugAssertM(m_material->alphaFilter() != AlphaFilter::DETECT, "AlphaFilter::DETECT must be resolved into ONE, BINARY, or BLEND when a material is created");

    const bool opaqueSamples = ((m_material->alphaFilter() == AlphaFilter::ONE) || 
        (m_material->alphaFilter() == AlphaFilter::BINARY) || 
        (m_material->alphaFilter() == AlphaFilter::COVERAGE_MASK) || 
        ! m_material->bsdf()->lambertian().nonUnitAlpha()) &&
        (!hasTransmission());

    const bool emissiveOk   = m_material->emissive().isBlack() || notNull(specification.encoding[GBuffer::Field::EMISSIVE].format);
    const bool lambertianOk = m_material->bsdf()->lambertian().isBlack() || notNull(specification.encoding[GBuffer::Field::LAMBERTIAN].format);
    const bool glossyOk     = m_material->bsdf()->glossy().isBlack() || notNull(specification.encoding[GBuffer::Field::GLOSSY].format);

    return opaqueSamples && emissiveOk && lambertianOk && glossyOk;
}


bool UniversalSurface::anyUnblended() const {
    // Transmissive if the largest color channel of the lowest values across the whole texture is nonzero
    const bool allTransmissive    = (m_material->bsdf()->transmissive().min().max() > 0.0f) && (! hasRefractiveTransmission());
    const bool allPartialCoverage = 
        (((m_material->alphaFilter() == AlphaFilter::BLEND) || (m_material->alphaFilter() == AlphaFilter::COVERAGE_MASK)) && (m_material->bsdf()->lambertian().max().a < 1.0f)) ||
        ((m_material->alphaFilter() == AlphaFilter::BINARY) && (m_material->bsdf()->lambertian().max().a < 0.5f));

    return ! (allTransmissive || allPartialCoverage);
}


void UniversalSurface::renderWireframeHomogeneous
(RenderDevice*                      rd, 
 const Array<shared_ptr<Surface> >& surfaceArray, 
 const Color4&                      color,
 bool                               previous) const {

    rd->pushState(); {

        rd->setDepthWrite(false);
        rd->setDepthTest(RenderDevice::DEPTH_LEQUAL);
        rd->setRenderMode(RenderDevice::RENDER_WIREFRAME);
        rd->setPolygonOffset(-0.5f);
        Args args;
        args.setUniform("color", color);
        args.setMacro("HAS_TEXTURE", false);
        for (int g = 0; g < surfaceArray.size(); ++g) {
            const shared_ptr<UniversalSurface>& surface = dynamic_pointer_cast<UniversalSurface>(surfaceArray[g]);
            debugAssertM(surface, "Surface::renderWireframeHomogeneous passed the wrong subclass");
            const shared_ptr<UniversalSurface::GPUGeom>& geom(surface->gpuGeom());
            
            if (geom->twoSided) {
                rd->setCullFace(CullFace::NONE);
            } else {
                rd->setCullFace(CullFace::BACK);
            }

            CFrame cframe;
            surface->getCoordinateFrame(cframe, previous);
            rd->setObjectToWorldMatrix(cframe);

            args.setAttributeArray("g3d_Vertex", geom->vertex);
            args.setIndexStream(geom->index);
            args.setPrimitiveType(geom->primitive);
            args.setUniform("gammaAdjust", 1.0);
            LAUNCH_SHADER_WITH_HINT("unlit.*", args, m_profilerHint);
        }

    } rd->popState();
}


void UniversalSurface::bindDepthPeelArgs
(Args& args, 
 RenderDevice* rd, 
 const shared_ptr<Texture>& depthPeelTexture, 
 const float minZSeparation) {

    const bool useDepthPeel = notNull(depthPeelTexture);
    args.setMacro("USE_DEPTH_PEEL", useDepthPeel ? 1 : 0);
    if (useDepthPeel) {
        const Vector3& clipInfo = Projection(rd->projectionMatrix(), rd->viewport().wh()).reconstructFromDepthClipInfo();
        args.setUniform("previousDepthBuffer", depthPeelTexture, Sampler::buffer());
        args.setUniform("minZSeparation",  minZSeparation);
        args.setUniform("currentToPreviousScale", Vector2(depthPeelTexture->width()  / rd->viewport().width(),
                                                          depthPeelTexture->height() / rd->viewport().height()));
        args.setUniform("clipInfo", clipInfo);
    }
}


struct DepthOnlyBatchDescriptor {
    uint32 glIndexBuffer;
    bool twoSided;
    bool hasBones;
    CFrame cframe;

    bool operator==(const DepthOnlyBatchDescriptor& other) const {
        return glIndexBuffer == other.glIndexBuffer && twoSided == other.twoSided && hasBones == other.hasBones && cframe == other.cframe;
    }

    bool operator!=(const DepthOnlyBatchDescriptor& other) const {
        return !operator==(other);
    }

    DepthOnlyBatchDescriptor() {}
    DepthOnlyBatchDescriptor(const shared_ptr<UniversalSurface>& surf) :
        glIndexBuffer(surf->gpuGeom()->index.buffer()->openGLVertexBufferObject()),
        twoSided(surf->gpuGeom()->twoSided),
        hasBones(surf->gpuGeom()->hasBones()),
        cframe(surf->frame()) {}
    
    static size_t hashCode(const DepthOnlyBatchDescriptor& d) {
        return d.glIndexBuffer + int(d.twoSided) + int(d.hasBones) + (*(int*)&d.cframe.translation.x) + (*(int*)&d.cframe.rotation[1][1]);
    }
};


static void categorizeByBatchDescriptor(const Array<shared_ptr<Surface> >& all, Array< Array<shared_ptr<Surface> > >& derivedArray) {
    derivedArray.fastClear();

    // Allocate space for the worst case, so that we don't have to copy arrays
    // all over the place during resizing.
    derivedArray.reserve(all.size());

    Table<DepthOnlyBatchDescriptor, int, DepthOnlyBatchDescriptor> descriptorToIndex;
    // Allocate the table elements in a memory area that can be cleared all at once
    // without invoking destructors.
    descriptorToIndex.clearAndSetMemoryManager(AreaMemoryManager::create(100 * 1024));

    for (int s = 0; s < all.size(); ++s) {
      const shared_ptr<Surface>& instance = all[s];
        
        bool created = false;
        int& index = descriptorToIndex.getCreate(DepthOnlyBatchDescriptor(dynamic_pointer_cast<UniversalSurface>(instance)), created);
        if (created) {
            // This is the first time that we've encountered this subclass.
            // Allocate the next element of subclassArray to hold it.
            index = derivedArray.size();
            derivedArray.next();
        }
        derivedArray[index].append(instance);
    }
}

void UniversalSurface::depthRenderHelper
   (RenderDevice*                      rd,
    Args&                              args, 
    const shared_ptr<UniversalSurface>&surface,
    const String&                      profilerHint,
    const shared_ptr<Texture>&         previousDepthBuffer,
    const float                        minZSeparation,
    const Color3&                      transmissionWeight,
    const shared_ptr<Shader>&          depthShader,
    const shared_ptr<Shader>&          depthPeelShader,
    const CullFace                     cull) {

    const shared_ptr<UniversalSurface::GPUGeom>& geom = surface->gpuGeom();

    if (geom->twoSided) {
        rd->setCullFace(CullFace::NONE);
    }

    // Needed for every type of pass 
    CFrame cframe;
    surface->getCoordinateFrame(cframe, false);
    rd->setObjectToWorldMatrix(cframe);

    surface->setShaderArgs(args);

    // Disable transparency for depth peeling?!
    args.setMacro("DISCARD_IF_FULL_COVERAGE", 0);
    args.setMacro("HAS_ALPHA", 0);
    args.setMacro("HAS_TRANSMISSIVE", 0);
    args.setMacro("USE_PARALLAX_MAPPING", 0);
    args.setUniform("transmissionWeight", transmissionWeight, true);

    // Don't use lightMaps for depth only
    args.setMacro("NUM_LIGHTMAP_DIRECTIONS", 0);
    args.setMacro("NUM_LIGHTS", 0);
    args.setMacro("USE_IMAGE_STORE", 0);

    bindDepthPeelArgs(args, rd, previousDepthBuffer, minZSeparation);
            
    // N.B. Alpha testing is handled explicitly inside the shader.
    if (notNull(previousDepthBuffer)) {
        LAUNCH_SHADER_PTR_WITH_HINT(depthPeelShader, args, profilerHint);
    } else {
        LAUNCH_SHADER_PTR_WITH_HINT(depthShader, args, profilerHint);
    }

    if (geom->twoSided) {
        rd->setCullFace(cull);
    }
}

void UniversalSurface::renderDepthOnlyHomogeneous
(RenderDevice*                      rd, 
 const Array<shared_ptr<Surface> >& surfaceArray,
 const shared_ptr<Texture>&         previousDepthBuffer,
 const float                        minZSeparation,
 TransparencyTestMode               transparencyTestMode,
 const Color3&                      transmissionWeight) const {
    debugAssertGLOk();

    static shared_ptr<Shader> depthNonOpaqueShader =
        Shader::fromFiles
        (System::findDataFile("UniversalSurface/UniversalSurface_depthOnly.vrt"),
         System::findDataFile("UniversalSurface/UniversalSurface_depthOnlyNonOpaque.pix"));

    static const shared_ptr<Shader> depthShader = 
        Shader::fromFiles(System::findDataFile("UniversalSurface/UniversalSurface_depthOnly.vrt")
#       ifdef G3D_OSX // OS X crashes if there isn't a shader bound for depth only rendering
            , System::findDataFile("UniversalSurface/UniversalSurface_depthOnly.pix")
#       endif
        );

    static const shared_ptr<Shader> depthPeelShader =
        Shader::fromFiles(System::findDataFile("UniversalSurface/UniversalSurface_depthOnly.vrt"),
                          System::findDataFile("UniversalSurface/UniversalSurface_depthPeel.pix"));

    rd->setColorWrite(false);
    const CullFace cull = rd->cullFace();

    static Array<shared_ptr<Surface> > opaqueSurfaces;
    static Array<shared_ptr<Surface> > transparentSurfaces;
    opaqueSurfaces.fastClear();
    transparentSurfaces.fastClear();

    for (int i = 0; i < surfaceArray.size(); ++i) {
        const shared_ptr<UniversalSurface>& surface = dynamic_pointer_cast<UniversalSurface>(surfaceArray[i]);
        debugAssertM(surface, "Surface::renderDepthOnlyHomogeneous passed the wrong subclass");
        if (surface->transparencyType() != TransparencyType::NONE) {
            debugAssertM(surface->material()->hasAlpha() || surface->material()->hasTransmissive(),
                "This model does not have any transparency properties but has transparencyType() != TransparencyType::NONE");
            transparentSurfaces.append(surface);
        } else {
            opaqueSurfaces.append(surface);
        }
    }

    // Batch rendering is an optimization that uses a single draw call for all surfaces
    // that share vertex buffers. This can potentially speed up rendering of models
    // with lots of substructure but no internal transformations.
    const bool batchRender = true;

    // Process opaque surfaces first, front-to-back to maximize early-z test performance
    if (batchRender) {
        static Array< Array<shared_ptr<Surface> > > batchTable;
        batchTable.fastClear();

        // Separate into batches that have the same cull face, bones, coordinate frame, and vertex buffer. This is useful
        // for depth rendering because materials have been stripped, so many surfaces can all be rendered in the same call.
        //
        // We could potentially batch surfaces with different coordinate frames together by binding an array of coordinate frames instead.
        categorizeByBatchDescriptor(opaqueSurfaces, batchTable);
        for (int b = batchTable.size() - 1; b >= 0; --b) {
            const Array<shared_ptr<Surface> >& batch = batchTable[b];
            const shared_ptr<UniversalSurface>& canonicalSurface = dynamic_pointer_cast<UniversalSurface>(batch[0]);

            Args args;
            for (int s = batch.size() - 1; s >= 0; --s) {
                args.appendIndexStream(dynamic_pointer_cast<UniversalSurface>(batch[s])->gpuGeom()->index);
            }

            depthRenderHelper(rd, args, canonicalSurface, format("batch%d (%s)", b, canonicalSurface->m_profilerHint.c_str()), previousDepthBuffer, minZSeparation, transmissionWeight, depthShader, depthPeelShader, cull);
        } // for each surface  

        batchTable.fastClear();
    } else {
        for (int g = opaqueSurfaces.size() - 1; g >= 0; --g) {
            const shared_ptr<UniversalSurface>& surface = dynamic_pointer_cast<UniversalSurface>(opaqueSurfaces[g]);
            debugAssert(surface->transparencyType() == TransparencyType::NONE);
            Args args;
            depthRenderHelper(rd, args, surface, surface->m_profilerHint, previousDepthBuffer, minZSeparation, transmissionWeight, depthShader, depthPeelShader, cull);
        } // for each surface  
    }

    // Now process surfaces with transparency 
    for (int g = 0; g < transparentSurfaces.size(); ++g) {
        const shared_ptr<UniversalSurface>& surface = dynamic_pointer_cast<UniversalSurface>(transparentSurfaces[g]);
        debugAssertM(surface->material()->hasAlpha() || surface->material()->hasTransmissive(),
            "This model does not have any transparency properties but was previously classified as transparent");

        debugAssertM(surface, "Surface::renderDepthOnlyHomogeneous passed the wrong subclass");
        debugAssert(surface->transparencyType() != TransparencyType::NONE);
        const shared_ptr<UniversalSurface::GPUGeom>& geom = surface->gpuGeom();
            
        if (geom->twoSided) {
            rd->setCullFace(CullFace::NONE);
        }
            
        // Needed for every type of pass 
        CFrame cframe;
        surface->getCoordinateFrame(cframe, false);
        rd->setObjectToWorldMatrix(cframe);
            
        Args args;

        surface->setShaderArgs(args, true);
        
        debugAssertM((args.macro("HAS_ALPHA") == "1") || (args.macro("HAS_TRANSMISSIVE") == "1"),
            "Did not set any transparency flag");

        bindDepthPeelArgs(args, rd, previousDepthBuffer, minZSeparation);
        args.setUniform("transmissionWeight", transmissionWeight);
        args.setMacro("DISCARD_IF_NO_TRANSPARENCY", transparencyTestMode == TransparencyTestMode::STOCHASTIC_REJECT_NONTRANSPARENT);

        const TransparencyType transparencyType = surface->transparencyType();

        args.setMacro("STOCHASTIC",
            (transparencyType != TransparencyType::BINARY) &&
            (transparencyTestMode != TransparencyTestMode::REJECT_TRANSPARENCY));


        // The depth with alpha shader handles the depth peel case internally
        LAUNCH_SHADER_PTR_WITH_HINT(depthNonOpaqueShader, args, surface->m_profilerHint);
     
        if (geom->twoSided) {
            rd->setCullFace(cull);
        }
    } // for each surface   

    opaqueSurfaces.fastClear();
    transparentSurfaces.fastClear();
}
    

void UniversalSurface::renderIntoGBufferHomogeneous
(RenderDevice*                      rd, 
 const Array<shared_ptr<Surface> >& surfaceArray,
 const shared_ptr<GBuffer>&         gbuffer,
 const shared_ptr<Texture>&         depthPeelTexture,
 const float                        minZSeparation,
 const LightingEnvironment&         lightingEnvironment) const {

    rd->pushState(); {
        const CullFace oldCullFace = rd->cullFace();

        // Render front-to-back for early-out Z
        for (int s = surfaceArray.size() - 1; s >= 0; --s) {
            const shared_ptr<UniversalSurface>& surface = dynamic_pointer_cast<UniversalSurface>(surfaceArray[s]);

            debugAssertM(notNull(surface), 
                         "Non UniversalSurface element of surfaceArray "
                         "in UniversalSurface::renderIntoGBufferHomogeneous");

            if (surface->hasRefractiveTransmission()) {
                // These surfaces can't appear in the G-buffer because they aren't shaded
                // until after the screen-space refraction texture has been captured.
                continue;
            }

            if (! surface->anyUnblended()) {
                continue;
            }

            const shared_ptr<GPUGeom>&           gpuGeom = surface->gpuGeom();
            const shared_ptr<UniversalMaterial>& material = surface->material();
            debugAssertM(notNull(material), "nullptr material on surface");
            (void)material;

            Args args;
            CFrame cframe;
            surface->getCoordinateFrame(cframe, false);
            rd->setObjectToWorldMatrix(cframe);

            CFrame previousFrame;
            surface->getCoordinateFrame(previousFrame, true);

            gbuffer->setShaderArgsWritePosition(previousFrame, rd, args);

            if (gpuGeom->twoSided) {
                rd->setCullFace(CullFace::NONE);
            }

            surface->setShaderArgs(args, true);

            args.setMacro("NUM_LIGHTS", 0);
            args.setMacro("USE_IMAGE_STORE", 0);

            const Rect2D& colorRect = gbuffer->colorRect();
            args.setUniform("lowerCoord", colorRect.x0y0());
            args.setUniform("upperCoord", colorRect.x1y1());
            
            bindDepthPeelArgs(args, rd, depthPeelTexture, minZSeparation);
            
            //  Debugging
            const int count = int(clamp(debugSubmitFraction, 0.0f, 1.0f) * (surface->m_gpuGeom->index.size() / 3));
            debugTrianglesSubmitted += count;            
            if (debugSubmitFraction < 1.0f) {
                args.setNumIndices(count * 3);
            }
            // N.B. Alpha testing is handled explicitly inside the shader.
            LAUNCH_SHADER_WITH_HINT("UniversalSurface/UniversalSurface_gbuffer.*", args, surface->m_profilerHint);


            if (gpuGeom->twoSided) {
                rd->setCullFace(oldCullFace);
            }
        }
    } rd->popState();
}



void UniversalSurface::renderIntoSVOHomogeneous
(RenderDevice*                  rd,
 Array<shared_ptr<Surface> >&   surfaceArray,
 const shared_ptr<SVO>&         svo,
 const CFrame&                  previousCameraFrame) const {
    /*
    static shared_ptr<Shader> s_svoGbufferShader;
    if (isNull(s_svoGbufferShader)) {
        s_svoGbufferShader = Shader::fromFiles
            (System::findDataFile("UniversalSurface/UniversalSurface_SVO.vrt"),
            System::findDataFile("UniversalSurface/UniversalSurface_SVO.geo"),
            System::findDataFile("UniversalSurface/UniversalSurface_SVO.pix"));
    }
    shared_ptr<Shader> shader = s_svoGbufferShader;
    */
    rd->pushState(); {

        rd->setColorWrite(false);
        rd->setAlphaWrite(false);
        rd->setDepthWrite(false);
        rd->setCullFace(CullFace::NONE);
        rd->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS);
        svo->setOrthogonalProjection(rd);

        glEnable(0x9346); //CONSERVATIVE_RASTERIZATION_NV

        for (int s = 0; s < surfaceArray.size(); ++s) {
            const shared_ptr<UniversalSurface>& surface = dynamic_pointer_cast<UniversalSurface>(surfaceArray[s]);
            debugAssertM(notNull(surface),
                "Non UniversalSurface element of surfaceArray "
                "in UniversalSurface::renderIntoSVOHomogeneous");

            const shared_ptr<GPUGeom>&           gpuGeom = surface->gpuGeom();
            const shared_ptr<UniversalMaterial>& material = surface->material();
            debugAssert(material);

            Args args;

            CFrame cframe;
            surface->getCoordinateFrame(cframe, false);
            rd->setObjectToWorldMatrix(cframe);

            args.setMacro("NUM_LIGHTS", 0);
            args.setMacro("HAS_ALPHA", 0);

            // Bind material arguments
            material->setShaderArgs(args, "material_");

            // Define FragmentBuffer GBuffer outputs
            //args.appendToPreamble(svo->writeDeclarationsFragmentBuffer());    //Done in bindWriteUniforms

            // Bind image, bias, scale arguments
            svo->bindWriteUniformsFragmentBuffer(args);


            // Bind geometry
            gpuGeom->setShaderArgs(args);

            for (int s = 0; s< svo->getNumSurfaceLayers(); s++){
                args.setUniform("curSurfaceOffset", -float(s)*1.0f / float(svo->fineVoxelResolution()));

                // N.B. Alpha testing is handled explicitly inside the shader.
                LAUNCH_SHADER_WITH_HINT("UniversalSurface/UniversalSurface_SVO.*", args, m_profilerHint);
                //rd->apply(shader, args);
            }

        } // for each surface

        glDisable(0x9346);

    } rd->popState();

}


void UniversalSurface::sortFrontToBack(Array<shared_ptr<UniversalSurface> >& a, const Vector3& v) {
    Array<shared_ptr<Surface> > s;
    s.resize(a.size());
    for (int i = 0; i < s.size(); ++i) {
        s[i] = a[i];
    }
    Surface::sortFrontToBack(s, v);
    for (int i = 0; i < s.size(); ++i) {
        a[i] = dynamic_pointer_cast<UniversalSurface>(s[i]);
    }
}


UniversalSurface::UniversalSurface
    (const String&                              name,
    const CoordinateFrame&                      frame,
    const CoordinateFrame&                      previousFrame,
    const shared_ptr<UniversalMaterial>&        material,
    const shared_ptr<GPUGeom>&                  gpuGeom,
    const CPUGeom&                              cpuGeom,
    const shared_ptr<ReferenceCountedObject>&   source,
    const ExpressiveLightScatteringProperties&  expressive,
    const shared_ptr<Model>&                    model,
    const shared_ptr<Entity>&                   entity,
    const shared_ptr<UniformTable>&             uniformTable,
    int                                         numInstances) :
    Surface(expressive),
    m_name(name),
    m_frame(frame),
    m_previousFrame(previousFrame),
    m_material(material),
    m_gpuGeom(gpuGeom),
    m_cpuGeom(cpuGeom),
    m_numInstances(numInstances),
    m_uniformTable(uniformTable),
    m_source(source) {
    m_model = model;
    m_entity = entity;
    m_profilerHint = (notNull(entity) ? entity->name() + "/" : "") + (notNull(model) ? model->name() + "/" : "") + m_name;

    m_isLight = notNull(entity) && dynamic_pointer_cast<Light>(entity);

    m_rigidBodyID = (int64)(entity.get()) ^
            (int64)(gpuGeom.get()) ^
            (int64)(material.get()) ^
            (int64)(cpuGeom.index) ^
            (((int64)(notNull(cpuGeom.index) ? (*cpuGeom.index)[0] : 0)) << 32);
}


shared_ptr<UniversalSurface> UniversalSurface::create
(const String&                              name,
 const CFrame&                              frame, 
 const CFrame&                              previousFrame,
 const shared_ptr<UniversalMaterial>&       material,
 const shared_ptr<GPUGeom>&                 gpuGeom,
 const CPUGeom&                             cpuGeom,
 const shared_ptr<ReferenceCountedObject>&  source,
 const ExpressiveLightScatteringProperties& expressive,
 const shared_ptr<Model>&                   model,
 const shared_ptr<Entity>&                  entity,
 const shared_ptr<UniformTable>&            uniformTable,
 int                                        numInstances) {
    debugAssertM(gpuGeom, "Passed a void GPUGeom pointer to UniversalSurface::create.");
    // Cannot check if the gpuGeom is valid because it might not be filled out yet
    return createShared<UniversalSurface>(name, frame, previousFrame, material, gpuGeom, cpuGeom, source, expressive, model, entity, uniformTable, numInstances);
}   


bool UniversalSurface::requiresBlending() const {
    return hasNonRefractiveTransmission() ||
        hasTransmission() ||
        (m_material->alphaFilter() == AlphaFilter::BLEND);
}


bool UniversalSurface::hasRefractiveTransmission() const {
    return hasTransmission() && (m_material->bsdf()->etaReflect() != m_material->bsdf()->etaTransmit());
}


bool UniversalSurface::hasNonRefractiveTransmission() const {
    return hasTransmission() && (m_material->bsdf()->etaReflect() == m_material->bsdf()->etaTransmit());
}


void UniversalSurface::setShaderArgs(Args& args, bool useStructFormat) const {
    m_gpuGeom->setShaderArgs(args);

    if (useStructFormat) {
        m_material->setShaderArgs(args, "material.");
        args.setMacro("INFER_AMBIENT_OCCLUSION_AT_TRANSPARENT_PIXELS", m_material->inferAmbientOcclusionAtTransparentPixels());
        args.setMacro("HAS_ALPHA",        m_material->hasAlpha());
        args.setMacro("HAS_TRANSMISSIVE", m_material->hasTransmissive());
        args.setMacro("HAS_EMISSIVE",     m_material->hasEmissive());
        args.setMacro("ALPHA_HINT",       m_material->alphaFilter());
    } else {
        m_material->setShaderArgs(args, "material_");
    }
    args.append(m_uniformTable);
    args.setNumInstances(m_numInstances);
}


void UniversalSurface::launchForwardShader(Args& args) const {
    if (false && m_gpuGeom->hasBones()) {
        LAUNCH_SHADER_WITH_HINT("UniversalSurface/UniversalSurface_boneWeights.*", args, m_profilerHint);
    } else {
        LAUNCH_SHADER_WITH_HINT("UniversalSurface/UniversalSurface_render.*", args, m_profilerHint);
    }
}


void UniversalSurface::modulateBackgroundByTransmission(RenderDevice* rd) const {
    if (! hasTransmission()) { return; }

    rd->pushState(); {
        // Modulate background by the transmission color
        Args args;

        args.setMacro("HAS_ALPHA",  m_material->hasAlpha());
        args.setMacro("ALPHA_HINT", m_material->alphaFilter());
        args.setMacro("HAS_TRANSMISSIVE", m_material->hasTransmissive());
        args.setMacro("HAS_EMISSIVE", false);

        // Don't use lightMaps
        args.setMacro("NUM_LIGHTMAP_DIRECTIONS", 0);

        m_gpuGeom->setShaderArgs(args);
        m_material->setShaderArgs(args, "material.");
        rd->setObjectToWorldMatrix(m_frame);
        rd->setBlendFunc(RenderDevice::BLEND_ZERO, RenderDevice::BLEND_SRC_COLOR);
        LAUNCH_SHADER_WITH_HINT("UniversalSurface/UniversalSurface_modulateBackground.*", args, m_profilerHint);

    } rd->popState();
}


void UniversalSurface::render
   (RenderDevice*                       rd,
    const LightingEnvironment&          environment,
    RenderPassType                      passType) const {

    const bool anyUnblendedPass = 
        (passType == RenderPassType::TRANSPARENT_AS_OPAQUE) ||
        (passType == RenderPassType::OPAQUE_SAMPLES) ||
        (passType == RenderPassType::UNBLENDED_SCREEN_SPACE_REFRACTION_SAMPLES);

    if ((passType != RenderPassType::TRANSPARENT_AS_OPAQUE) &&
        
        ((anyUnblendedPass && ! anyUnblended()) ||
        ((passType == RenderPassType::OPAQUE_SAMPLES) && hasRefractiveTransmission()) ||
        ((passType == RenderPassType::UNBLENDED_SCREEN_SPACE_REFRACTION_SAMPLES) && !hasRefractiveTransmission()) ||
        (! anyUnblendedPass && ! requiresBlending()))) {
        // Nothing to do in these cases
        return;
    }

    CFrame cframe;
    Sphere myBounds;
    getObjectSpaceBoundingSphere(myBounds);
    getCoordinateFrame(cframe, false);
    myBounds = cframe.toWorldSpace(myBounds);
    
    RenderDevice::BlendFunc srcBlend;
    RenderDevice::BlendFunc dstBlend;
    RenderDevice::BlendEq   blendEq;
    RenderDevice::BlendEq   ignoreEq;
    RenderDevice::BlendFunc ignoreFunc;
    rd->getBlendFunc(Framebuffer::COLOR0, srcBlend, dstBlend, blendEq, ignoreFunc, ignoreFunc, ignoreEq);

    const bool twoSided = m_gpuGeom->twoSided;
    const CullFace oldCullFace = rd->cullFace();

    LightingEnvironment reducedLighting(environment);

    // Remove lights that cannot affect this object
    for (int L = 0; L < reducedLighting.lightArray.size(); ++L) {
        if (! reducedLighting.lightArray[L]->possiblyIlluminates(myBounds)) {
            // This light does not affect this object
            reducedLighting.lightArray.fastRemove(L);
            --L;
        }
    }

    Args args;
    reducedLighting.setShaderArgs(args, "");
    // If making a transparent as opaque pass, we only want to discard where alpha = 0, not alpha < 0.5 or alpha < 1
    args.setMacro("UNBLENDED_PASS", anyUnblendedPass && (passType != RenderPassType::TRANSPARENT_AS_OPAQUE));

    rd->setObjectToWorldMatrix(m_frame);
    setShaderArgs(args, true);
    args.setUniform("depthBuffer", Texture::opaqueBlackIfNull(rd->framebuffer()->texture(Framebuffer::DEPTH)), Sampler::buffer());

    rd->setDepthWrite(anyUnblendedPass);
    rd->setDepthTest(RenderDevice::DEPTH_LESS);

    bindScreenSpaceTexture(args, reducedLighting, rd, environment.screenColorGuardBand(), environment.screenDepthGuardBand());

    if (hasRefractiveTransmission()) {
        args.setMacro("REFRACTION", 1);
    }

    // For non-partial coverage passes
    args.setUniform("clipInfo", Vector3(0, 0, 0));

    switch (passType) {
    case RenderPassType::OPAQUE_SAMPLES:
    case RenderPassType::UNBLENDED_SCREEN_SPACE_REFRACTION_SAMPLES:
    case RenderPassType::TRANSPARENT_AS_OPAQUE:
        rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);
        if (twoSided) { rd->setCullFace(CullFace::NONE); }
        launchForwardShader(args);
        break;

    case RenderPassType::MULTIPASS_BLENDED_SAMPLES:
        // The shader is configured for premultipled alpha
        if (hasTransmission()) {
            // The modulate background pass will darken the background appropriately
            rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ONE);
        } else {
            rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
            // For AO inference
            const Projection projection(rd->projectionMatrix(), rd->viewport().wh());
            args.setUniform("clipInfo", projection.reconstructFromDepthClipInfo());
        }
        if (twoSided) {
            rd->setCullFace(CullFace::FRONT);
            modulateBackgroundByTransmission(rd);
            launchForwardShader(args);

            rd->setCullFace(CullFace::BACK);
            modulateBackgroundByTransmission(rd);
            launchForwardShader(args);
        } else {
            rd->setCullFace(CullFace::BACK);
            modulateBackgroundByTransmission(rd);
            launchForwardShader(args);
        }
        break;

    case RenderPassType::SINGLE_PASS_UNORDERED_BLENDED_SAMPLES:
        if (m_gpuGeom->twoSided) { rd->setCullFace(CullFace::NONE); }
        if (! hasTransmission()) {
            // For AO inference
            const Projection projection(rd->projectionMatrix(), rd->viewport().wh());
            args.setUniform("clipInfo", projection.reconstructFromDepthClipInfo());
        }
        launchForwardShader(args);
        break;
       
    }
    
    // Restore old blend state
    rd->setBlendFunc(srcBlend, dstBlend, blendEq);
    rd->setCullFace(oldCullFace);
}


void UniversalSurface::bindScreenSpaceTexture
   (Args&                       args, 
    const LightingEnvironment&  lightingEnvironment, 
    RenderDevice*               rd, 
    const Vector2int16          colorGuardBandSize,
    const Vector2int16          depthGuardBandSize) const {

    const CFrame& cameraFrame = rd->cameraToWorldMatrix();

    Sphere sphere;
    getObjectSpaceBoundingSphere(sphere);
    const Sphere& bounds3D = m_frame.toWorldSpace(sphere);
    // const float distanceToCamera = (sphere.center + m_frame.translation - cameraFrame.translation).dot(cameraFrame.lookVector());
                    
    // Estimate of distance from object to background to
    // be constant (we could read back depth buffer, but
    // that won't produce frame coherence)

    const float zDistanceToHit = max(2.0f, bounds3D.radius);
    const float backZ = cameraFrame.pointToObjectSpace(bounds3D.center).z - zDistanceToHit;
    float backPlaneZ = min(-0.5f, backZ);
    args.setUniform("backgroundZ", backPlaneZ);
                    
    args.setUniform("etaRatio", m_material->bsdf()->etaReflect() / m_material->bsdf()->etaTransmit());
                    
    // Find out how big the back plane is in meters
    Projection P(rd->projectionMatrix(), rd->viewport().wh());
    P.setFarPlaneZ(backPlaneZ);
    Vector3 ur, ul, ll, lr;
    P.getFarViewportCorners(rd->viewport(), ur, ul, ll, lr);

    // Since we use the lengths only, do not bother taking to world space
    Vector2 backSizeMeters((ur - ul).length(), (ur - lr).length());


    const Vector2 trimBand = depthGuardBandSize - colorGuardBandSize;
    args.setUniform("backSizeMeters", backSizeMeters);
    args.setUniform("background", lightingEnvironment.screenColorTexture(), Sampler::video());
    args.setUniform("backgroundMinCoord", Vector2(trimBand + Vector2int16(1, 1)) / lightingEnvironment.screenColorTexture()->vector2Bounds());
    args.setUniform("backgroundMaxCoord", Vector2(1, 1) - Vector2(trimBand + Vector2int16(1, 1)) / lightingEnvironment.screenColorTexture()->vector2Bounds());
}

String UniversalSurface::name() const {
    return m_name;
}


bool UniversalSurface::hasTransmission() const {
    return m_material->bsdf()->transmissive().notBlack();
}


void UniversalSurface::getCoordinateFrame(CoordinateFrame& c, bool previous) const {
    if (previous) {
        c = m_previousFrame;
    } else {
        c = m_frame;
    }
}


void UniversalSurface::getObjectSpaceBoundingSphere(Sphere& s, bool previous) const {
    (void) previous;
    s = m_gpuGeom->sphereBounds;
}


void UniversalSurface::getObjectSpaceBoundingBox(AABox& b, bool previous) const {
    (void) previous;
    b = m_gpuGeom->boxBounds;
}


void UniversalSurface::getObjectSpaceGeometry
   (Array<int>&                  index, 
    Array<Point3>&               vertex, 
    Array<Vector3>&              normal, 
    Array<Vector4>&              packedTangent, 
    Array<Point2>&               texCoord, 
    bool                         previous) const {

    index.copyPOD(*(m_cpuGeom.index));
    //  If the CPUVertexArray is not null, then it supercedes the other data
    if (notNull(m_cpuGeom.vertexArray)) {
        const Array<CPUVertexArray::Vertex>& vertexArray = (m_cpuGeom.vertexArray)->vertex;
        const int offset = vertex.size();
        const int N = vertexArray.size();
        vertex.resize(offset + N);
        normal.resize(offset + N);
        packedTangent.resize(offset + N);
        texCoord.resize(offset + N);
        runConcurrently(0, N, [&](int i) {
            const CPUVertexArray::Vertex& vert = vertexArray[i];
            const int j = i + offset;
            vertex[j] = vert.position;
            normal[j] = vert.normal;
            packedTangent[j] = vert.tangent;
            texCoord[j] = vert.texCoord0;
        });
    } else {
        vertex.copyPOD((m_cpuGeom.geometry)->vertexArray);
        normal.copyPOD((m_cpuGeom.geometry)->normalArray);
        packedTangent.copyPOD(*(m_cpuGeom.packedTangent));
        texCoord.copyPOD(*(m_cpuGeom.texCoord0));
    }
}


void UniversalSurface::CPUGeom::copyVertexDataToGPU
(AttributeArray&               vertex, 
 AttributeArray&               normal, 
 AttributeArray&               packedTangentVAR, 
 AttributeArray&               texCoord0VAR, 
 AttributeArray&               texCoord1VAR, 
 AttributeArray&               vertexColorVAR,
 VertexBuffer::UsageHint       hint) {

    if (notNull(vertexArray)) {
        vertexArray->copyToGPU(vertex, normal, packedTangentVAR, texCoord0VAR, texCoord1VAR, vertexColorVAR, hint);
    } else {
        // Non-interleaved support
        texCoord1VAR = AttributeArray();
        vertexColorVAR = AttributeArray();

        size_t vtxSize = sizeof(Vector3) * geometry->vertexArray.size();
        size_t texSize = sizeof(Vector2) * texCoord0->size();
        size_t tanSize = sizeof(Vector4) * packedTangent->size();

        if ((vertex.maxSize() >= vtxSize) &&
            (normal.maxSize() >= vtxSize) &&
            ((tanSize == 0) || (packedTangentVAR.maxSize() >= tanSize)) &&
            ((texSize == 0) || (texCoord0VAR.maxSize() >= texSize))) {
            AttributeArray::updateInterleaved
               (geometry->vertexArray,  vertex,
                geometry->normalArray,  normal,
                *packedTangent,         packedTangentVAR,
                *texCoord0,             texCoord0VAR);

        } else {

            // Maximum round-up size of varArea.
            int roundOff = 16;

            // Allocate new VARs
            shared_ptr<VertexBuffer> varArea = VertexBuffer::create(vtxSize * 2 + texSize + tanSize + roundOff, hint);
            AttributeArray::createInterleaved
                (geometry->vertexArray, vertex,
                 geometry->normalArray, normal,
                 *packedTangent,        packedTangentVAR,
                 *texCoord0,            texCoord0VAR,
                 varArea);       
        }
    }
}



namespace _internal {

class IndexOffsetTableKey {
public:
    const CPUVertexArray* vertexArray;
    CFrame cFrame;
    IndexOffsetTableKey(const CPUVertexArray* vArray) : vertexArray(vArray){}
    static size_t hashCode(const IndexOffsetTableKey& key) {
        const size_t cframeHash = key.cFrame.rotation.row(0).hashCode() + key.cFrame.rotation.row(1).hashCode() + 
                                    key.cFrame.rotation.row(2).hashCode() + key.cFrame.translation.hashCode();
        return HashTrait<void*>::hashCode(key.vertexArray) + cframeHash;
    }
    static bool equals(const _internal::IndexOffsetTableKey& a, const _internal::IndexOffsetTableKey& b) {
        return a.vertexArray == b.vertexArray && a.cFrame == b.cFrame;
    }
};

}


void UniversalSurface::getTrisHomogeneous
(const Array<shared_ptr<Surface> >& surfaceArray, 
 CPUVertexArray&                    cpuVertexArray, 
 Array<Tri>&                        triArray,
 bool                               computePrevPosition) const{

    // Maps already seen surface-owned vertexArrays to the vertex index offset in the CPUVertexArray


    Table<const _internal::IndexOffsetTableKey, uint32, _internal::IndexOffsetTableKey, _internal::IndexOffsetTableKey> indexOffsetTable;

    const bool PREVIOUS = true;
    const bool CURRENT = false;

    for (int i = 0; i < surfaceArray.size(); ++i) {
            
        const shared_ptr<UniversalSurface>& surface = dynamic_pointer_cast<UniversalSurface>(surfaceArray[i]);
        alwaysAssertM(surface, "Non-UniversalSurface passed to UniversalSurface::getTrisHomogenous.");
 
        const UniversalSurface::CPUGeom&             cpuGeom   = surface->cpuGeom();
        const shared_ptr<UniversalSurface::GPUGeom>& gpuGeom   = surface->gpuGeom();
    
        bool twoSided = gpuGeom->twoSided;
        
        debugAssert(gpuGeom->primitive == PrimitiveType::TRIANGLES);
        debugAssert(notNull(cpuGeom.index));
    
        const Array<int>& index(*cpuGeom.index);
    
        _internal::IndexOffsetTableKey key(cpuGeom.vertexArray);
        // Object to world matrix.  Guaranteed to be an RT transformation,
        // so we can directly transform normals as if they were vectors.
        surface->getCoordinateFrame(key.cFrame, CURRENT);

        CFrame prevFrame;
        surface->getCoordinateFrame(prevFrame, PREVIOUS);

        const bool hasPartialCoverage = surface->material()->hasPartialCoverage();

        bool created = false;
        uint32& indexOffset = indexOffsetTable.getCreate(key, created);
        if (created) {
            indexOffset = cpuVertexArray.size();

            if (computePrevPosition) {
                cpuVertexArray.transformAndAppend(*(key.vertexArray), key.cFrame, prevFrame);
            } else {
                cpuVertexArray.transformAndAppend(*(key.vertexArray), key.cFrame);
            }
        } 

        alwaysAssertM(notNull(cpuGeom.vertexArray), "No support for non-interlaced vertex formats");

        for (int i = 0; i < index.size(); i += 3) {
            triArray.append
                (Tri(index[i + 0] + indexOffset,
                     index[i + 1] + indexOffset,
                     index[i + 2] + indexOffset,

                     cpuVertexArray,
                     surface,
                     twoSided,
                     hasPartialCoverage));

        } // for index
    } // for surface
}


void UniversalSurface::GPUGeom::setShaderArgs(Args& args) const {
    debugAssert(normal.valid());
    debugAssert(index.valid());

    args.setAttributeArray("g3d_Vertex", vertex);
    args.setAttributeArray("g3d_Normal", normal);
            
    if (texCoord0.valid() && (texCoord0.size() > 0)){
        args.setAttributeArray("g3d_TexCoord0", texCoord0);
    }

    if (texCoord1.valid() && (texCoord1.size() > 0)){
        args.setAttributeArray("g3d_TexCoord1", texCoord1);
    }

    if (vertexColor.valid() && (vertexColor.size() > 0)){
        args.setMacro("HAS_VERTEX_COLOR", true);
        args.setAttributeArray("g3d_VertexColor", vertexColor);
    } else {
        args.setMacro("HAS_VERTEX_COLOR", false);
    }

    if (packedTangent.valid() && (packedTangent.size() > 0)) {
        args.setAttributeArray("g3d_PackedTangent", packedTangent);
    }

    if (hasBones()) {
        args.setAttributeArray("g3d_BoneIndices", boneIndices);
        args.setAttributeArray("g3d_BoneWeights", boneWeights);
        args.setUniform("boneMatrixTexture", boneTexture, Sampler::buffer());
        args.setUniform("prevBoneMatrixTexture", prevBoneTexture, Sampler::buffer(), false);
        args.setMacro("HAS_BONES", 1);
    } else {
        args.setMacro("HAS_BONES", 0); 
    }
    args.setIndexStream(index);
    args.setPrimitiveType(primitive);
}

UniversalSurface::GPUGeom::~GPUGeom() {
    if (boneTexture.use_count() == 1) {
        // Reuse this texture
        s_boneTextureBufferPool.push_back(boneTexture);
    }
}


shared_ptr<Texture> UniversalSurface::GPUGeom::allocateBoneTexture(int width, int height) {
    // Checking for buffer pool
    for (int i = 0; i < s_boneTextureBufferPool.size(); ++i) {
        const shared_ptr<Texture>& t = s_boneTextureBufferPool[i];
        if ((t->width() == width) && (t->height() == height)) {
            // Copy the reference before changing the allocation of the bufferpool
            shared_ptr<Texture> texture = t;

            s_boneTextureBufferPool.fastRemove(i);
            return texture;
        }
    }

    // No texture found in the buffer pool. Allocate one (use 16-bit precision, which is 2x as fast as 32-bit)
    return Texture::createEmpty("G3D::UniversalSurface::GPUGeom::BoneTexture", width, height, ImageFormat::RGBA16F(), Texture::DIM_2D);
}


UniversalSurface::GPUGeom::GPUGeom(const shared_ptr<GPUGeom>& other) {
    alwaysAssertM(notNull(other), "Tried to construct GPUGeom from nullptr pointer");
    primitive       = other->primitive;
    index           = other->index;
    vertex          = other->vertex;
    normal          = other->normal;
    packedTangent   = other->packedTangent;
    texCoord0       = other->texCoord0;
    texCoord1       = other->texCoord1;
    vertexColor     = other->vertexColor;
    boneIndices     = other->boneIndices;
    boneWeights     = other->boneWeights;
    boneTexture     = other->boneTexture;
    prevBoneTexture = other->prevBoneTexture;
    twoSided        = other->twoSided;
    boxBounds       = other->boxBounds;
    sphereBounds    = other->sphereBounds;
}

} // G3D

