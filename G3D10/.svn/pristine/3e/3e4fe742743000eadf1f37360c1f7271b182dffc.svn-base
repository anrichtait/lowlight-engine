/**
  \file G3D-app.lib/source/VulkanTriTree.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifdef _MSC_VER

#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "StandAlone/DirStackFileIncluder.h"


#include "waveVK/waveVK.h"
#include "G3D-base/PixelTransferBuffer.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-app/UniversalMaterial.h"
#include "G3D-app/UniversalSurface.h"
#include "G3D-app/VulkanTriTree.h"

namespace G3D {

    const TBuiltInResource DefaultTBuiltInResource = {
        /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4
    };

    std::string GetSuffix(const std::string& name)
    {
        const size_t pos = name.rfind('.');
        return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
    }

    EShLanguage GetShaderStage(const std::string& stage)
    {
        if (stage == "rchit") {
            return EShLangClosestHitNV;
        }
        else if (stage == "rahit") {
            return EShLangAnyHitNV;
        }
        else if (stage == "rmiss") {
            return EShLangMissNV;
        }
        else if (stage == "rgen") {
            return EShLangRayGenNV;
        }
        else {
            assert(0 && "Unknown shader stage");
            return EShLangCount;
        }
    }

    void VulkanTriTree::finalizeAccelerationStructure() const {

        // TODO: these will be replaced by user compiled options.
        const std::string folder = FilePath::expandEnvironmentVariables("$(g3d)/G3D10/external/waveVK.lib/bin/").c_str();

        // Ray gen shader
        const std::string raygenFile = folder + "raygen.rgen";
        std::vector<unsigned int> raygenBytes;
        compileGLSLShader(raygenFile, raygenBytes);

        // Miss shader
        const std::string missFile = folder + "miss.rmiss";
        std::vector<unsigned int> missBytes;
        compileGLSLShader(missFile, missBytes);

        // Closest hit shader
        const std::string closesthitFile = folder + "closesthit.rchit";
        std::vector<unsigned int> closesthitBytes;
        compileGLSLShader(closesthitFile, closesthitBytes);

        // Any hit shader
        const std::string anyhitFile = folder + "anyhit.rahit";
        std::vector<unsigned int> anyhitBytes;
        compileGLSLShader(anyhitFile, anyhitBytes);

        // Shadow miss shader (reserved for future use)
        const std::string shadowmissFile = folder + "shadowMiss.rmiss";
        std::vector<unsigned int> shadowmissBytes;
        compileGLSLShader(shadowmissFile, shadowmissBytes);

        m_bvh->finalizeAccelerationStructure(raygenBytes, missBytes, closesthitBytes, anyhitBytes, shadowmissBytes);
    }

    void VulkanTriTree::compileGLSLShader(const std::string& inputFilename, std::vector<unsigned int>& resultBytes) const {

        if (!glslangInitialized) {
            glslang::InitializeProcess();
            glslangInitialized = true;
        }

        EShLanguage shaderStage = GetShaderStage(GetSuffix(inputFilename));


        std::ifstream fileStream(inputFilename);
        std::string shaderString((std::istreambuf_iterator<char>(fileStream)), (std::istreambuf_iterator<char>()));
        const char* shaderCString = shaderString.c_str();

        glslang::TShader shader(shaderStage);
        shader.setStrings(&shaderCString, 1);

        // Can also be queried from the implementation
        const int ClientInputSemanticsVersion = 100;
        const glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_1;
        const glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_5;

        shader.setEnvInput(glslang::EShSourceGlsl, shaderStage, glslang::EShClientVulkan, ClientInputSemanticsVersion);
        shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
        shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

        TBuiltInResource Resources = DefaultTBuiltInResource;
        Resources.limits.generalUniformIndexing = true; // from https://www.reddit.com/r/vulkan/comments/cm1t6c/nonconstantindexexpression_error_how_to_fix/.compact
        Resources.limits.generalSamplerIndexing = true;
        const EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

        const int DefaultVersion = 100;

        DirStackFileIncluder Includer;

        // Must be relative path.
        Includer.pushExternalLocalDirectory(inputFilename);

        std::string vertPreprocessedGLSL;

        if (!shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &vertPreprocessedGLSL, Includer))
        {
            std::cout << "GLSL Preprocessing Failed for: " << inputFilename << std::endl;
            std::cout << shader.getInfoLog() << std::endl;
            std::cout << shader.getInfoDebugLog() << std::endl;
        }

        //Reset string on the shaders
        const char* shaderPreCStr = vertPreprocessedGLSL.c_str();

        shader.setStrings(&shaderPreCStr, 1);

        if (!shader.parse(&Resources, 100, false, messages)) {
            debugPrintf(shader.getInfoLog());
            debugPrintf(shader.getInfoDebugLog());
            debugPrintf((std::string("Failed to parse shader ") + inputFilename + "\n").c_str());
            throw;
        }

        glslang::TProgram Program;
        Program.addShader(&shader);

        if (!Program.link(messages)) {
            std::cout << "Failed to link program." << std::endl;
        }

        spv::SpvBuildLogger logger = {};
        glslang::SpvOptions spvOptions = {};
        glslang::GlslangToSpv(*Program.getIntermediate(shader.getStage()), resultBytes, &logger);
    }

    VulkanTriTree::VulkanTriTree() {
        m_bvh = new wave::VKBVH();
    }

    VulkanTriTree::~VulkanTriTree() {
        delete m_bvh;
        m_bvh = nullptr;
    }

	void VulkanTriTree::ensureTextureCached(const shared_ptr<Texture>& tex) {
		if (m_textureCache.containsKey(tex->openGLID())) {
			return;
		}

		shared_ptr<Texture> texInterop = createVulkanInteropTexture(tex->width(), tex->height(), tex->encoding(), tex->numMipMapLevels(), "VK" + tex->name(), false);
		// TODO: create the correct number of mip levels in the interop texture.
		tex->copyInto(texInterop);
		m_textureCache.set(tex->openGLID(), texInterop);
	}

    int VulkanTriTree::createVulkanMaterial(const shared_ptr<UniversalMaterial>& material) {

        bool first = isNull(m_ignoreTexture);
        // Set the ignore texture, if necessary
        if (isNull(m_ignoreTexture)) {
            m_ignoreTexture = Texture::createEmpty("ignore", 1888,1122, ImageFormat::RGBA32F());
            Texture::copy(Texture::opaqueBlack(), m_ignoreTexture);
        }

        // Tri has a new material, so set the material in the wave::BVH and add it to the table.
        const shared_ptr<Texture>& bump = notNull(material->bump()) ? convertToVulkanFormat(material->bump()->normalBumpMap()->texture()) : m_ignoreTexture;
        const shared_ptr<Texture>& lambertian = material->bsdf()->hasLambertian() ? convertToVulkanFormat(material->bsdf()->lambertian().texture()) : m_ignoreTexture;
        const shared_ptr<Texture>& glossy = material->bsdf()->hasGlossy() ? convertToVulkanFormat(material->bsdf()->glossy().texture()) : m_ignoreTexture;
        const shared_ptr<Texture>& transmissive = material->hasTransmissive() ? convertToVulkanFormat(material->bsdf()->transmissive().texture()) : m_ignoreTexture;
        const shared_ptr<Texture>& emissive = material->hasEmissive() ? convertToVulkanFormat(material->emissive().texture()) : m_ignoreTexture;


        //bump->generateMipMaps();
        //lambertian->generateMipMaps();
        //glossy->generateMipMaps();
        //transmissive->generateMipMaps();
        //emissive->generateMipMaps();

        ensureTextureCached(bump);
        int bumpID = m_interopTextureTable[m_textureCache[bump->openGLID()]].textureIndex;

        ensureTextureCached(lambertian);
        int lambertianID = m_interopTextureTable[m_textureCache[lambertian->openGLID()]].textureIndex;

        ensureTextureCached(glossy);
        int glossyID = m_interopTextureTable[m_textureCache[glossy->openGLID()]].textureIndex;

        ensureTextureCached(transmissive);
        int transmissiveID = m_interopTextureTable[m_textureCache[transmissive->openGLID()]].textureIndex;

        ensureTextureCached(emissive);
        int emissiveID = m_interopTextureTable[m_textureCache[emissive->openGLID()]].textureIndex;

        return m_bvh->createMaterial(
            material->hasAlpha(),

            bumpID, // texN
            &bump->encoding().readMultiplyFirst[0],
            &bump->encoding().readAddSecond[0],

            lambertianID, // tex0
            &lambertian->encoding().readMultiplyFirst[0],
            &lambertian->encoding().readAddSecond[0],

            glossyID, // tex1
            &glossy->encoding().readMultiplyFirst[0],
            &glossy->encoding().readAddSecond[0],

            transmissiveID, // tex2
            &transmissive->encoding().readMultiplyFirst[0],
            &transmissive->encoding().readAddSecond[0],

            emissiveID, // tex3
            &emissive->encoding().readMultiplyFirst[0],
            &emissive->encoding().readAddSecond[0],

            (material->hasTransmissive()) ? material->bsdf()->etaReflect() / material->bsdf()->etaTransmit() : 1.0f, // etaPos / etaNeg

            material->flags()
        );
    }

	void VulkanTriTree::setContents
	(const Array<shared_ptr<Surface> >& surfaceArray,
		ImageStorage                       newStorage) {

		// Assume all IDs we're tracking from previous frames are stale.
		// If we discover it isn't stale, remove it from the list of objects scheduled for deletion.
		// If we discover a new value, add it.
		//m_sky = nullptr;

		//// all elements were marked as dead in the previous pass
		for (const shared_ptr<Surface>& s : surfaceArray) {
			const shared_ptr<UniversalSurface>& uniS = dynamic_pointer_cast<UniversalSurface>(s);
			if (isNull(uniS)) {
				// VulkanTriTree only supports UniversalSurface, but must fail silently in other cases because the skybox is not one.
				continue;
			}

			const CFrame& frame = uniS->frame();
			const uint64 rigidBodyID = uniS->rigidBodyID();

			bool created = false;
			SurfaceCacheElement& e = m_surfaceCache.getCreate(rigidBodyID, created);
			// Mark as live
			e.live = true;

			if (!created) {
				// We've cached the surface, so just update the transform.
				if (uniS->lastChangeTime() > m_lastBuildTime) {
                    m_bvh->setTransform(e.geometryIndex, static_cast<const float*>(frame.toMatrix4()));
				}
			}
			else {

				// Add the surface (and possibly material) to the cache.         
				//Array<int>                  index;
				//Array<Point3>               position;
				//Array<Vector3>              normal;
				//Array<Vector4>              tangentAndFacing;
				//Array<Point2>               texCoord;

				uniS->cpuGeom().vertexArray;
				//// Although this call peforms a giant CPU copy, it is needed because the data is stored
				//// in each model interlaced and so is not in a usable format for OptiX
				//uniS->getObjectSpaceGeometry(index, position, normal, tangentAndFacing, texCoord);

				const Array<int>& index = *(uniS->cpuGeom().index);
				const CPUVertexArray* cpuVertexArray = uniS->cpuGeom().vertexArray;

				std::vector<wave::Vertex> vertices;
				std::vector<int> indices;
				//        bool vertexArrayAdded = false;
				//        VertexCacheElement& vertices = m_vertexCache.getCreate(cpuVertexArray, vertexArrayAdded);
				//        if (vertexArrayAdded) {
				//            // Copy the data from the cpuGeom into the VertexCacheElement arrays
				for (const CPUVertexArray::Vertex& v : cpuVertexArray->vertex) {
					vertices.push_back({ v.position, v.normal, v.tangent, v.texCoord0 });
				}

				for (const int& idx : *(uniS->cpuGeom().index)) {
					indices.push_back(idx);
				}

				const Matrix4& M = frame.toMatrix4();


				// Assign materials
				const shared_ptr<UniversalMaterial>& material = uniS->material();

				int materialIdx = -1;
				if (notNull(material)) {
					// Always create a new material. We only cache textures for now.
					materialIdx = createVulkanMaterial(material);
				}
				else {
					alwaysAssertM(false, "VulkanTriTree only accepts UniversalMaterial");
				}

				bool newFrameCreated = false;
				int transformID = m_frameCache.getCreate(frame, newFrameCreated);
				if (newFrameCreated) {
					// nextID is always the nextID that will be used, so postincrement
					transformID = m_nextFrameCacheID++;
					m_frameCache[frame] = transformID;
				}

				GeometryIndex idx = m_bvh->createGeometry(vertices, (int)vertices.size(), indices, materialIdx, (int)indices.size(), static_cast<const float*>(M), true, 0);

				e.geometryIndex = idx;
			}
		}

		//static Array<uint64> staleArray;
		//staleArray.fastClear();
		//for (SurfaceCache::Iterator e = m_surfaceCache.begin(); e.hasMore(); ++e) {
		//    if (!e->value.live) {
		//        staleArray.push(e->key);
		//    }
		//    else {
		//        // Reset for the next pass
		//        e->value.live = false;
		//    }
		//}

		//// Remove all stale geometry
		//for (const uint64& rigidBodyId : staleArray) {
		//    SurfaceCacheElement e;
		//    if (m_surfaceCache.get(rigidBodyId, e)) {
		//        m_bvh->deleteGeometry(e.geometryIndex);
		//        m_surfaceCache.remove(rigidBodyId);
		//    }
		//    else {
		//        alwaysAssertM(false, "Stale ID not found in m_surfaceCache.");
		//    }
		//}

		m_lastBuildTime = System::time();
	}

    void VulkanTriTree::signalVKSemaphore(const shared_ptr<Texture>& tex) const {
        GLenum dstLayout = GL_LAYOUT_SHADER_READ_ONLY_EXT;
        GLuint texID = tex->openGLID();
        glSignalSemaphoreEXT(m_interopTextureTable[tex].glComplete, 0, nullptr, 1, &texID, &dstLayout);
        debugAssertGLOk();
    }

    void VulkanTriTree::waitVKSemaphore(const shared_ptr<Texture>& tex) const {
        GLenum dstLayout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
        GLuint texID = tex->openGLID();
        glWaitSemaphoreEXT(m_interopTextureTable[tex].glReady, 0, nullptr, 1, &texID, &dstLayout);
        debugAssertGLOk();
    }

    void VulkanTriTree::intersectRays
    (const shared_ptr<Texture>& rayOrigin,
        const shared_ptr<Texture>& rayDirection,
        const shared_ptr<GBuffer>& results,
        IntersectRayOptions        options,
        const shared_ptr<Texture>& rayCoherence) const {

        // 1. Request new textures from Vulkan
        static shared_ptr<Texture> vkRayOriginsTexture;
        static shared_ptr<Texture> vkRayDirectionsTexture;
        if (isNull(vkRayOriginsTexture)) {
            vkRayOriginsTexture = createVulkanInteropTexture(rayOrigin->width(), rayOrigin->height(), ImageFormat::RGBA32F(), 1, "vkRayOrigins", true);
            vkRayDirectionsTexture = createVulkanInteropTexture(rayDirection->width(), rayDirection->height(), ImageFormat::RGBA32F(), 1, "vkRayDirections", true);
        }

        // 2. Copy the ray origin and direction data into the vulkan textures.
        rayOrigin->copyInto(vkRayOriginsTexture);
        rayDirection->copyInto(vkRayDirectionsTexture);

        // 2.5 Normal GBuffer preparation, below.
        alwaysAssertM(notNull(results), "Result GBuffer must be preallocated");
        results->resize(rayOrigin->width(), rayOrigin->height());

        alwaysAssertM(notNull(results), "Result GBuffer must be preallocated");
        // Set up the G-buffer
        GBuffer::Specification spec;
        spec.encoding[GBuffer::Field::WS_POSITION] = ImageFormat::RGBA32F();
        spec.encoding[GBuffer::Field::WS_NORMAL] = ImageFormat::RGBA32F();
        spec.encoding[GBuffer::Field::CS_NORMAL] = nullptr;
        spec.encoding[GBuffer::Field::LAMBERTIAN] = ImageFormat::RGBA32F();
        spec.encoding[GBuffer::Field::GLOSSY] = ImageFormat::RGBA32F();
        spec.encoding[GBuffer::Field::EMISSIVE] = ImageFormat::RGBA32F();

        // Removing the depth buffer forces the deferred shader to read the explicit position buffer
        //spec.encoding[GBuffer::Field::TRANSMISSIVE]= ImageFormat::RGBA32F();
        spec.encoding[GBuffer::Field::DEPTH_AND_STENCIL] = nullptr;
        results->setSpecification(spec);
        results->resize(rayOrigin->width(), rayOrigin->height());
        results->prepare(RenderDevice::current, results->camera(), results->timeOffset(), results->timeOffset(), results->depthGuardBandThickness(), results->colorGuardBandThickness());

        // 3. Request result textures that map to each of the GBuffer fields.
        static Array<shared_ptr<Texture>> vkResultTextures;
        if (vkResultTextures.size() == 0) {
            vkResultTextures.resize(5);
            for (int i = 0; i < 5; ++i) {
                vkResultTextures[i] = createVulkanInteropTexture(rayOrigin->width(), rayOrigin->height(), ImageFormat::RGBA32F(), 1, format("ResultBuffer_%d", i), true);
            }
        }

        // 4. Call intersectRays with 7 textures (Vulkan backend overload).

        intersectRays(vkRayOriginsTexture, 
                      vkRayDirectionsTexture, 
                      vkResultTextures[0], // lambertian
                      vkResultTextures[1], // glossy
                      vkResultTextures[2], // emissive
                      vkResultTextures[3], // shading normal
                      vkResultTextures[4]);// position

        // 5. Update the GBuffer textures from the result textures.
        vkResultTextures[4]->copyInto(results->texture(GBuffer::Field::WS_POSITION));
        vkResultTextures[3]->copyInto(results->texture(GBuffer::Field::WS_NORMAL));
        vkResultTextures[0]->copyInto(results->texture(GBuffer::Field::LAMBERTIAN));
        vkResultTextures[1]->copyInto(results->texture(GBuffer::Field::GLOSSY));
        vkResultTextures[2]->copyInto(results->texture(GBuffer::Field::EMISSIVE));
    }

	void VulkanTriTree::intersectRays(
		const shared_ptr<Texture>& rayOrigins,
		const shared_ptr<Texture>& rayDirections,
        const shared_ptr<Texture>& lambertian,
        const shared_ptr<Texture>& glossy,
        const shared_ptr<Texture>& emissive,
        const shared_ptr<Texture>& shadingNormal,
        const shared_ptr<Texture>& position) const {

        static bool first = true;
        if (first) {
            finalizeAccelerationStructure();
            first = false;
        }

        const int width = rayOrigins->width();
        const int height = rayOrigins->height();

        signalVKSemaphore(rayOrigins);
        signalVKSemaphore(rayDirections);

        signalVKSemaphore(lambertian);
        signalVKSemaphore(glossy);
        signalVKSemaphore(emissive);
        signalVKSemaphore(shadingNormal);
        signalVKSemaphore(position);

		m_bvh->doVulkanRendering(
            width, 
            height,
            m_interopTextureTable[rayOrigins].textureIndex,
            m_interopTextureTable[rayDirections].textureIndex,
            m_interopTextureTable[lambertian].textureIndex,
            m_interopTextureTable[glossy].textureIndex,
            m_interopTextureTable[emissive].textureIndex,
            m_interopTextureTable[shadingNormal].textureIndex,
            m_interopTextureTable[position].textureIndex);

        waitVKSemaphore(rayOrigins);
        waitVKSemaphore(rayDirections);

        waitVKSemaphore(lambertian);
        waitVKSemaphore(glossy);
        waitVKSemaphore(emissive);
        waitVKSemaphore(shadingNormal);
        waitVKSemaphore(position);
	}

shared_ptr<Texture> VulkanTriTree::createVulkanInteropTexture(const int width, const int height, Texture::Encoding encoding, int mipLevels, const String& texName, bool buffer) const {

    VKInteropHandles handles;

    // Allocate memory for this texture in Vulkan. allocatedMemory = size of allocated memory in bytes
    uint64 allocatedMemory = 0;
    handles.textureIndex = m_bvh->allocateVulkanInteropTexture(width, height, mipLevels, buffer, allocatedMemory);

    assert(allocatedMemory > 0);

    String finaltexName = texName + "_" + String(std::to_string(int(handles.textureIndex)));

    GLuint glTexture = GL_NONE;

    glCreateTextures(GL_TEXTURE_2D, 1, &glTexture);

    // Import semaphores
    glGenSemaphoresEXT(1, &handles.glReady);
    glGenSemaphoresEXT(1, &handles.glComplete);

    // Platform specific import.  On non-Win32 systems use glImportSemaphoreFdEXT instead
    glImportSemaphoreWin32HandleEXT(handles.glReady, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, m_bvh->glReadyHandle(handles.textureIndex));
    glImportSemaphoreWin32HandleEXT(handles.glComplete, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, m_bvh->glCompleteHandle(handles.textureIndex));

    // Import memory
    glCreateMemoryObjectsEXT(1, &handles.memory);
    debugAssertGLOk();


    // Platform specific import.  On non-Win32 systems use glImportMemoryFdEXT instead.
    // We need to import the memory as the size that vulkan allocates it, but we create the
    // glTextureStorage with the size we want the *gl* texture to be, below.
    glImportMemoryWin32HandleEXT(handles.memory,
        allocatedMemory,
        GL_HANDLE_TYPE_OPAQUE_WIN32_EXT,
        m_bvh->glMemoryHandle(handles.textureIndex));

    debugAssertGLOk();

    // Use the imported memory as backing for the OpenGL texture.  The internalFormat, and mip 
    // count should match the ones used by Vulkan to create the image and determine its memory
    // allocation. The *dimensions* should match the openGL texture dimensions, which can (and
    // usually will) be smaller than the allocated memory.
    glTextureStorageMem2DEXT(glTexture, mipLevels, encoding.format->openGLFormat, width, height, handles.memory, 0);

    debugAssertGLOk();

    shared_ptr<Texture> tex = Texture::fromGLTexture(finaltexName, glTexture, encoding, AlphaFilter::ONE);

    m_interopTextureTable.set(tex, handles);

    return tex;
}

shared_ptr<Texture> VulkanTriTree::convertToVulkanFormat(shared_ptr<Texture> tex) {
    alwaysAssertM((tex->dimension() == Texture::DIM_2D) || (tex->dimension() == Texture::DIM_CUBE_MAP), "We currently only support 2D textures and cubemaps for interop with OptiX");

    // Cache of textures that had to be converted to another
    // opengl format before OptiX interop wrapping
    static Table<GLint, shared_ptr<Texture>> formatConvertedTextureCache;

    //if (!optixSupportsTexture(tex)) {
        // Convert from current format to RGBA32F
    // TODO: verify
	bool needsConversion = false;
	shared_ptr<Texture>& converted = formatConvertedTextureCache.getCreate(tex->openGLID(), needsConversion);
	if (needsConversion) {
		if (tex->dimension() == Texture::DIM_2D) {
			const ImageFormat* fmt = tex->format();
			const ImageFormat* fmtAlpha = ImageFormat::getFormatWithAlpha(fmt);
			const ImageFormat* newFormat = nullptr;

            // TODO: implement other formats besides RGBA32F
			//if ((tex->format()->code == ImageFormat::Code::CODE_SRGBA8) || (tex->format()->code == ImageFormat::Code::CODE_SRGB8)) {
			//	// sRGB8 -> RGBA16F or sRGBA8 -> RGBA16F
			//	newFormat = ImageFormat::RGBA16F();
			//}
			//else if (fmtAlpha) {
			//	// Add an alpha channel
			//	newFormat = fmtAlpha;
			//}
			//else {
				// Nothing worked, just use RGBA32F
				newFormat = ImageFormat::RGBA32F();
			//}
			converted = Texture::createEmpty("Converted0 " + tex->name(), tex->width(), tex->height(),
				Texture::Encoding(newFormat, tex->encoding().frame, tex->encoding().readMultiplyFirst, tex->encoding().readAddSecond));
			Texture::copy(tex, converted);
		}
	}
	tex = converted;
    //}

    return tex;
}

#if 0



    void VulkanTriTree::intersectRays
    (const shared_ptr<GLPixelTransferBuffer>&         rayOrigins,
        const shared_ptr<GLPixelTransferBuffer>&         rayDirections,
        const shared_ptr<GLPixelTransferBuffer>&         booleanResults,
        IntersectRayOptions                options) const {

        registerReallocationAndMapHooks(rayOrigins);
        registerReallocationAndMapHooks(rayDirections);

        registerReallocationAndMapHooks(booleanResults);

        alwaysAssertM((options & 1), "Intersect rays with a single output buffer assumes occlusion cast");
        m_bvh->occlusionCast(rayOrigins->glBufferID(), rayDirections->glBufferID(), rayOrigins->width(), rayOrigins->height(), booleanResults->glBufferID(), !(options & 8), !(options & 4), !(options & 2));
    }


    void VulkanTriTree::copyToRayPBOs(const Array<Ray>& rays) const {
        const int width = 512; const int height = iCeil((float)rays.size() / (float)width);
        //const int width = 640; const int height = 360;

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
        Vector4* originPointer = (Vector4*)m_rayOrigins->mapWrite();
        Vector4* directionPointer = (Vector4*)m_rayDirections->mapWrite();

        // Exploit the fact that we know the exact memory layout of the Ray class,
        // which was designed to match this format.
        runConcurrently(0, rays.size(), [&](int i) {
            const Ray& ray = rays[i];
            originPointer[i] = *(const Vector4*)(&ray);
            directionPointer[i] = *((const Vector4*)(&ray) + 1);
        });

        m_rayDirections->unmap();
        m_rayOrigins->unmap();
    }


    void VulkanTriTree::intersectRays
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


    void VulkanTriTree::intersectRays
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

        m_bvh->cast(
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
        );

        // Map the output data
        const Vector4uint16* hitLocation = reinterpret_cast<const Vector4uint16*>(m_outPBOArray[Field::HIT_LOCATION]->mapRead());
        const Vector4* position = reinterpret_cast<const Vector4*>(m_outPBOArray[Field::POSITION]->mapRead());

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


    void VulkanTriTree::intersectRays
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

    void VulkanTriTree::intersectRays
    (const shared_ptr<GLPixelTransferBuffer>&       rayOrigin,
        const shared_ptr<GLPixelTransferBuffer>&    rayDirection,
        const shared_ptr<GLPixelTransferBuffer>     results[5],
        IntersectRayOptions                         options,
        const shared_ptr<GLPixelTransferBuffer>&    rayCone) const {

        BEGIN_PROFILER_EVENT("Pre-cast");
        registerReallocationAndMapHooks(rayOrigin);
        registerReallocationAndMapHooks(rayDirection);

        for (int i = 0; i < 5; ++i) {
            registerReallocationAndMapHooks(results[i]);
        }
        const int width = rayOrigin->width();
        const int height = rayOrigin->height();
        END_PROFILER_EVENT();

        BEGIN_PROFILER_EVENT("wave.lib: ray cast");
        m_bvh->cast(
            rayOrigin->glBufferID(),
            rayDirection->glBufferID(),
            width,
            height,
            results[2]->glBufferID(), // Lambertian
            results[3]->glBufferID(), // Glossy
            GL_NONE,                  // Transmissive
            results[4]->glBufferID(), // Emissive

            GL_NONE,                  // Hit location
            results[1]->glBufferID(), // Shading Normal
            results[0]->glBufferID(), // Position
            GL_NONE,                  // Geometric normal 

            !(options & TriTree::NO_PARTIAL_COVERAGE_TEST),
            !(options & TriTree::DO_NOT_CULL_BACKFACES)
        );
        END_PROFILER_EVENT();
    }

    void VulkanTriTree::intersectRays
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

    void VulkanTriTree::intersectRays
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

        m_bvh->cast
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
        );

        debugAssertGLOk();

        const Vector4* position = reinterpret_cast<const Vector4*>(m_outPBOArray[Field::POSITION]->mapRead());
        const Vector4* geometricNormal = reinterpret_cast<const Vector4*>(m_outPBOArray[Field::GEOMETRIC_NORMAL]->mapRead());
        const Vector4* shadingNormal = reinterpret_cast<const Vector4*>(m_outPBOArray[Field::SHADING_NORMAL]->mapRead());
        const Color4* lambertian = reinterpret_cast<const Color4*>(m_outPBOArray[Field::MATERIAL0]->mapRead());
        const Color4* glossy = reinterpret_cast<const Color4*>(m_outPBOArray[Field::MATERIAL1]->mapRead());
        const Color4* transmissive = reinterpret_cast<const Color4*>(m_outPBOArray[Field::MATERIAL2]->mapRead());
        const Color4* emissive = reinterpret_cast<const Color4*>(m_outPBOArray[Field::MATERIAL3]->mapRead());

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


    bool VulkanTriTree::intersectRay
    (const Ray&                         ray,
        Hit&                               hit,
        IntersectRayOptions                options) const {

        Array<Ray> rays = { ray };
        Array<Hit> hits = { hit };

        copyToRayPBOs(rays);

        intersectRays(m_rayOrigins, m_rayDirections, hits, options);

        return hits[0].triIndex != Hit::NONE;
    }

    void VulkanTriTree::registerReallocationAndMapHooks(const shared_ptr<GLPixelTransferBuffer>& t) const {

        std::weak_ptr<GLPixelTransferBuffer> b;
        int id = t->glBufferID();
        if (m_registeredBufferIDs.get(id, b)) {
            if (!b.expired()) {
                return;
            }
        }
        m_registeredBufferIDs.set(id, std::weak_ptr<GLPixelTransferBuffer>(t));
        // Use weak_ptr to reference the VulkanTriTree without incrementing the reference count,  
        // allowing us to collect the tree before the registered textures if we ever need to.
        t->registerReallocationHook([tritree = std::weak_ptr<const VulkanTriTree>(dynamic_pointer_cast<const VulkanTriTree>(shared_from_this()))](GLint glID) {
            const shared_ptr<const VulkanTriTree>& ptr = tritree.lock();
            if (notNull(ptr)) { ptr->m_bvh->unregisterCachedBuffer(glID); };
        });

        t->registerMapHook([tritree = std::weak_ptr<const VulkanTriTree>(dynamic_pointer_cast<const VulkanTriTree>(shared_from_this()))](GLint glID) {
            const shared_ptr<const VulkanTriTree>& ptr = tritree.lock();
            if (notNull(ptr)) { ptr->m_bvh->unmapCachedBuffer(glID); };
        });

    }
#endif
}
#endif
