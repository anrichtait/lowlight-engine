/**
  \file G3D-app.lib/source/DefaultRenderer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/fileutils.h"
#include "G3D-app/DefaultRenderer.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Framebuffer.h"
#include "G3D-app/LightingEnvironment.h"
#include "G3D-app/Camera.h"
#include "G3D-app/Surface.h"
#include "G3D-app/AmbientOcclusion.h"
#include "G3D-app/GApp.h"
#include "G3D-base/FileSystem.h"
#include <regex>

#include "G3D-app/VisibleEntity.h"
#include "G3D-app/OptiXTriTree.h"
#include "G3D-app/SkyboxSurface.h"

namespace G3D {

#define HIGH_PRECISION_OIT_FORMAT    ImageFormat::RGBA16F()
#define HIGH_PRECISION_OIT_FORMAT_RG ImageFormat::RG16F()

DefaultRenderer::DefaultRenderer(const String& className, const String& namespacePrefix) :
    m_shaderName(className),
    m_textureNamePrefix(namespacePrefix + className + "::"),
    m_deferredShading(false),
    m_orderIndependentTransparency(false),
    m_oitLowResDownsampleFactor(4), 
    m_oitUpsampleFilterRadius(2), 
    m_oitHighPrecision(true) {

    m_oitWriteDeclarationGLSLFilename = FilePath::canonicalize(System::findDataFile("shader/" + m_shaderName + "/" + m_shaderName + "_OIT_writePixel.glsl"));
}

void DefaultRenderer::getDynamicObjectBounds(const Array<shared_ptr<Entity>>& sceneEntities, Array<AABox>& dynamicBounds) {
    if (!(m_numInitializationFrames > 0)) {
        // If not already done, compute the set of dynamic objects.
            // Pass Array of bounding boxes to set the probe flags
        for (const shared_ptr<Entity>& e : sceneEntities) {
            const shared_ptr<VisibleEntity>& visible = dynamic_pointer_cast<VisibleEntity>(e);
            // Only pass changing surfaces.
            if (isNull(visible) || !visible->canChange()) {
                continue;
            }

            AABox bounds;
            visible->getLastBounds(bounds);
            // Do not add infinite bounding boxes.
            if (bounds != AABox::inf()) {
                dynamicBounds.push_back(bounds);
            }
        }
    }
}

void DefaultRenderer::convergeUninitializedProbes(RenderDevice* rd, const Array<shared_ptr<Surface>>& surfaceArray, const shared_ptr<TriTree>& tritree, const shared_ptr<GBuffer>& gbuffer, const LightingEnvironment& lightingEnv) {
    const bool glossy = m_enableGlossyGI;
    const bool diffuse = m_enableDiffuseGI;

    m_enableGlossyGI = false;
    m_enableDiffuseGI = true;

    debugAssertM(m_ddgiVolumeArray.size() > 0, "No DDGIVolumes");

    // At scene initialization, include all geometry for the first frame.
    // Many scenes do not mark their initial geometry static, which would
    // turn all probes off without including all geometry.
    bool allGeometry = m_numInitializationFrames == SCENE_INIT;

    // Converge for SCENE_INIT frames.
    while (initializationFrames() > 0) {
        for (const shared_ptr<DDGIVolume>& ddgiVolume : m_ddgiVolumeArray) {
            ddgiVolume->gatherTracingProbes({ DDGIVolume::ProbeStates::UNINITIALIZED });
        }
        traceAndUpdateProbes(rd, surfaceArray, tritree, gbuffer, lightingEnv, 512, (allGeometry) ? RenderMask::ALL_GEOMETRY : RenderMask::STATIC_GEOMETRY);
        --m_numInitializationFrames;
    }

    m_enableGlossyGI = glossy;
    m_enableDiffuseGI = diffuse;

    m_newlyXProbes = true;

}

void DefaultRenderer::traceGlossyIndirectIllumination
(RenderDevice* rd,
    const Array<shared_ptr<Surface> >& sortedVisibleSurfaceArray,
    const shared_ptr<GBuffer>& primaryGBuffer,
    const LightingEnvironment& environment,
    const shared_ptr<TriTree>& tritree) {

    const int reflectionWidth = primaryGBuffer->width();
    const int reflectionHeight = m_enableGlossyGI ? primaryGBuffer->height() : 0;

    resizeReflectionRayTextures(reflectionWidth, reflectionHeight);

    // 2. Generate glossy rays
    int numGlossyRays = 0;
    generateMirrorRays(rd, primaryGBuffer, numGlossyRays);

    BEGIN_PROFILER_EVENT("Glossy Ray Cast");
    sampleArbitraryRays(m_reflectionRayOriginsTexture, m_reflectionRayDirectionsTexture, tritree, m_reflectionGBuffer, numGlossyRays, RenderMask::ALL_GEOMETRY, glossyMipLevel());
    END_PROFILER_EVENT();

    if (m_glossyYScale > 1) {
        // If glossy GI was just enabled, initialize glossy GI resources.
        if (isNull(m_glossyGIFramebuffer)) {
			m_glossyGIFramebuffer = Framebuffer::create(Texture::createEmpty("DefaultRenderer::m_glossyGIFramebuffer", 1024, 1024, ImageFormat::R11G11B10F()));
			m_glossyGIFramebuffer->texture(0)->visualization.max = 4.0f;
        }
		if (isNull(m_shadedMirrorRaysFramebuffer)) {
			m_shadedMirrorRaysFramebuffer = Framebuffer::create(Texture::createEmpty("DefaultRenderer::m_shadedMirrorRaysFramebuffer", 1024, 1024, m_glossyGIFramebuffer->texture(0)->format(), Texture::DIM_2D, false));
		}

        m_shadedMirrorRaysFramebuffer->resize(m_reflectionRayOriginsTexture->width(), m_reflectionRayOriginsTexture->height());
    }

    BEGIN_PROFILER_EVENT("Glossy GI Shade");
    shadeArbitraryRays
    (rd,
        sortedVisibleSurfaceArray,
        (m_glossyYScale > 1) ? m_shadedMirrorRaysFramebuffer : m_blurredMIPReflectionFramebuffer,
        environment,
        m_reflectionRayOriginsTexture,
        m_reflectionRayDirectionsTexture,
        m_reflectionGBuffer,
        m_enableDiffuseGI,
        false);
    END_PROFILER_EVENT();

    BEGIN_PROFILER_EVENT("Glossy GI Filter");
    if (m_glossyYScale > 1) {

		if (isNull(m_blurredMIPReflectionFramebuffer)) {
			m_blurredMIPReflectionFramebuffer = Framebuffer::create(Texture::createEmpty("DefaultRenderer::m_blurredMIPReflectionFramebuffer", 1024, 1024, m_glossyGIFramebuffer->texture(0)->format(), Texture::DIM_2D, true));
		}
        // Stretch the result over the full-res buffer
        m_blurredMIPReflectionFramebuffer->resize(primaryGBuffer->width(), primaryGBuffer->height());
        rd->push2D(m_blurredMIPReflectionFramebuffer); {
            Args args;
            args.setUniform("srcTexture", m_shadedMirrorRaysFramebuffer->texture(0), Sampler::video());
            args.setRect(rd->viewport());
            LAUNCH_SHADER("DefaultRenderer/DefaultRenderer_upsampleReflections.pix", args);
        } rd->pop2D();
    }
    END_PROFILER_EVENT();
}

void DefaultRenderer::traceAndUpdateProbes
(RenderDevice* rd,
    const Array<shared_ptr<Surface>>& surfaceArray,
    const shared_ptr<TriTree>& tritree,
    const shared_ptr<GBuffer>& primaryGBuffer,
    const LightingEnvironment& environment,
    const int                              raysPerProbe,
    const unsigned int visibilityMask) {

    const bool initializing = m_numInitializationFrames > 0;

    BEGIN_PROFILER_EVENT("DefaultRenderer::updateDynamicGI");

    // Resize all buffers
    const int reflectionWidth = primaryGBuffer->width();
    const int reflectionHeight = m_enableGlossyGI ? primaryGBuffer->height() : 0;

    resizeReflectionRayTextures(reflectionWidth, reflectionHeight);
    resizeIrradianceRayTextures(raysPerProbe);

    debugAssertM(m_ddgiVolumeArray.size() > 0, "No DDGIVolumes on DefaultRenderer");

    // Generate diffuse (probe) rays
    int numDiffuseRays = 0;
    for (const shared_ptr<DDGIVolume>& ddgiVolume : m_ddgiVolumeArray) {
        int probeRays = (raysPerProbe == -1) ? ddgiVolume->specification().raysPerProbe : raysPerProbe;
        int numIrradianceRaysGenerated = 0;
        generateIrradianceRays(rd,
            numDiffuseRays,
            ddgiVolume,
            numIrradianceRaysGenerated,
            probeRays);
        numDiffuseRays += numIrradianceRaysGenerated;
    }

    BEGIN_PROFILER_EVENT("Unified Ray Cast");
    m_raysThisFrame = numDiffuseRays;
    // Irradiance
    sampleArbitraryRays(m_irradianceRayOriginsTexture, m_irradianceRayDirectionsTexture, tritree, m_irradianceRaysGBuffer, numDiffuseRays, visibilityMask, diffuseMipLevel());
    END_PROFILER_EVENT();

    // Shade glossy and diffuse rays using the probes.
    if (!initializing) {
        BEGIN_PROFILER_EVENT("Diffuse GI Shade Rays");
        // Irradiance
        shadeArbitraryRays
        (rd,
            surfaceArray,
            m_irradianceRaysShadedFB,
            environment,
            m_irradianceRayOriginsTexture,
            m_irradianceRayDirectionsTexture,
            m_irradianceRaysGBuffer,
            true,
            m_ddgiVolumeArray[0]->specification().glossyToMatte);
        END_PROFILER_EVENT();
    }

    // Update diffuse irradiance probes.
    BEGIN_PROFILER_EVENT("Update Probes");
    int irradianceRayOffset = 0;
    for (const shared_ptr<DDGIVolume>& ddgiVolume : m_ddgiVolumeArray) {
        int numRays = (raysPerProbe == -1) ? ddgiVolume->specification().raysPerProbe : raysPerProbe;
        if (ddgiVolume->specification().enableProbeUpdate) {
            ddgiVolume->updateAllProbeTypes(rd,
                surfaceArray,
                m_irradianceRaysGBuffer->texture(GBuffer::Field::WS_POSITION),
                m_irradianceRaysShadedFB->texture(0),
                irradianceRayOffset,
                numRays);
        }
        // Update UNINITIALIZED to starting state or update NewlyX -> X.
        if (initializing) {
            ddgiVolume->computeProbeOffsetsAndFlags(rd,
                m_irradianceRaysGBuffer->texture(GBuffer::Field::WS_POSITION),
                irradianceRayOffset,
                raysPerProbe,
                (m_numInitializationFrames > 1));

        }

        irradianceRayOffset += (ddgiVolume->probeCount() - ddgiVolume->skippedProbes()) * numRays;
    }
    if (m_shouldClearUninitializedProbes) {
        m_shouldClearUninitializedProbes = false;
    }
    END_PROFILER_EVENT();
    END_PROFILER_EVENT();
}

void DefaultRenderer::updateDiffuseGI(RenderDevice* rd,
    const shared_ptr<Scene>& scene,
    const shared_ptr<GBuffer>& primaryGBuffer,
    const shared_ptr<Camera>& camera) {

    if (!m_enableDiffuseGI) {
        return;
    }

    LightingEnvironment& environment = scene->lightingEnvironment();

    // Get scene bounds.
    AABox sceneBounds;
    scene->getVisibleBounds(sceneBounds);

    // Check if we need to (re)create the probe volumes.
    if (m_ddgiVolumeArray.size() == 0 ||
        m_bounds != sceneBounds ||
        m_ddgiVolumeArray.size() != environment.ddgiVolumeArray.size()) {
        createProbeVolumes(sceneBounds, environment, camera);
    }

    // Pose surfaces
    Array<shared_ptr<Surface>> surfaceArray;
    scene->onPose(surfaceArray);

    bool glossy = m_enableGlossyGI;
    m_enableGlossyGI = false;

    // Notify that the camera has changed.
    // If the camera has moved out of the center zone, set the row of probes that moved to uninitialized.
    // Reset the offsets for those probes to 0.
    BEGIN_PROFILER_EVENT("Notify Of Camera Position");
    const Point3& cameraWSPosition = notNull(camera) ? camera->frame().translation : primaryGBuffer->camera()->frame().translation;
    for (const shared_ptr<DDGIVolume>& ddgiVolume : m_ddgiVolumeArray) {

        // If a volume spawns camera locked, we need this on the first frame.
        ddgiVolume->resizeIfNeeded();

        if (ddgiVolume->notifyOfCameraPosition(cameraWSPosition)) {
            addVolumeInitializationFrames(CAMERA_TRACK);
        }
    }
    END_PROFILER_EVENT();

    const shared_ptr<TriTree>& tritree = scene->tritree();
    // If there are uninitialized probes, run the position optimizer.
    // This state will be set externally by either the previous call notifying of 
    // camera position or the app on scene reload.
    BEGIN_PROFILER_EVENT("Classify Uninitialized Probes");
    if (m_numInitializationFrames > 0) {
        convergeUninitializedProbes(rd, surfaceArray, tritree, primaryGBuffer, environment);
    }
    END_PROFILER_EVENT();

    BEGIN_PROFILER_EVENT("Notify Of Dynamic Objects");


    // Get bounds on dynamic objects.
    Array<shared_ptr<Entity>> sceneEntities;
    scene->getEntityArray(sceneEntities);
    Array<AABox> dymamicBounds;
    getDynamicObjectBounds(sceneEntities, dymamicBounds);

    for (const shared_ptr<DDGIVolume>& ddgiVolume : m_ddgiVolumeArray) {
        // Notify the DDGIVolume of any bounds that changed.
        ddgiVolume->notifyOfDynamicObjects(dymamicBounds, Array<Vector3>());
        ddgiVolume->gatherTracingProbes({ DDGIVolume::ProbeStates::JUST_WOKE, DDGIVolume::ProbeStates::JUST_VIGILANT });
    }
    END_PROFILER_EVENT();

    BEGIN_PROFILER_EVENT("Converge NewlyX Probes");

    bool shouldTrace = false;
    for (const shared_ptr<DDGIVolume>& ddgiVolume : m_ddgiVolumeArray) {
        shouldTrace = shouldTrace || ddgiVolume->hasTracingProbes();
    }

    // Update probe states NewlyX->X.
    if (shouldTrace) {
        traceAndUpdateProbes(rd, surfaceArray, tritree, primaryGBuffer, environment, 1024, RenderMask::ALL_GEOMETRY);
    }
    END_PROFILER_EVENT();

    BEGIN_PROFILER_EVENT("Trace And Update Probes");

    BEGIN_PROFILER_EVENT("Gather Tracing Probes");
    for (const shared_ptr<DDGIVolume>& ddgiVolume : m_ddgiVolumeArray) {
        ddgiVolume->gatherTracingProbes({ DDGIVolume::ProbeStates::AWAKE, DDGIVolume::ProbeStates::VIGILANT });
    }
    END_PROFILER_EVENT();

    m_enableGlossyGI = glossy;
    traceAndUpdateProbes(rd, surfaceArray, tritree, primaryGBuffer, environment, m_ddgiVolumeArray[0]->specification().raysPerProbe, RenderMask::ALL_GEOMETRY);
    END_PROFILER_EVENT();
}

void DefaultRenderer::createProbeVolumes(const AABox& sceneBounds, LightingEnvironment& environment, const shared_ptr<Camera>& camera) {
    m_ddgiVolumeArray.fastClear();
    m_showProbeLocations.fastClear();

    // Compute good default bounds
    AABox defaultCircumscribedBounds;
    AABox defaultInscribedBounds;
    {
        AABox startingSceneBounds = sceneBounds;
        // The viewer doesn't load geo, so initialize a zero volume
        // AABB in this case.
        if (startingSceneBounds.low().isNaN()) {
            startingSceneBounds = AABox(Point3(0, 0, 0), Point3(0, 0, 0));
        }

        const Vector3 circumscribeDims = startingSceneBounds.extent() * 1.05f;
        defaultCircumscribedBounds =
            AABox(startingSceneBounds.center() - circumscribeDims * Vector3(0.5f, 0.5f, 0.5f),
                startingSceneBounds.center() + circumscribeDims * Vector3(0.5f, 0.5f, 0.5f));

        const Vector3 inscribeDims = startingSceneBounds.extent() * Vector3(0.9f, 0.86f, 0.9f);
        defaultInscribedBounds =
            AABox(startingSceneBounds.center() - inscribeDims * Vector3(0.5f, 0.5f, 0.5f),
                startingSceneBounds.center() + inscribeDims * Vector3(0.5f, 0.5f, 0.5f));
    }

   
    Table<String, DDGIVolumeSpecification> specTable;
    for (int i = 0; i < environment.ddgiVolumeSpecificationArray.size(); ++i) {
        if (environment.ddgiVolumeSpecificationArray[i].bounds.volume() == 0.0f) {
			// Bounds weren't set in the spec, so pass the computed bounds.
            specTable.set(format("DDGIVolume%d", i), DDGIVolumeSpecification(environment.ddgiVolumeSpecificationArray[i].toAny(), defaultInscribedBounds, defaultCircumscribedBounds));
        }
        else {
            specTable.set(format("DDGIVolume%d", i), environment.ddgiVolumeSpecificationArray[i]);
        }
    }

    if (specTable.size() == 0) {
        // Create a default specification
        specTable.set("DDGIVolume", DDGIVolumeSpecification(Any(Any::TABLE, "DDGIVolumeSpecification"), defaultInscribedBounds, defaultCircumscribedBounds));
    }

    // Instantiate the volumes
    for (Table<String, DDGIVolumeSpecification>::Iterator it = specTable.begin(); it.hasMore(); ++it) {
        m_ddgiVolumeArray.append(DDGIVolume::create(it->key, it->value, camera->frame().translation));
        m_showProbeLocations.append(false);
    }

    // Sort the volumes in *increasing* size, so that we prefer smaller volumes
    m_ddgiVolumeArray.sort([](const shared_ptr<DDGIVolume>& a, const shared_ptr<DDGIVolume>& b) {
        return a->specification().bounds.volume() < b->specification().bounds.volume();
        });

    // There are usually 1-3 probe volumes, so we don't mind this copy.
    environment.ddgiVolumeArray = m_ddgiVolumeArray;

    // Force creation of the volume textures.
    for (const shared_ptr<DDGIVolume>& volume : m_ddgiVolumeArray) {
        volume->resizeIfNeeded();
    }

    addVolumeInitializationFrames(SCENE_INIT);
}

void DefaultRenderer::generateMirrorRays
(RenderDevice* rd,
    const shared_ptr<GBuffer>& primaryGBuffer,
    int& numRays) {

    const int width = primaryGBuffer->width();
    const int height = iCeil(float(primaryGBuffer->height()) / float(m_glossyYScale));

	if (isNull(m_reflectionGBuffer)) {
		GBuffer::Specification gbufferSpec;
		gbufferSpec.encoding[GBuffer::Field::LAMBERTIAN].format = ImageFormat::RGBA8();
		gbufferSpec.encoding[GBuffer::Field::GLOSSY].format = ImageFormat::RGBA8();
		gbufferSpec.encoding[GBuffer::Field::EMISSIVE].format = ImageFormat::RGBA32F();
		gbufferSpec.encoding[GBuffer::Field::WS_POSITION].format = ImageFormat::RGBA32F();
		gbufferSpec.encoding[GBuffer::Field::WS_NORMAL] = Texture::Encoding(ImageFormat::RGBA32F(), FrameName::CAMERA, 1.0f, 0.0f);
		gbufferSpec.encoding[GBuffer::Field::DEPTH_AND_STENCIL].format = nullptr;
		gbufferSpec.encoding[GBuffer::Field::CS_NORMAL] = nullptr;
		gbufferSpec.encoding[GBuffer::Field::CS_POSITION] = nullptr;

		m_reflectionGBuffer = GBuffer::create(gbufferSpec, "DefaultRenderer::m_reflectionGBuffer");
	}

    m_reflectionGBuffer->resize(width, height);

    Args args;

    args.setImageUniform("rayOriginImage", m_reflectionRayOriginsTexture, Access::WRITE);
    args.setImageUniform("rayDirectionImage", m_reflectionRayDirectionsTexture, Access::WRITE);
    
    const Rect2D viewport = Rect2D::xywh(0, 0, (float)width, (float)height);
    args.setRect(viewport);
    numRays = width * height;
    args.setUniform("numRays", numRays);
    args.computeGridDim = Vector3int32(iCeil(width / 16.0f), iCeil(height / 16.0f), 1);
    args.setUniform("scale", Vector2int32(1, m_glossyYScale));
    primaryGBuffer->setShaderArgsRead(args, "gbuffer_");

    LAUNCH_SHADER("DefaultRenderer_generateMirrorRays.glc", args);
}

void DefaultRenderer::generateIrradianceRays(RenderDevice* rd, const int offset, const shared_ptr<DDGIVolume>& ddgiVolume, int& numGeneratedRays, const int raysPerProbe) {
    BEGIN_PROFILER_EVENT("generateIrradianceRays");

    const int width = raysPerProbe,
        height = ddgiVolume->probeCount();

    // Didn't generate rays for these probes.
    numGeneratedRays = width * (height - ddgiVolume->skippedProbes());

    // Return early if we skipped all probes in this step.
    // This can happen if we are still initiaizing but all
    // probes in the volume already have their starting value.
    if (numGeneratedRays == 0) {
        END_PROFILER_EVENT();
        return;
    }

    Args args;

    args.setImageUniform("rayOriginImage", m_irradianceRayOriginsTexture, Access::WRITE);
    args.setImageUniform("rayDirectionImage", m_irradianceRayDirectionsTexture, Access::WRITE);

    ddgiVolume->rayBlockIndexOffsetBuffer()->bindAsShaderStorageBuffer(2);

    ddgiVolume->setShaderArgs(args, "ddgiVolume.");
    args.setMacro("RAYS_PER_PROBE", raysPerProbe);

    args.setUniform("offset", offset);
    args.setUniform("totalRays", raysPerProbe * ddgiVolume->probeCount());
    args.setUniform("gridSize", Vector2int32(width, height));
    args.setUniform("textureWidth", m_irradianceRayOriginsTexture->width());

    // Set the random orientation on the DDGIVolume.
    const Matrix3& randomOrientation = 
        m_enableProbeRayRandomRotation ? 
        Matrix3::fromAxisAngle(Vector3::random(), Random::common().uniform(0.f, 2 * pif())) :
        Matrix3::identity();
    ddgiVolume->setRandomOrientation(randomOrientation);
    args.setUniform("randomOrientation", randomOrientation);

    // Rays per probe should always be divisible by 32.
    assert(width % 32 == 0);
    args.setComputeGridDim(Vector3int32(width / 32, height, 1));
    args.setComputeGroupSize(Vector3int32(32, 1, 1));

    LAUNCH_SHADER("DDGIVolume_generateRays.glc", args);

    debugAssertGLOk();
    END_PROFILER_EVENT();
}

void DefaultRenderer::resizeIrradianceRayTextures(const int raysPerProbe) {
    BEGIN_PROFILER_EVENT("resizeIrradianceRayTextures");
    // Irradiance ray textures
    // Note that these are ray textures (and gbuffer) for *all* DDGIVolumes being
    // traced this frame.
    int rayDimX = 0, rayDimY = 0;

    for (const shared_ptr<DDGIVolume>& ddgiVolume : m_ddgiVolumeArray) {

        int probeRays = (raysPerProbe == -1) ? ddgiVolume->specification().raysPerProbe : raysPerProbe;
        // w = max number of rays per probe traced by any volume.
        rayDimX = max(probeRays, rayDimX);
        // h = sum of all probes in each volume, one probe per row (lots of empty space until
        // we optimize by adding multiple probes in a row).
        rayDimY += ddgiVolume->probeCount();
    }

    if (isNull(m_irradianceRaysShadedFB)) {
		m_irradianceRaysShadedFB = Framebuffer::create(Texture::createEmpty("DefaultRenderer::m_irradianceRaysShadedFB", 256, 256, ImageFormat::R11G11B10F()));

		m_irradianceRayOriginsTexture = Texture::createEmpty("DefaultRenderer::m_irradianceRayOriginsTexture", 256, 256, ImageFormat::RGBA32F());
		m_irradianceRayDirectionsTexture = Texture::createEmpty("DefaultRenderer::m_irradianceRayDirectionsTexture", 256, 256, ImageFormat::RGBA32F());

        GBuffer::Specification gbufferSpec;
        gbufferSpec.encoding[GBuffer::Field::LAMBERTIAN].format = ImageFormat::RGBA8();
        gbufferSpec.encoding[GBuffer::Field::GLOSSY].format = ImageFormat::RGBA8();
        gbufferSpec.encoding[GBuffer::Field::EMISSIVE].format = ImageFormat::RGBA32F();
        gbufferSpec.encoding[GBuffer::Field::WS_POSITION].format = ImageFormat::RGBA32F();
        gbufferSpec.encoding[GBuffer::Field::WS_NORMAL] = Texture::Encoding(ImageFormat::RGBA32F(), FrameName::CAMERA, 1.0f, 0.0f);
        gbufferSpec.encoding[GBuffer::Field::DEPTH_AND_STENCIL].format = nullptr;
        gbufferSpec.encoding[GBuffer::Field::CS_NORMAL] = nullptr;
        gbufferSpec.encoding[GBuffer::Field::CS_POSITION] = nullptr;

		m_irradianceRaysGBuffer = GBuffer::create(gbufferSpec, "DefaultRenderer::m_irradianceRaysGBuffer");
    }

    m_irradianceRaysShadedFB->resize(rayDimX, rayDimY);
    m_irradianceRaysGBuffer->resize(rayDimX, rayDimY);

    m_irradianceRayOriginsTexture->resize(rayDimX, rayDimY);
    m_irradianceRayDirectionsTexture->resize(rayDimX, rayDimY);

    END_PROFILER_EVENT();
}

void DefaultRenderer::resizeReflectionRayTextures(const int screenWidth, const int screenHeight) {
    BEGIN_PROFILER_EVENT("resizeReflectionRayTextures");
    // Reflection ray textures
    const int reflectionTextureHeight = iCeil(float(screenHeight) / float(m_glossyYScale));

    if (isNull(m_reflectionRayOriginsTexture)) {
		m_reflectionRayOriginsTexture = Texture::createEmpty("DefaultRenderer::m_reflectionRayOriginsTexture", 256, 256, ImageFormat::RGBA32F());
    }
    if (isNull(m_reflectionRayDirectionsTexture)) {
        m_reflectionRayDirectionsTexture = Texture::createEmpty("DefaultRenderer::m_reflectionRayDirectionsTexture", 256, 256, ImageFormat::RGBA32F());
    }
    m_reflectionRayOriginsTexture->resize(screenWidth, reflectionTextureHeight);
    m_reflectionRayDirectionsTexture->resize(screenWidth, reflectionTextureHeight);

    END_PROFILER_EVENT();
}

void DefaultRenderer::sampleArbitraryRays
(const shared_ptr<Texture>&           rayOrigins,
    const shared_ptr<Texture>&        rayDirections,
    const shared_ptr<TriTree>& tritree,
    const shared_ptr<GBuffer>&                      gbuffer,
    const int                                       totalRays,
    const unsigned int								visibilityMask,
    const int mipLevel) {
    const TriTree::IntersectRayOptions traceOptions = TriTree::DO_NOT_CULL_BACKFACES;
    const int width = rayOrigins->width(), height = rayOrigins->height();

    shared_ptr<Texture> rayOriginsAndDirections[2] = { rayOrigins, rayDirections };

    BEGIN_PROFILER_EVENT("Cache Ray Textures");
    for (int i = 0; i < 2; ++i) {
        const shared_ptr<Texture>& rayTexture = rayOriginsAndDirections[i];
        if (!m_rayOriginsAndDirectionsTable.containsKey(rayTexture)) {
            m_rayOriginsAndDirectionsTable.set(rayTexture, GLPixelTransferBuffer::create(width, height, ImageFormat::RGBA32F()));
        }
        // The DefaultRenderer resizes and caches its ray origin and direction textures,
        // so this is a no-op in all cases, and is only included for correctness.
        m_rayOriginsAndDirectionsTable[rayTexture]->resize(width, height);

        rayTexture->toPixelTransferBuffer(m_rayOriginsAndDirectionsTable[rayTexture]);
    }
    END_PROFILER_EVENT();

    const shared_ptr<GLPixelTransferBuffer>& rayOriginsBuffer = m_rayOriginsAndDirectionsTable[rayOrigins];
    const shared_ptr<GLPixelTransferBuffer>& rayDirectionsBuffer = m_rayOriginsAndDirectionsTable[rayDirections];


    BEGIN_PROFILER_EVENT("Grow Result Buffer");
    if (isNull(m_pboGBuffer[0]) || m_pboGBuffer[0]->width() < width || m_pboGBuffer[0]->height() < height) {
        for (int i = 0; i < 5; ++i) {
            // Take max of old width/height and new to
            // avoid ping-ponging for per-frame size changes.
            int oldWidth = 0, oldHeight = 0;
            if (notNull(m_pboGBuffer[i])) {
                oldWidth = m_pboGBuffer[i]->width();
                oldHeight = m_pboGBuffer[i]->height();
            }
            switch (i) {
            case 2:
            case 3:
                m_pboGBuffer[i] = GLPixelTransferBuffer::create(max(width, oldWidth), max(height, oldHeight), ImageFormat::RGBA8());
                break;
            default:
                m_pboGBuffer[i] = GLPixelTransferBuffer::create(max(width, oldWidth), max(height, oldHeight), ImageFormat::RGBA32F());
            }
        }
    }

    const Vector2int32& wavefrontDimensions = Vector2int32(width, iCeil(totalRays / (float)width));

    END_PROFILER_EVENT();
    debugAssertM(notNull(tritree), "TriTree is null!");
    BEGIN_PROFILER_EVENT("sample + interop");
    tritree->intersectRays(rayOriginsBuffer, rayDirectionsBuffer, m_pboGBuffer, traceOptions, nullptr, mipLevel, wavefrontDimensions, visibilityMask);
    END_PROFILER_EVENT();

    BEGIN_PROFILER_EVENT("Copy results to GBuffer");
    // Diffuse buffer, or ray trace enabled and on the ray trace buffer
    // Update the GBuffer
    gbuffer->texture(GBuffer::Field::WS_POSITION)->update(m_pboGBuffer[0], 0, CubeFace::POS_X, true, 0, false);
    gbuffer->texture(GBuffer::Field::WS_NORMAL)->update(m_pboGBuffer[1], 0, CubeFace::POS_X, true, 0, false);
    gbuffer->texture(GBuffer::Field::LAMBERTIAN)->update(m_pboGBuffer[2], 0, CubeFace::POS_X, true, 0, false);
    gbuffer->texture(GBuffer::Field::GLOSSY)->update(m_pboGBuffer[3], 0, CubeFace::POS_X, true, 0, false);
    gbuffer->texture(GBuffer::Field::EMISSIVE)->update(m_pboGBuffer[4], 0, CubeFace::POS_X, true, 0, false);
    END_PROFILER_EVENT();

    // Compute and writeout distance to intersection, with backface encoded in the sign bit.
    // Used by DDGI.
    if (totalRays > 0) {
        BEGIN_PROFILER_EVENT("Compute DDGI Distance encoding");
        {
            Args args;

            // Read only buffers
            args.setImageUniform("rayOriginsImage", rayOrigins, Access::READ);
            args.setImageUniform("rayDirectionsImage", rayDirections, Access::READ);
            args.setImageUniform("normalsImage", gbuffer->texture(GBuffer::Field::WS_NORMAL), Access::READ);

            // Position buffer to be modified.
            args.setImageUniform("positionsImage", gbuffer->texture(GBuffer::Field::WS_POSITION), Access::READ_WRITE);

            args.setComputeGroupSize(Vector3int32(16, 16, 1));
            args.setComputeGridDim(Vector3int32(iCeil((float)wavefrontDimensions.x / 16.0f), iCeil((float)wavefrontDimensions.y / 16.0f), 1));

            args.setUniform("gridSize", wavefrontDimensions);

            LAUNCH_SHADER("DDGIVolume/DDGIVolume_EncodeDDGIDistance.glc", args);

        }
        END_PROFILER_EVENT();
    }
}

    
void DefaultRenderer::shadeArbitraryRays
(RenderDevice* rd,
    const Array<shared_ptr<Surface>>& surfaceArray,
    const shared_ptr<Framebuffer>& targetFramebuffer,
    const LightingEnvironment& environment,
    const shared_ptr<Texture>& rayOrigins,
    const shared_ptr<Texture>& rayDirections,
    const shared_ptr<GBuffer>& gbuffer,
    const bool                                          useProbeIndirect,
    const bool                                          glossyToMatte) {

    //const int width = rayOrigins->width(), height = rayOrigins->height();

    // Find the skybox
    shared_ptr<SkyboxSurface> skyboxSurface;
    for (const shared_ptr<Surface>& surface : surfaceArray) {
        skyboxSurface = dynamic_pointer_cast<SkyboxSurface>(surface);
        if (skyboxSurface) { break; }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Perform deferred shading on the GBuffer
    rd->push2D(targetFramebuffer); {
        // Disable screen-space effects. Note that this is a COPY of LightingEnvironment we're making in order to mutate it,
        // and intentionally not a REFERENCE.
        LightingEnvironment e = environment;
        e.ambientOcclusionSettings.enabled = false;

        Args args;
        e.setShaderArgs(args);
        gbuffer->setShaderArgsRead(args, "gbuffer_");
        args.setRect(rd->viewport());

        args.setMacro("SECOND_ORDER_SHADING", true);
        args.setMacro("GLOSSY_TO_MATTE", glossyToMatte);

        // Intentionally ignore the glossy term on second order indirect; it is hard to notice in practice and costs us a lot to compute
        args.setUniform("glossyIndirectBuffer", Texture::opaqueBlack(), Sampler::buffer());

        args.setMacro("OVERRIDE_SKYBOX", true);
        if (skyboxSurface) {
            skyboxSurface->setShaderArgs(args, "skybox_");
        }

        args.setMacro("USE_GLOSSY_INDIRECT_BUFFER", false);

        // Arbitrary rays don't care about the camera position, but it's needed
        // as an argument for sampling multiple volumes, so set it at the origin.
        args.setUniform("cameraPos", Point3(0, 0, 0));

        args.setMacro("ENABLE_DIFFUSE_GI", useProbeIndirect);
        args.setMacro("ENABLE_SECOND_ORDER_GLOSSY", m_ddgiVolumeArray.size() > 0 ? m_ddgiVolumeArray[0]->specification().enableSecondOrderGlossy : false);
        rayDirections->setShaderArgs(args, "gbuffer_WS_RAY_DIRECTION_", Sampler::buffer());
        args.setUniform("energyPreservation", m_energyPreservation);

        if (isNull(m_deferredShader)) {
            m_deferredShader = Shader::getShaderFromPattern(m_shaderName + "_deferredShade.pix");
        }
        LAUNCH_SHADER_PTR(m_deferredShader, args);
    } rd->pop2D();
}

void DefaultRenderer::allocateAllOITBuffers
   (RenderDevice*                   rd,
    bool                            highPrecision) {
    
    const int lowResWidth  = rd->width()  / m_oitLowResDownsampleFactor;
    const int lowResHeight = rd->height() / m_oitLowResDownsampleFactor;

    m_oitFramebuffer = Framebuffer::create(m_textureNamePrefix + "m_oitFramebuffer");
    allocateOITFramebufferAttachments(rd, m_oitFramebuffer, rd->width(), rd->height(), highPrecision);
    m_oitLowResFramebuffer = Framebuffer::create(m_textureNamePrefix + "m_oitLowResFramebuffer");

    allocateOITFramebufferAttachments(rd, m_oitLowResFramebuffer, lowResWidth, lowResHeight, highPrecision);

    const ImageFormat* depthFormat = rd->drawFramebuffer()->texture(Framebuffer::DEPTH)->format();
    shared_ptr<Texture> lowResDepthBuffer = Texture::createEmpty(m_textureNamePrefix + "lowResDepth", lowResWidth, lowResHeight, depthFormat);
    m_oitLowResFramebuffer->set(Framebuffer::DEPTH, lowResDepthBuffer);

    m_backgroundFramebuffer = Framebuffer::create
        (Texture::createEmpty(m_textureNamePrefix + "backgroundTexture", 
            rd->width(), rd->height(), rd->drawFramebuffer()->texture(0)->format()));
    
    m_csOctLowResNormalFramebuffer = Framebuffer::create(Texture::createEmpty(m_textureNamePrefix + "m_csOctLowResNormalFramebuffer", 
            lowResWidth, lowResHeight, ImageFormat::RG8_SNORM()));

}


void DefaultRenderer::allocateOITFramebufferAttachments
   (RenderDevice*                   rd, 
    const shared_ptr<Framebuffer>&  oitFramebuffer, 
    int                             w,
    int                             h,
    bool                            highPrecision) {
    
    oitFramebuffer->set(Framebuffer::COLOR0, Texture::createEmpty(oitFramebuffer->name() + "/RT0 (A)", w, h, highPrecision ? HIGH_PRECISION_OIT_FORMAT : ImageFormat::RGBA16F()));
    oitFramebuffer->setClearValue(Framebuffer::COLOR0, Color4::zero());
    {
        const shared_ptr<Texture>& texture = Texture::createEmpty(oitFramebuffer->name() + "/RT1 (Brgb, D)", w, h, highPrecision ? HIGH_PRECISION_OIT_FORMAT : ImageFormat::RGBA8());
        texture->visualization.channels = Texture::Visualization::RGB;
        oitFramebuffer->set(Framebuffer::COLOR1, texture);
        oitFramebuffer->setClearValue(Framebuffer::COLOR1, Color4(1, 1, 1, 0));
    }
    {
        const shared_ptr<Texture>& texture = Texture::createEmpty(oitFramebuffer->name() + "/RT2 (delta)", w, h, highPrecision ? HIGH_PRECISION_OIT_FORMAT_RG : ImageFormat::RG8_SNORM());
        oitFramebuffer->set(Framebuffer::COLOR2, texture);
        oitFramebuffer->setClearValue(Framebuffer::COLOR2, Color4::zero());
    }
}


void DefaultRenderer::clearAndRenderToOITFramebuffer
   (RenderDevice*                   rd,
    const shared_ptr<Framebuffer>&  oitFramebuffer,
    Array < shared_ptr<Surface>>&   surfaceArray,
    const shared_ptr<GBuffer>&      gbuffer,
    const LightingEnvironment&      environment) {

    rd->setFramebuffer(oitFramebuffer);
    rd->clearFramebuffer(true, false);

    // Allow writePixel to read the depth buffer. Make the name unique so that it doesn't conflict with the depth texture
    // passed to ParticleSurface for soft particle depth testing
    oitFramebuffer->texture(Framebuffer::DEPTH)->setShaderArgs(oitFramebuffer->uniformTable, "_depthTexture.", Sampler::buffer());

    oitFramebuffer->uniformTable.setMacro("WRITE_PIXEL_FILENAME", m_oitWriteDeclarationGLSLFilename);
    rd->pushState(oitFramebuffer); {
        // Set blending modes
        // Accum (A)
        rd->setBlendFunc(RenderDevice::BLEND_ONE,  RenderDevice::BLEND_ONE,                 RenderDevice::BLENDEQ_ADD, RenderDevice::BLENDEQ_SAME_AS_RGB, Framebuffer::COLOR0);

        // Background modulation (beta) and diffusion (D)
        rd->setBlendFunc(Framebuffer::COLOR1,
                         RenderDevice::BLEND_ZERO, RenderDevice::BLEND_ONE_MINUS_SRC_COLOR, RenderDevice::BLENDEQ_ADD, 
                         RenderDevice::BLEND_ONE,  RenderDevice::BLEND_ONE,                 RenderDevice::BLENDEQ_ADD);

        // Refraction (delta)
        rd->setBlendFunc(RenderDevice::BLEND_ONE,  RenderDevice::BLEND_ONE,                 RenderDevice::BLENDEQ_ADD, RenderDevice::BLENDEQ_SAME_AS_RGB, Framebuffer::COLOR2);

        forwardShade(rd, surfaceArray, gbuffer, environment, RenderPassType::SINGLE_PASS_UNORDERED_BLENDED_SAMPLES, ARBITRARY);
    } rd->popState();
}


void DefaultRenderer::resizeOITBuffersIfNeeded
   (const int                       width, 
    const int                       height, 
    const int                       lowResWidth,
    const int                       lowResHeight) {
    
    if ((m_oitFramebuffer->width() != width) ||
        (m_oitFramebuffer->height() != height) ||
        (m_oitLowResFramebuffer->width() != lowResWidth) ||
        (m_oitLowResFramebuffer->height() != lowResHeight)) {

        m_oitFramebuffer->resize(width, height);
        m_oitLowResFramebuffer->resize(lowResWidth, lowResHeight);
        m_csOctLowResNormalFramebuffer->resize(lowResWidth, lowResHeight);
        m_backgroundFramebuffer->resize(width, height);
    }
}


void DefaultRenderer::computeLowResDepthAndNormals(RenderDevice* rd, const shared_ptr<Texture>& csHighResNormalTexture) {
    // Nearest-neighbor downsample depth
    Texture::copy
       (m_oitFramebuffer->texture(Framebuffer::DEPTH),
        m_oitLowResFramebuffer->texture(Framebuffer::DEPTH),
        0, 0,
        float(m_oitLowResDownsampleFactor),
        Vector2int16(0, 0),
        CubeFace::POS_X,
        CubeFace::POS_X,
        rd,
        false);

    // Downsample and convert normals to Octahedral format
    if (notNull(csHighResNormalTexture)) {
        rd->push2D(m_csOctLowResNormalFramebuffer); {
            Args args;

            csHighResNormalTexture->setShaderArgs(args, "csHighResNormalTexture.", Sampler::buffer());
            args.setUniform("lowResDownsampleFactor", m_oitLowResDownsampleFactor);
            args.setRect(rd->viewport());
            LAUNCH_SHADER("DefaultRenderer_downsampleNormal.pix", args);
        } rd->pop2D();
    }
}

void DefaultRenderer::renderIndirectIllumination
(RenderDevice* rd,
    const Array<shared_ptr<Surface> >& sortedVisibleSurfaceArray,
    const shared_ptr<GBuffer>& primaryGBuffer,
    const LightingEnvironment& environment,
    const shared_ptr<TriTree>& tritree) {

    BEGIN_PROFILER_EVENT("Render Indirect Illumination");

    if (!m_enableGlossyGI) {
        rd->pushState(m_glossyGIFramebuffer); {
            rd->setColorClearValue(Color4::zero());
            rd->clear(true, false, false);
        } rd->popState();
        END_PROFILER_EVENT();
        return;
    }

    traceGlossyIndirectIllumination(rd, sortedVisibleSurfaceArray, primaryGBuffer, environment, tritree);

    if (isNull(m_gaussianMIPFilter)) {
        m_gaussianMIPFilter = GaussianMIPFilter::create();
    }
    // 10. Compute glossy bilateral mipmaps
    // Bilateral blur according to distance and smoothness of the primary surface
    m_gaussianMIPFilter->apply(rd,
        m_blurredMIPReflectionFramebuffer->texture(0),
        primaryGBuffer->texture(GBuffer::Field::WS_POSITION),
        primaryGBuffer->texture(GBuffer::Field::WS_NORMAL),
        primaryGBuffer->texture(GBuffer::Field::GLOSSY),
        primaryGBuffer->camera()->frame());


    const int width = rd->framebuffer()->width(), height = rd->framebuffer()->height();
    m_glossyGIFramebuffer->resize(width, height);
    m_blurredMIPReflectionFramebuffer->resize(width, height);

    // 12. Compute `m_glossyGIFramebuffer`
    // Gather the correct MIP level for each pixel based on its distance and
    // smoothness
    rd->push2D(m_glossyGIFramebuffer); {
        Args args;
        args.setUniform("blurredReflections", m_blurredMIPReflectionFramebuffer->texture(0), Sampler(WrapMode::CLAMP, InterpolateMode::TRILINEAR_MIPMAP));
        args.setUniform("reflectedPosition",
            (notNull(m_reflectionGBuffer->texture(GBuffer::Field::WS_POSITION)) ? m_reflectionGBuffer : primaryGBuffer)->texture(GBuffer::Field::WS_POSITION),
            Sampler::buffer());
        args.setUniform("reflectedPositionScale", Vector2(float(m_reflectionGBuffer->width()), float(m_reflectionGBuffer->height()))
            / m_blurredMIPReflectionFramebuffer->vector2Bounds());
        primaryGBuffer->setShaderArgsRead(args, "gbuffer_");
        args.setUniform("mipLimit", m_gaussianMIPFilter->mipLimit());
        args.setRect(rd->viewport());
        LAUNCH_SHADER("DefaultRenderer_gatherGlossy.pix", args);
    } rd->pop2D();

    END_PROFILER_EVENT();
}

void DefaultRenderer::render
   (RenderDevice*                       rd,
    const shared_ptr<Camera>&           camera,
    const shared_ptr<Framebuffer>&      framebuffer,
    const shared_ptr<Framebuffer>&      depthPeelFramebuffer,
    LightingEnvironment&                lightingEnvironment,
    const shared_ptr<GBuffer>&          gbuffer, 
    const Array<shared_ptr<Surface>>&   allSurfaces,
    const std::function<const shared_ptr<TriTree>&()>& tritreeFunction) {

    alwaysAssertM(! lightingEnvironment.ambientOcclusionSettings.enabled || notNull(lightingEnvironment.ambientOcclusion),
        "Ambient occlusion is enabled but no ambient occlusion object is bound to the lighting environment");
    
    // Share the depth buffer with the forward-rendering pipeline
    if (notNull(gbuffer)) {
        framebuffer->set(Framebuffer::DEPTH, gbuffer->texture(GBuffer::Field::DEPTH_AND_STENCIL));
    }
    if (notNull(depthPeelFramebuffer)) {
        depthPeelFramebuffer->resize(framebuffer->width(), framebuffer->height());
    }

    // Cull and sort
    Array<shared_ptr<Surface> > sortedVisibleSurfaces, forwardOpaqueSurfaces, forwardBlendedSurfaces;
    cullAndSort(camera, gbuffer, framebuffer->rect2DBounds(), allSurfaces, sortedVisibleSurfaces, forwardOpaqueSurfaces, forwardBlendedSurfaces);

    debugAssert(framebuffer);
    // Bind the main framebuffer
    rd->pushState(framebuffer); {
        rd->clear();
        rd->setProjectionAndCameraMatrix(camera->projection(), camera->frame());
        
        const bool needDepthPeel = lightingEnvironment.ambientOcclusionSettings.useDepthPeelBuffer && lightingEnvironment.ambientOcclusionSettings.enabled;
        if (notNull(gbuffer)) {
            computeGBuffer(rd, sortedVisibleSurfaces, gbuffer, needDepthPeel ? depthPeelFramebuffer : nullptr, lightingEnvironment.ambientOcclusionSettings.depthPeelSeparationHint);
        }

        // Shadowing + AO
        computeShadowing(rd, allSurfaces, gbuffer, depthPeelFramebuffer, lightingEnvironment);
        debugAssertM(allSurfaces.size() < 500000, "It is very unlikely that you intended to draw 500k surfaces. There is probably heap corruption.");

        // Maybe launch deferred pass
        if (deferredShading()) {
            // We only use the tritree for tracing glossy here. The diffuse GI is handled in onSimulation by the app.
            const shared_ptr<TriTree>& tritree = (m_enableGlossyGI && tritreeFunction != nullptr) ? tritreeFunction() : nullptr;
            renderIndirectIllumination(rd, sortedVisibleSurfaces, gbuffer, lightingEnvironment, tritree);
            renderDeferredShading(rd, sortedVisibleSurfaces, gbuffer, lightingEnvironment);
        }

        // Main forward pass
        renderOpaqueSamples(rd, deferredShading() ? forwardOpaqueSurfaces : sortedVisibleSurfaces, gbuffer, lightingEnvironment);

        // Prepare screen-space lighting for the *next* frame
        lightingEnvironment.copyScreenSpaceBuffers(framebuffer, notNull(gbuffer) ? gbuffer->colorGuardBandThickness() : Vector2int16(), notNull(gbuffer) ? gbuffer->depthGuardBandThickness() : Vector2int16());

        renderOpaqueScreenSpaceRefractingSamples(rd, deferredShading() ? forwardOpaqueSurfaces : sortedVisibleSurfaces, gbuffer, lightingEnvironment);

        // Samples that require blending
        if (m_orderIndependentTransparency) {
            renderOrderIndependentBlendedSamples(rd, forwardBlendedSurfaces, gbuffer, lightingEnvironment);
        } else {
            renderSortedBlendedSamples(rd, forwardBlendedSurfaces, gbuffer, lightingEnvironment);
        }
    } rd->popState();

}


void DefaultRenderer::renderDeferredShading
   (RenderDevice*                       rd,  
    const Array<shared_ptr<Surface> >&  sortedVisibleSurfaceArray, 
    const shared_ptr<GBuffer>&          gbuffer,
    const LightingEnvironment&          environment) {

    debugAssertM(sortedVisibleSurfaceArray.size() < 500000, "It is very unlikely that you intended to draw 500k surfaces. There is probably heap corruption.");
    (void)sortedVisibleSurfaceArray;

    // Make a pass over the screen, performing shading
    rd->push2D(); {
        rd->setGuardBandClip2D(gbuffer->trimBandThickness());

        // Uncomment to avoid shading the skybox in the deferred pass because it will be forward rendered.
        // In practice, this is not a great savings because the deferred shader has an early out, and 
        // it causes some problems for screen-space effects if the skybox is not present.
        // rd->setDepthTest(RenderDevice::DEPTH_GREATER);

        Args args;
        setDeferredShadingArgs(args, gbuffer, environment);
        args.setRect(rd->viewport());

        if (isNull(m_deferredShader)) {
            m_deferredShader = Shader::getShaderFromPattern(m_shaderName + "_deferredShade.pix");
        }

        LAUNCH_SHADER_PTR(m_deferredShader, args);
    } rd->pop2D();
}


void DefaultRenderer::setDeferredShadingArgs(
    Args&                               args, 
    const shared_ptr<GBuffer>&          gbuffer, 
    const LightingEnvironment&          environment) {  

    environment.setShaderArgs(args);
    gbuffer->setShaderArgsRead(args, "gbuffer_");
    args.setMacro("COMPUTE_PERCENT", m_diskFramebuffer ? 100 : -1);

    args.setMacro("USE_GLOSSY_INDIRECT_BUFFER", m_enableGlossyGI);
    args.setMacro("GLOSSY_TO_MATTE", 0);
    args.setMacro("OVERRIDE_SKYBOX", false);
    args.setMacro("ENABLE_SECOND_ORDER_GLOSSY", false);
    args.setMacro("ENABLE_DIFFUSE_GI", m_enableDiffuseGI);
    // If glossy GI was disabled this framebuffer won't have been initialized.
    if (m_enableGlossyGI) {
        args.setUniform("glossyIndirectBuffer", m_glossyGIFramebuffer->texture(0), Sampler::buffer());
    }
    args.setUniform("energyPreservation", 1.0f);
    args.setUniform("cameraPos", gbuffer->camera()->frame().translation);
}


void DefaultRenderer::renderOpaqueSamples
   (RenderDevice*                       rd, 
    Array<shared_ptr<Surface> >&        surfaceArray, 
    const shared_ptr<GBuffer>&          gbuffer, 
    const LightingEnvironment&          environment) {

    BEGIN_PROFILER_EVENT("DefaultRenderer::renderOpaqueSamples");
    forwardShade(rd, surfaceArray, gbuffer, environment, RenderPassType::OPAQUE_SAMPLES, ARBITRARY);
    END_PROFILER_EVENT();
}


void DefaultRenderer::renderOpaqueScreenSpaceRefractingSamples
   (RenderDevice*                       rd, 
    Array<shared_ptr<Surface> >&        surfaceArray, 
    const shared_ptr<GBuffer>&          gbuffer, 
    const LightingEnvironment&          environment) {

    BEGIN_PROFILER_EVENT("DefaultRenderer::renderOpaqueScreenSpaceRefractingSamples");
    forwardShade(rd, surfaceArray, gbuffer, environment, RenderPassType::UNBLENDED_SCREEN_SPACE_REFRACTION_SAMPLES, ARBITRARY);
    END_PROFILER_EVENT();
}


void DefaultRenderer::renderSortedBlendedSamples       
   (RenderDevice*                       rd, 
    Array<shared_ptr<Surface> >&        surfaceArray, 
    const shared_ptr<GBuffer>&          gbuffer, 
    const LightingEnvironment&          environment) {

    BEGIN_PROFILER_EVENT("DefaultRenderer::renderSortedBlendedSamples");
    forwardShade(rd, surfaceArray, gbuffer, environment, RenderPassType::MULTIPASS_BLENDED_SAMPLES, BACK_TO_FRONT);
    END_PROFILER_EVENT();
}


void DefaultRenderer::renderOrderIndependentBlendedSamples
   (RenderDevice*                       rd, 
    Array<shared_ptr<Surface> >&        surfaceArray, 
    const shared_ptr<GBuffer>&          gbuffer, 
    const LightingEnvironment&          environment) {

    BEGIN_PROFILER_EVENT("DefaultRenderer::renderOrderIndependentBlendedSamples");

    if (surfaceArray.size() > 0) {

        // Categorize the surfaces by desired resolution
        static Array< shared_ptr<Surface> > hiResSurfaces;
        static Array< shared_ptr<Surface> > loResSurfaces;

        for (const shared_ptr<Surface>& s : surfaceArray) {
            if (s->preferLowResolutionTransparency() && (m_oitLowResDownsampleFactor != 1)) {
                loResSurfaces.append(s);
            } else {
                hiResSurfaces.append(s);
            }
        }

        const int lowResWidth  = rd->width()  / m_oitLowResDownsampleFactor;
        const int lowResHeight = rd->height() / m_oitLowResDownsampleFactor;

        // Test whether we need to allocate the OIT buffers 
        // (i.e., are they non-existent or at the wrong precision)
        if (isNull(m_oitFramebuffer) || 
            ((m_oitFramebuffer->texture(0)->format() == HIGH_PRECISION_OIT_FORMAT) != m_oitHighPrecision)) {
            allocateAllOITBuffers(rd, m_oitHighPrecision);
        }

        resizeOITBuffersIfNeeded(rd->width(), rd->height(), lowResWidth, lowResHeight);

        // Re-use the depth from the main framebuffer (for depth testing only)
        m_oitFramebuffer->set(Framebuffer::DEPTH, rd->drawFramebuffer()->texture(Framebuffer::DEPTH));
        
        // Copy the current color buffer to the background buffer, since we'll be compositing into
        // the color buffer at the end of the OIT process
        rd->drawFramebuffer()->blitTo(rd, m_backgroundFramebuffer, false, false, false, false, true);
        
        ////////////////////////////////////////////////////////////////////////////////////
        //
        // Accumulation pass over (3D) transparent surfaces
        //        
        const shared_ptr<Framebuffer>& oldBuffer = rd->drawFramebuffer();

        clearAndRenderToOITFramebuffer(rd, m_oitFramebuffer, hiResSurfaces, gbuffer, environment);

        if (loResSurfaces.size() > 0) {
            // Create a low-res copy of the depth (and normal) buffers for depth testing and then
            // for use as the key for bilateral upsampling.
            computeLowResDepthAndNormals(rd, gbuffer->texture(GBuffer::Field::CS_NORMAL));

            clearAndRenderToOITFramebuffer(rd, m_oitLowResFramebuffer, loResSurfaces, gbuffer, environment);
            rd->push2D(m_oitFramebuffer); {
                // Set blending modes
                // Accum (A)
                rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ONE, RenderDevice::BLENDEQ_ADD, RenderDevice::BLENDEQ_SAME_AS_RGB, Framebuffer::COLOR0);

                // Background modulation (beta) and diffusion (D)
                rd->setBlendFunc(Framebuffer::COLOR1,
                    RenderDevice::BLEND_ZERO, RenderDevice::BLEND_ONE_MINUS_SRC_COLOR, RenderDevice::BLENDEQ_ADD,
                    RenderDevice::BLEND_ONE, RenderDevice::BLEND_ONE, RenderDevice::BLENDEQ_ADD);

                // Delta (refraction)
                rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ONE, RenderDevice::BLENDEQ_ADD, RenderDevice::BLENDEQ_SAME_AS_RGB, Framebuffer::COLOR2);

                Args args;
                args.setMacro("FILTER_RADIUS",                              m_oitUpsampleFilterRadius);

                args.setUniform("sourceDepth",                              m_oitLowResFramebuffer->texture(Framebuffer::DEPTH), Sampler::buffer());
                args.setUniform("destDepth",                                m_oitFramebuffer->texture(Framebuffer::DEPTH), Sampler::buffer());
                args.setUniform("sourceSize",                               Vector2(float(m_oitLowResFramebuffer->width()), float(m_oitLowResFramebuffer->height())));
                args.setUniform("accumTexture",                             m_oitLowResFramebuffer->texture(0), Sampler::buffer());
                args.setUniform("backgroundModulationAndDiffusionTexture",  m_oitLowResFramebuffer->texture(1), Sampler::buffer());
                args.setUniform("deltaTexture",                             m_oitLowResFramebuffer->texture(2), Sampler::buffer());
                args.setUniform("downsampleFactor",                         m_oitLowResDownsampleFactor);

                const shared_ptr<Texture>& destNormal = gbuffer->texture(GBuffer::Field::CS_NORMAL);
                if (notNull(destNormal)) {
                    args.setMacro("HAS_NORMALS", true);
                    destNormal->setShaderArgs(args, "destNormal.", Sampler::buffer()); 
                    args.setUniform("sourceOctNormal", m_csOctLowResNormalFramebuffer->texture(0), Sampler::buffer());
                } else {
                    args.setMacro("HAS_NORMALS", false);
                }

                args.setRect(rd->viewport());

                if (isNull(m_upsampleOITShader)) {
                    m_upsampleOITShader = Shader::getShaderFromPattern(m_shaderName + "_upsampleOIT.pix");
                }
                LAUNCH_SHADER_PTR(m_upsampleOITShader, args);
            } rd->pop2D();
        }

        // Remove the color buffer binding which is shared with the main framebuffer, so that we don't 
        // clear it on the next pass through this function. Not done for colored OIT
        // m_oitFramebuffer->set(Framebuffer::COLOR2, shared_ptr<Texture>());
        rd->setFramebuffer(oldBuffer);

        ////////////////////////////////////////////////////////////////////////////////////
        //
        // 2D compositing pass
        //

        rd->push2D(); {
            rd->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS);
            Args args;          
            m_backgroundFramebuffer->texture(0)->setShaderArgs(args, "backgroundTexture.", Sampler(WrapMode::CLAMP, InterpolateMode::BILINEAR_NO_MIPMAP));

            const Projection& projection = gbuffer->camera()->projection();
            const float ppd = 0.05f * rd->viewport().height() / tan(projection.fieldOfViewAngles(rd->viewport()).y);
            args.setUniform("pixelsPerDiffusion2", square(ppd));
            args.setUniform("trimBandThickness", gbuffer->trimBandThickness());
            m_oitFramebuffer->texture(0)->setShaderArgs(args, "accumTexture.", Sampler::buffer());
            m_oitFramebuffer->texture(1)->setShaderArgs(args, "backgroundModulationAndDiffusionTexture.", Sampler::buffer());
            m_oitFramebuffer->texture(2)->setShaderArgs(args, "deltaTexture.", Sampler::buffer());
            args.setRect(rd->viewport());

            if (isNull(m_compositeOITShader)) {
                m_compositeOITShader = Shader::getShaderFromPattern(m_shaderName + "_compositeWeightedBlendedOIT.pix");
            }
            LAUNCH_SHADER_PTR(m_compositeOITShader, args);
        } rd->pop2D();

        hiResSurfaces.fastClear();
        loResSurfaces.fastClear();
    }

    END_PROFILER_EVENT();
}


} //namespace
