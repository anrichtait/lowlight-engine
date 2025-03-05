/*
Copyright 2019, NVIDIA

2-Clause BSD License (https://opensource.org/licenses/BSD-2-Clause)

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <G3D/G3D.h>

#define VK_USE_PLATFORM_WIN32_KHR
// Vulkan!
#include "vulkan/vulkan.hpp"

// #VKRay
#define ALLOC_DEDICATED
#include "raytrace_vkpp.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "../include/vks/context.hpp"
#include "../include/vks/pipelines.hpp"

#include "../include/waveVK/waveVK.h"

// #VKRay
#include "../include/nv_helpers_vk/BottomLevelASGenerator.h"
#include "../include/nv_helpers_vk/TopLevelASGenerator.h"
#include "../include/nv_helpers_vk/VKHelpers.h"
#include "../include/nv_helpers_vk/DescriptorSetGenerator.h"
#include "../include/nv_helpers_vk/RaytracingPipelineGenerator.h"
#include "../include/nv_helpers_vk/ShaderBindingTableGenerator.h"

#define RAY_TRACE_IMAGE

typedef void* HANDLE;
struct ShareHandles {
    HANDLE memory{ INVALID_HANDLE_VALUE };
    HANDLE glReady{ INVALID_HANDLE_VALUE };
    HANDLE glComplete{ INVALID_HANDLE_VALUE };
};

Matrix4 minimalGLPerspective(float pixelWidth, float pixelHeight, float nearZ, float farZ, float verticalRadians, float subpixelShiftX = 0.0f, float subpixelShiftY = 0.0f) {
    const float k = 1.0f / tan(verticalRadians / 2.0f);

    const float c = (farZ == -INFINITY) ? -1.0f : (nearZ + farZ) / (nearZ - farZ);
    const float d = (farZ == -INFINITY) ? 1.0f : farZ / (nearZ - farZ);

    Matrix4 P(k * pixelHeight / pixelWidth, 0.0f, subpixelShiftX * k / (nearZ * pixelWidth), 0.0f,
        0.0f, k, subpixelShiftY * k / (nearZ * pixelHeight), 0.0f,
        0.0f, 0.0f, c, -2.0f * nearZ * d,
        0.0f, 0.0f, -1.0f, 0.0f);

    return P;
}

namespace wave {

	class _VKBVH {
	public:
		struct SharedResources {
			vks::Image texture;
			struct {
				vk::Semaphore glReady;
				vk::Semaphore glComplete;
			} semaphores;
			vk::CommandBuffer transitionCmdBuf;
			ShareHandles handles;
			vk::Device device;

			void init(const vks::Context& context, int width, int height, vk::Format format, int mipLevels, bool buffer) {
				device = context.device;
				vk::DispatchLoaderDynamic dynamicLoader{ context.instance, &vkGetInstanceProcAddr, device, &vkGetDeviceProcAddr };
				{
					auto handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
					{
						vk::SemaphoreCreateInfo sci;
						vk::ExportSemaphoreCreateInfo esci;
						sci.pNext = &esci;
						esci.handleTypes = handleType;
						semaphores.glReady = device.createSemaphore(sci);
						semaphores.glComplete = device.createSemaphore(sci);
					}
					handles.glReady = device.getSemaphoreWin32HandleKHR({ semaphores.glReady, handleType }, dynamicLoader);
					handles.glComplete = device.getSemaphoreWin32HandleKHR({ semaphores.glComplete, handleType }, dynamicLoader);
				}

				{
                    vk::ExternalMemoryImageCreateInfo externImageCreateInfo;
                    externImageCreateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;

					vk::ImageCreateInfo imageCreateInfo;
					imageCreateInfo.imageType = vk::ImageType::e2D;
					imageCreateInfo.format = format;
					imageCreateInfo.mipLevels = mipLevels;
					imageCreateInfo.arrayLayers = 1;
					imageCreateInfo.extent.depth = 1;
					imageCreateInfo.extent.width = width;
					imageCreateInfo.extent.height = height;
					imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
					imageCreateInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
                    imageCreateInfo.pNext = &externImageCreateInfo;
					texture.image = device.createImage(imageCreateInfo);
					texture.device = device;
					texture.format = imageCreateInfo.format;
					texture.extent = imageCreateInfo.extent;
				}

				{
					vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(texture.image);
					vk::MemoryAllocateInfo memAllocInfo;
					vk::ExportMemoryAllocateInfo exportAllocInfo{ vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32 };
					memAllocInfo.pNext = &exportAllocInfo;
					memAllocInfo.allocationSize = texture.allocSize = memReqs.size;
					memAllocInfo.memoryTypeIndex = context.getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
					texture.memory = device.allocateMemory(memAllocInfo);
					device.bindImageMemory(texture.image, texture.memory, 0);
					handles.memory = device.getMemoryWin32HandleKHR({ texture.memory, vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32 }, dynamicLoader);
				}

				{
					// Create sampler
					vk::SamplerCreateInfo samplerCreateInfo;
					samplerCreateInfo.magFilter = buffer ? vk::Filter::eNearest : vk::Filter::eLinear;
					samplerCreateInfo.minFilter = buffer ? vk::Filter::eNearest : vk::Filter::eLinear;
					samplerCreateInfo.mipmapMode = buffer ? vk::SamplerMipmapMode::eNearest : vk::SamplerMipmapMode::eLinear;
					// Max level-of-detail should match mip level count
					samplerCreateInfo.maxLod = (float)mipLevels;
					// Only enable anisotropic filtering if enabled on the device
					samplerCreateInfo.maxAnisotropy = context.deviceFeatures.samplerAnisotropy ? context.deviceProperties.limits.maxSamplerAnisotropy : 1.0f;
					samplerCreateInfo.anisotropyEnable = VK_FALSE;  //context.deviceFeatures.samplerAnisotropy;
					samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
					texture.sampler = device.createSampler(samplerCreateInfo);
				}
				{
					vk::ImageViewCreateInfo imageView;
					imageView.viewType = vk::ImageViewType::e2D;
					imageView.format = texture.format;
					imageView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
					imageView.subresourceRange.baseMipLevel = 0;
					imageView.subresourceRange.levelCount = mipLevels;
					imageView.subresourceRange.baseArrayLayer = 0;
					imageView.subresourceRange.layerCount = 1;
					imageView.image = texture.image;
					texture.view = device.createImageView(imageView);
				}
			}

			void destroy() {
				texture.destroy();
				device.destroy(semaphores.glComplete);
				device.destroy(semaphores.glReady);
			}
		};

		// Instance of the OBJ
		struct ObjInstance
		{
			G3D::Matrix4 transform{ G3D::Matrix4::identity() };    // Position of the instance
			G3D::Matrix4 transformIT{ G3D::Matrix4::identity() };  // Inverse transpose
		};

		std::vector<ObjInstance> m_objInstance;

		// Vector containing all shared textures resources.
		// New shared resources can be requested by client code.
		std::vector<SharedResources> m_sharedResourceVector;

		// For compaction.
		std::vector<vk::Semaphore> m_readySemaphores;
		std::vector<vk::Semaphore> m_completeSemaphores;

		vks::Context context;
		vk::Instance instance;
		vk::Device& device{ context.device };
		vk::DescriptorSetLayout descriptorSetLayout;

		//vk::Extent2D size;
		//uint32_t& width{ size.width };
		//uint32_t& height{ size.height };
		vk::RenderPass renderPass;
		vk::Framebuffer framebuffer = nullptr;
		// Color attachment is now the shared resource.
		vks::Image depthAttachment;

		// Ray Tracing
		VkPhysicalDeviceRayTracingPropertiesNV m_raytracingProperties = {};

		// Camera *frame* (the inverse of the camera view matrix).
		Matrix4 m_cameraFrame = Matrix4::identity();

		struct UniformBufferObject
		{
			Matrix4 model;
			Matrix4 view;
			Matrix4 proj;
			Matrix4 modelIT;

			// ray tracing tutorial
			Matrix4 viewInverse;
			Matrix4 projInverse;
		};

		vks::Buffer    m_uniformBuffer;

		// Updated with the uniform buffer.
		vks::Buffer    m_transformInverseTranspose;
		vks::Buffer    m_materialBuffer;

		struct Material {
			alignas(4) int         textureNID;
			alignas(4) int         texture0ID;
			alignas(4) int         texture1ID;
			alignas(4) int         texture2ID;
			alignas(4) int         texture3ID;

			alignas(16) G3D::Vector4  textureNScale;
			alignas(16) G3D::Vector4  textureNBias;
			alignas(16) G3D::Vector4   texture0Scale;
			alignas(16) G3D::Vector4   texture0Bias;
			alignas(16) G3D::Vector4  texture1Scale;
			alignas(16) G3D::Vector4  texture1Bias;
			alignas(16) G3D::Vector4  texture2Scale;
			alignas(16) G3D::Vector4  texture2Bias;
			alignas(16) G3D::Vector4  texture3Scale;
			alignas(16) G3D::Vector4  texture3Bias;

		};
		std::vector<Material> m_materials;

		struct GeometryInstance
		{
			vks::Buffer vertexBuffer;
			int vertexCount;
			VkDeviceSize vertexOffset;
			vks::Buffer indexBuffer;
			int indexCount;
			VkDeviceSize indexOffset;
			G3D::Matrix4 transform;
			int materialIndex;
		};
		std::vector<GeometryInstance> m_geometryInstances;

		struct AccelerationStructure
		{
			VkBuffer scratchBuffer = VK_NULL_HANDLE;
			VkDeviceMemory scratchMem = VK_NULL_HANDLE;
			VkBuffer resultBuffer = VK_NULL_HANDLE;
			VkDeviceMemory resultMem = VK_NULL_HANDLE;
			VkBuffer instancesBuffer = VK_NULL_HANDLE;
			VkDeviceMemory instancesMem = VK_NULL_HANDLE;
			VkAccelerationStructureNV structure = VK_NULL_HANDLE;
		};

		nv_helpers_vk::TopLevelASGenerator m_topLevelASGenerator;
		AccelerationStructure              m_topLevelAS;
		std::vector<AccelerationStructure> m_bottomLevelAS;

		nv_helpers_vk::DescriptorSetGenerator m_rtDSG;
		VkDescriptorPool                      m_rtDescriptorPool;
		VkDescriptorSetLayout                 m_rtDescriptorSetLayout;
		VkDescriptorSet                       m_rtDescriptorSet;


		VkPipelineLayout m_rtPipelineLayout = VK_NULL_HANDLE;
		VkPipeline       m_rtPipeline = VK_NULL_HANDLE;

		uint32_t m_rayGenIndex;
		uint32_t m_hitGroupIndex;
		uint32_t m_missIndex;

		nv_helpers_vk::ShaderBindingTableGenerator      m_sbtGen;
		vks::Buffer                                     m_shaderBindingTableBuffer;

		uint32_t m_shadowMissIndex;
		uint32_t m_shadowHitGroupIndex;

		vk::CommandBuffer mainLoopCommandBuffer = nullptr;

		// New stuff
		nvvkpp::RaytracingBuilder                       m_rtBuilder;
		VkAccelerationStructureNV                       m_ctypeAccelStructure;

		void updateCameraFrame(const Matrix4& m) {
			m_cameraFrame = m;
		}

		G3D::Matrix4 makeGLMMatrixFromPointer(const float* ptr) {
			return G3D::Matrix4(ptr[0], ptr[1], ptr[2], ptr[3],
				ptr[4], ptr[5], ptr[6], ptr[7],
				ptr[8], ptr[9], ptr[10], ptr[11],
				ptr[12], ptr[13], ptr[14], ptr[15]);
		}

		int createMaterial(const bool   hasAlpha,
			const int    textureNIndex,
			const float* scale_n,
			const float* bias_n,
			const int    texture0Index,
			const float* scale_0,
			const float* bias_0,
			const int    texture1Index,
			const float* scale_1,
			const float* bias_1,
			const int    texture2Index,
			const float* scale_2,
			const float* bias_2,
			const int    texture3Index,
			const float* scale_3,
			const float* bias_3,
			const float  materialConstant,
			uint8         flags)
		{
			// For now, just the texture indices.
			m_materials.push_back({ textureNIndex, texture0Index, texture1Index, texture2Index, texture3Index,
				G3D::Vector4(scale_n[0], scale_n[1], scale_n[2], scale_n[3]), G3D::Vector4(bias_n[0], bias_n[1], bias_n[2], bias_n[3]),
				G3D::Vector4(scale_0[0], scale_0[1], scale_0[2], scale_0[3]), G3D::Vector4(bias_0[0], bias_0[1], bias_0[2], bias_0[3]),
				G3D::Vector4(scale_1[0], scale_1[1], scale_1[2], scale_1[3]), G3D::Vector4(bias_1[0], bias_1[1], bias_1[2], bias_1[3]),
				G3D::Vector4(scale_2[0], scale_2[1], scale_2[2], scale_2[3]), G3D::Vector4(bias_2[0], bias_2[1], bias_2[2], bias_2[3]),
				G3D::Vector4(scale_3[0], scale_3[1], scale_3[2], scale_3[3]), G3D::Vector4(bias_3[0], bias_3[1], bias_3[2], bias_3[3])
				});
			return (int)m_materials.size() - 1;
		}

		// Only call this function after the vertex and index buffers have been initialized.
		int createGeometryInstance(const std::vector<Vertex>& vertices,
			const int					                      numVertices,
			const std::vector<int>& indices,
			const int					                      numIndices,
			const int                                         materialIndex,
			const float					                      geometryToWorldRowMajorMatrix[16]) {

			// Vertices
			vks::Buffer vertexBuffer = context.stageToDeviceBuffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, vertices);

			// Indices
			vks::Buffer indexBuffer = context.stageToDeviceBuffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, indices);

			// Debugging
			//for (int i = 0; i < 16; ++i) {
			//	float x = geometryToWorldRowMajorMatrix[i];
			//	if (x) {}
			//}

			G3D::Matrix4 id = G3D::Matrix4();
			G3D::Matrix4 incoming = makeGLMMatrixFromPointer(geometryToWorldRowMajorMatrix);

			ObjInstance instance;
			instance.transform = incoming;
            instance.transformIT = incoming.inverse().transpose();
			m_objInstance.emplace_back(instance);

			m_geometryInstances.push_back(
				//{ vertexBuffer, numVertices, 0, indexBuffer, numIndices, 0, G3D::Matrix4()});
				{ vertexBuffer, numVertices, 0, indexBuffer, numIndices, 0, incoming, materialIndex });

			return (int)m_geometryInstances.size() - 1;
		}

        void setTransform(int geometryIndex,
            const float geometryToWorldRowMajorMatrix[16]) {
        
            G3D::Matrix4 id = G3D::Matrix4();
            G3D::Matrix4 newTransform = makeGLMMatrixFromPointer(geometryToWorldRowMajorMatrix);

            m_geometryInstances[geometryIndex].transform = newTransform;
        }

		//--------------------------------------------------------------------------------------------------
	// Converting a OBJ primitive to the ray tracing geometry used for the BLAS
	//
		vk::GeometryNV objectToVkGeometryNV(const _VKBVH::GeometryInstance& model)
		{
			vk::GeometryTrianglesNV triangles;
			triangles.setVertexData(model.vertexBuffer.buffer);
			triangles.setVertexOffset(0);  // Start at the beginning of the buffer
			triangles.setVertexCount(model.vertexCount);
			triangles.setVertexStride(sizeof(Vertex));
			triangles.setVertexFormat(vk::Format::eR32G32B32Sfloat);  // 3xfloat32 for vertices
			triangles.setIndexData(model.indexBuffer.buffer);
			triangles.setIndexOffset(0);
			triangles.setIndexCount(model.indexCount);
			triangles.setIndexType(vk::IndexType::eUint32);  // 32-bit indices
			vk::GeometryDataNV geoData;
			geoData.setTriangles(triangles);
			vk::GeometryNV geometry;
			geometry.setGeometry(geoData);
			// Consider the geometry opaque for optimization
			geometry.setFlags(vk::GeometryFlagBitsNV::eOpaque);
			return geometry;
		}

		void createBottomLevelAS() {
			// BLAS - Storing each primitive in a geometry
			std::vector<std::vector<vk::GeometryNV>> blas;
			blas.reserve(m_geometryInstances.size());
			for (size_t i = 0; i < m_geometryInstances.size(); i++)
			{
				auto geo = objectToVkGeometryNV(m_geometryInstances[i]);
				// We could add more geometry in each BLAS, but we add only one for now
				blas.push_back({ geo });
			}
			m_rtBuilder.buildBlas(blas);
		}

		// Top level acceleration structure
		void createTopLevelAS()
		{
			std::vector<nvvkpp::RaytracingBuilder::Instance> tlas;
			tlas.reserve(m_geometryInstances.size());
			for (int i = 0; i < m_geometryInstances.size(); i++)
			{
				nvvkpp::RaytracingBuilder::Instance rayInst;
				rayInst.transform = m_geometryInstances[i].transform.transpose();  // Position of the instance
				rayInst.instanceId = i;                           // gl_InstanceID
				rayInst.blasId = i;// m_geometryInstances[i].objIndex;
				rayInst.hitGroupId = 0;  // We will use the same hit group for all objects
				rayInst.flags = vk::GeometryInstanceFlagBitsNV::eForceNoOpaque;
				tlas.emplace_back(rayInst);
			}
            m_rtBuilder.buildTlas(tlas, vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsNV::eAllowUpdate);
		}

        // Top level acceleration structure
        void updateTopLevelAS()
        {
            std::vector<nvvkpp::RaytracingBuilder::Instance> tlas;
            tlas.reserve(m_geometryInstances.size());
            for (int i = 0; i < m_geometryInstances.size(); i++)
            {
                nvvkpp::RaytracingBuilder::Instance rayInst;
                rayInst.transform = m_geometryInstances[i].transform.transpose();  // Position of the instance
                rayInst.instanceId = i;                           // gl_InstanceID
                rayInst.blasId = i;// m_geometryInstances[i].objIndex;
                rayInst.hitGroupId = 0;  // We will use the same hit group for all objects
                rayInst.flags = vk::GeometryInstanceFlagBitsNV::eForceNoOpaque;
                tlas.emplace_back(rayInst);
            }
            m_rtBuilder.updateTlasMatrices(tlas);
        }

		void createInstanceTransformBuffer() {
			// Create storage buffer to hold the transforms.
			VkDeviceSize bufferSize = sizeof(G3D::Matrix4) * m_geometryInstances.size();
			m_transformInverseTranspose = context.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				bufferSize);
		}

		void createMaterialBuffer() {
			// Create storage buffer to hold materials.
			VkDeviceSize bufferSize = sizeof(Material) * m_materials.size();
			m_materialBuffer = context.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				bufferSize);
		}

    void destroyAccelerationStructure(const AccelerationStructure& as) {
        vkDestroyBuffer(device, as.scratchBuffer, nullptr);
        vkFreeMemory(device, as.scratchMem, nullptr);
        vkDestroyBuffer(device, as.resultBuffer, nullptr);
        vkFreeMemory(device, as.resultMem, nullptr);
        vkDestroyBuffer(device, as.instancesBuffer, nullptr);
        vkFreeMemory(device, as.instancesMem, nullptr);
        vkDestroyAccelerationStructureNV(device, as.structure, nullptr);
    }

    // Create the descriptor set used by the raytracing shaders: note that all
    // shaders will access the same descriptor set, and therefore the set needs to
    // contain all the resources used by the shaders. For example, it will contain
    // all the textures used in the scene.
    void createRaytracingDescriptorSet()
    {
        // We will bind the vertex and index buffers, so we first add a barrier on
        // those buffers to make sure their data is actually present on the GPU
        VkBufferMemoryBarrier bmb = {};
        bmb.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bmb.pNext = nullptr;
        bmb.srcAccessMask = 0;
        bmb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bmb.offset = 0;
        bmb.size = VK_WHOLE_SIZE;

        // Add the bindings to the resources
        // Top-level acceleration structure, usable by both the ray generation and the
        // closest hit (to shoot shadow rays)
        m_rtDSG.AddBinding(0, 1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
            VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV); // Second added for shadow rays
// Raytracing output
        m_rtDSG.AddBinding(1, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
        // Camera information
        m_rtDSG.AddBinding(2, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_NV);
        

        
        // Vertex buffer
        m_rtDSG.AddBinding(3, (uint32_t)m_geometryInstances.size(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV);
        // Index buffer
        m_rtDSG.AddBinding(4, (uint32_t)m_geometryInstances.size(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV);
        // Ray Origins Image
        m_rtDSG.AddBinding(5, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		// Ray Directions Image
        m_rtDSG.AddBinding(6, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);

        // Scene data with transform
        m_rtDSG.AddBinding(7, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV);
        
        // lambertian, glossy, emissive, shadingNormal, position
        m_rtDSG.AddBinding(8, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
        m_rtDSG.AddBinding(9, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
        m_rtDSG.AddBinding(10, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
        m_rtDSG.AddBinding(11, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
        m_rtDSG.AddBinding(12, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);

        // Texture buffer
        m_rtDSG.AddBinding(13, (uint32_t)m_sharedResourceVector.size() - 7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV);

        // Material buffer
        m_rtDSG.AddBinding(14, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV);


        // Create the descriptor pool and layout
        m_rtDescriptorPool = m_rtDSG.GeneratePool(device);
        m_rtDescriptorSetLayout = m_rtDSG.GenerateLayout(device);

        // Generate the descriptor set
        m_rtDescriptorSet =
            m_rtDSG.GenerateSet(device, m_rtDescriptorPool, m_rtDescriptorSetLayout);

        // Bind the actual resources into the descriptor set
        // Top-level acceleration structure
        VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo;
        descriptorAccelerationStructureInfo.sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
        descriptorAccelerationStructureInfo.pNext = nullptr;
        descriptorAccelerationStructureInfo.accelerationStructureCount = 1;

        //  Cast the acceleration structure to the ctype.
        m_ctypeAccelStructure = VkAccelerationStructureNV(m_rtBuilder.getAccelerationStructure());
        descriptorAccelerationStructureInfo.pAccelerationStructures = &m_ctypeAccelStructure;
           //.&m_topLevelAS.structure;

        m_rtDSG.Bind(m_rtDescriptorSet, 0, { descriptorAccelerationStructureInfo });

        //// Everything else  

        // Camera matrices
        VkDescriptorBufferInfo camInfo = {};
        camInfo.buffer = m_uniformBuffer.buffer;
        camInfo.offset = 0;
        camInfo.range = sizeof(UniformBufferObject);

        m_rtDSG.Bind(m_rtDescriptorSet, 2, { camInfo });

        // Vertex buffer, index buffer, materials
        std::vector< VkDescriptorBufferInfo> vertexInfo;
        std::vector< VkDescriptorBufferInfo> indexInfo;
        
        std::vector<VkDescriptorImageInfo> texInfo;

        for (int i = 0; i < m_geometryInstances.size(); ++i) {
            VkDescriptorBufferInfo vInfo = {};
            vInfo.buffer = m_geometryInstances[i].vertexBuffer.buffer;
            vInfo.offset = 0;
            vInfo.range = VK_WHOLE_SIZE;

            vertexInfo.push_back(vInfo);

            VkDescriptorBufferInfo iInfo = {};
            iInfo.buffer = m_geometryInstances[i].indexBuffer.buffer;
            iInfo.offset = 0;
            iInfo.range = VK_WHOLE_SIZE;

            indexInfo.push_back(iInfo);
        }


        for (int i = 7; i < m_sharedResourceVector.size(); ++i) {
            VkDescriptorImageInfo tInfo;
            // TODO: should this be color attachment optimal?
            tInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            tInfo.imageView = m_sharedResourceVector[i].texture.view;
            tInfo.sampler = m_sharedResourceVector[i].texture.sampler;

            texInfo.push_back(tInfo);
        }

        
        m_rtDSG.Bind(m_rtDescriptorSet, 3, vertexInfo );
        m_rtDSG.Bind(m_rtDescriptorSet, 4, indexInfo );

        m_rtDSG.Bind(m_rtDescriptorSet, 13, texInfo);


        // Material Buffer
        VkDescriptorBufferInfo materialInfo = {};
        materialInfo.buffer = m_materialBuffer.buffer;
        materialInfo.offset = 0;
        materialInfo.range = VK_WHOLE_SIZE;

        m_rtDSG.Bind(m_rtDescriptorSet, 14, { materialInfo });

        // Transform buffer
        VkDescriptorBufferInfo transformInfo = {};
        transformInfo.buffer = m_transformInverseTranspose.buffer;
        transformInfo.offset = 0;
        transformInfo.range = VK_WHOLE_SIZE;
        
        m_rtDSG.Bind(m_rtDescriptorSet, 7, { transformInfo });
        // Material buffer
        //VkDescriptorBufferInfo materialInfo = {};
        //materialInfo.buffer = m_matColorBuffer;
        //materialInfo.offset = 0;
        //materialInfo.range = VK_WHOLE_SIZE;
        //m_rtDSG.Bind(m_rtDescriptorSet, 5, { materialInfo });

        // Textures
        // No textures for now.
        //std::vector<VkDescriptorImageInfo> imageInfos;
        //for (size_t i = 0; i < m_textureSampler.size(); ++i)
        //{
        //    VkDescriptorImageInfo imageInfo = {};
        //    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        //    imageInfo.imageView = m_textureImageView[i];
        //    imageInfo.sampler = m_textureSampler[i];
        //    imageInfos.push_back(imageInfo);
        //}
        //if (!m_textureSampler.empty())
        //{
        //    m_rtDSG.Bind(m_rtDescriptorSet, 6, imageInfos);
        //}

        // Copy the bound resource handles into the descriptor set
        m_rtDSG.UpdateSetContents(device, m_rtDescriptorSet);
    }


    VkShaderModule createShaderModule(const std::vector<unsigned int>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(unsigned int); // This forces multiple of 4
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    // Convenient function to load the shaders (SPIR-V)
//
    std::vector<unsigned int> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        auto              fileSize = static_cast<size_t>(file.tellg());
        std::vector<unsigned int> buffer(fileSize / sizeof(unsigned int) + 1);

        file.seekg(0);
        file.read((char*)buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    // Prepare default shaders for each pipeline stage.
    void createRaytracingPipeline() {
        const std::vector<unsigned int>& raygenBytes = readFile("shader/raygen.spv");
        const std::vector<unsigned int>& missBytes = readFile("shader/miss.spv");
        const std::vector<unsigned int>& closesthitBytes = readFile("shader/closesthit.spv");
        const std::vector<unsigned int>& anyhitBytes = readFile("shader/anyhit.spv");
        const std::vector<unsigned int>& shadowMissBytes = readFile("shader/shadowMiss.spv");

        createRaytracingPipeline(raygenBytes, missBytes, closesthitBytes, anyhitBytes, shadowMissBytes);
    }

    // Create the raytracing pipeline, containing the handles and data for each
    // raytracing shader For each shader or hit group we retain its index, so that
    // they can be bound to the geometry in the shader binding table.
    void createRaytracingPipeline(const std::vector<unsigned int>& raygenBytes,
        const std::vector<unsigned int>& missBytes,
        const std::vector<unsigned int>& closesthitBytes,
        const std::vector<unsigned int>& anyhitBytes,
        const std::vector<unsigned int>& shadowMissBytes)
    {

        nv_helpers_vk::RayTracingPipelineGenerator pipelineGen;
        // We use only one ray generation, that will implement the camera model
        VkShaderModule rayGenModule = createShaderModule(raygenBytes);
        m_rayGenIndex = pipelineGen.AddRayGenShaderStage(rayGenModule);
        // The first miss shader is used to look-up the environment in case the rays
        // from the camera miss the geometry
        VkShaderModule missModule = createShaderModule(missBytes);
        m_missIndex = pipelineGen.AddMissShaderStage(missModule);

        // The second miss shader is invoked when a shadow ray misses the geometry. It
        // simply indicates that no occlusion has been found
        VkShaderModule missShadowModule = createShaderModule(shadowMissBytes);
        m_shadowMissIndex = pipelineGen.AddMissShaderStage(missShadowModule);

        // The first hit group defines the shaders invoked when a ray shot from the
        // camera hit the geometry. In this case we only specify the closest hit
        // shader, and rely on the build-in triangle intersection and pass-through
        // any-hit shader. However, explicit intersection and any hit shaders could be
        // added as well.
        m_hitGroupIndex = pipelineGen.StartHitGroup();
        VkShaderModule closestHitModule = createShaderModule(closesthitBytes);
        pipelineGen.AddHitShaderStage(closestHitModule, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
        VkShaderModule anyHitModule = createShaderModule(anyhitBytes);
        pipelineGen.AddHitShaderStage(anyHitModule, VK_SHADER_STAGE_ANY_HIT_BIT_NV);
        pipelineGen.EndHitGroup();

        // The second hit group defines the shaders invoked when a shadow ray hits the
        // geometry. For simple shadows we do not need any shader in that group: we will rely on
        // initializing the payload and update it only in the miss shader
        m_shadowHitGroupIndex = pipelineGen.StartHitGroup();
        pipelineGen.EndHitGroup();


        // The raytracing process now can only shoot rays from the camera, hence a
        // recursion level of 1. This number should be kept as low as possible for
        // performance reasons. Even recursive raytracing should be flattened into a
        // loop in the ray generation to avoid deep recursion.
        pipelineGen.SetMaxRecursionDepth(2);

        // Generate the pipeline
        pipelineGen.Generate(device, m_rtDescriptorSetLayout, &m_rtPipeline,
            &m_rtPipelineLayout);

        vkDestroyShaderModule(device, rayGenModule, nullptr);
        vkDestroyShaderModule(device, missModule, nullptr);
        vkDestroyShaderModule(device, closestHitModule, nullptr);
        vkDestroyShaderModule(device, missShadowModule, nullptr);
        vkDestroyShaderModule(device, anyHitModule, nullptr);
    }

    // Create the shader binding table, associating the geometry to the indices of the shaders in the
    // ray tracing pipeline
    void createShaderBindingTable()
    {
        // Add the entry point, the ray generation program
        m_sbtGen.AddRayGenerationProgram(m_rayGenIndex, {});
        // Add the miss shader for the camera rays
        m_sbtGen.AddMissProgram(m_missIndex, {});

        // Add the miss shader for the shadow rays
        m_sbtGen.AddMissProgram(m_shadowMissIndex, {});

        // For each instance, we will have 1 hit group for the camera rays.
        // When setting the instances in the top-level acceleration structure we indicated the index
        // of the hit group in the shader binding table that will be invoked.

        // Add the hit group defining the behavior upon hitting a surface with a camera ray
        m_sbtGen.AddHitGroup(m_hitGroupIndex, {});

        // Add the hit group defining the behavior upon hitting a surface with a shadow ray
        m_sbtGen.AddHitGroup(m_shadowHitGroupIndex, {});

        // Compute the required size for the SBT
        VkDeviceSize shaderBindingTableSize = m_sbtGen.ComputeSBTSize(m_raytracingProperties);

        //m_uniformBuffer = context.createBuffer(vk::BufferUsageFlagBits::eUniformBuffer,
        //    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        //    bufferSize);

        m_shaderBindingTableBuffer = context.createBuffer(vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible, shaderBindingTableSize);
        // Allocate mappable memory to store the SBT
        //nv_helpers_vk::createBuffer(context.physicalDevice, device, shaderBindingTableSize,
        //    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &m_shaderBindingTableBuffer,
        //    &m_shaderBindingTableMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        // Generate the SBT using mapping. For further performance a staging buffer should be used, so
        // that the SBT is guaranteed to reside on GPU memory without overheads.
        m_sbtGen.Generate(device, m_rtPipeline, m_shaderBindingTableBuffer.buffer,
            m_shaderBindingTableBuffer.memory);
    }

    void initRayTracing() {
        // Query the values of shaderHeaderSize and maxRecursionDepth in current implementation
        m_raytracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
        m_raytracingProperties.pNext = nullptr;
        m_raytracingProperties.maxRecursionDepth = 0;
        m_raytracingProperties.shaderGroupHandleSize = 0;
        VkPhysicalDeviceProperties2 props;
        props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props.pNext = &m_raytracingProperties;
        props.properties = {};
        vkGetPhysicalDeviceProperties2(context.physicalDevice, &props);

        // New RT builder
        m_rtBuilder.setup(context.device, context.physicalDevice, 0);
    }

    void updateRTUniformBuffer(int width, int height) {
        UniformBufferObject ubo = {};
        ubo.model = Matrix4::identity();
        ubo.modelIT = ubo.model.inverse().transpose();

        ubo.view = m_cameraFrame;

        const float aspectRatio = (float)width / (float)height;// m_framebufferSize.width / static_cast<float>(m_framebufferSize.height);
        ubo.proj = minimalGLPerspective((float)width, (float)height, -0.1f, -256.0f, (float)toRadians(60.0f));
        ubo.proj[1][1] *= -1;  // Inverting Y for Vulkan

        // Ray Tracing tutorial
        ubo.viewInverse = ubo.view.transpose();
        ubo.projInverse = ubo.proj.transpose().inverse();

        void* data;
        vkMapMemory(device, m_uniformBuffer.memory, 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, m_uniformBuffer.memory);

        // Object transforms
        // First draft, not optimized at all, just checking for correctness.
        {
            void* data;
            vkMapMemory(device, m_transformInverseTranspose.memory, 0, sizeof(G3D::Matrix4) * m_geometryInstances.size(), 0, &data);
            for (int i = 0; i < m_geometryInstances.size(); ++i) {
                G3D::Matrix4* dataMat4 = (G3D::Matrix4*)data;
                G3D::Matrix4 transformIT = m_geometryInstances[i].transform.inverse().transpose();
                memcpy(&dataMat4[i], &transformIT, sizeof(G3D::Matrix4));
            }
            vkUnmapMemory(device, m_transformInverseTranspose.memory);
        }

        // Propagate transform updates to the AS.
        updateTopLevelAS();

        // Materials
        {
            void* data;
            size_t size = sizeof(Material);
            vkMapMemory(device, m_materialBuffer.memory, 0, sizeof(Material) * m_materials.size(), 0, &data);
            memcpy(data, &m_materials[0], sizeof(Material) * m_materials.size());
            //for (int i = 0; i < m_materials.size(); ++i) {
            //    memcpy(&(((Material*)data)[i]), &m_materials[i], sizeof(Material));
            //}
            vkUnmapMemory(device, m_materialBuffer.memory);
        }

    }

    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = context.getCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(context.queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(context.queue);

        vkFreeCommandBuffers(device, context.getCommandPool(), 1, &commandBuffer);
    }

    /*
        Submit command buffer to a queue and wait for fence until queue operations have been finished
    */
    void submitWork(const vk::CommandBuffer& cmdBuffer, const vk::Queue queue) {
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

		std::vector<vk::PipelineStageFlags> stageFlags;
        bool needsInitialization = m_completeSemaphores.size() == 0;
		for (SharedResources& resource : m_sharedResourceVector) {
			stageFlags.push_back(vk::PipelineStageFlagBits::eBottomOfPipe);
			if (needsInitialization) {
				// Compact complete and ready semaphores into a contiguous array.
				m_completeSemaphores.push_back(resource.semaphores.glComplete);
				m_readySemaphores.push_back(resource.semaphores.glReady);
			}
		}
		submitInfo.pWaitDstStageMask = &stageFlags[0];

        submitInfo.waitSemaphoreCount = (uint32_t)m_sharedResourceVector.size();
        submitInfo.pWaitSemaphores = &m_completeSemaphores[0];

        submitInfo.signalSemaphoreCount = (uint32_t)m_sharedResourceVector.size();
        submitInfo.pSignalSemaphores = &m_readySemaphores[0];

        // This would never have been correct because we create the
        // fence but don't pass it to the submit call.
        //vk::Fence fence = device.createFence({});
        queue.submit({ submitInfo }, {});
        //device.waitForFences(fence, true, UINT64_MAX);
        //device.destroy(fence);
    }

    void updateOutputBuffers(VkImageView lambertian, VkImageView glossy, VkImageView emissive, VkImageView shadingNormal, VkImageView position)
    {
        // Ray buffer
        VkDescriptorImageInfo outputBufferImageInfo;
        outputBufferImageInfo.sampler = nullptr;
        outputBufferImageInfo.imageView = lambertian;
        outputBufferImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        m_rtDSG.Bind(m_rtDescriptorSet, 8, { outputBufferImageInfo });
        outputBufferImageInfo.imageView = glossy;
        m_rtDSG.Bind(m_rtDescriptorSet, 9, { outputBufferImageInfo });
        outputBufferImageInfo.imageView = emissive;
        m_rtDSG.Bind(m_rtDescriptorSet, 10, { outputBufferImageInfo });
        outputBufferImageInfo.imageView = shadingNormal;
        m_rtDSG.Bind(m_rtDescriptorSet, 11, { outputBufferImageInfo });
        outputBufferImageInfo.imageView = position;
        m_rtDSG.Bind(m_rtDescriptorSet, 12, { outputBufferImageInfo });

        // Copy the bound resource handles into the descriptor set
        m_rtDSG.UpdateSetContents(device, m_rtDescriptorSet);
    }

	void updateRayBuffers(VkImageView origins, VkImageView directions)
	{
		// Ray buffer
		VkDescriptorImageInfo rayBufferImageInfo;
		rayBufferImageInfo.sampler = nullptr;
		rayBufferImageInfo.imageView = origins;
		rayBufferImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		m_rtDSG.Bind(m_rtDescriptorSet, 5, { rayBufferImageInfo });

		rayBufferImageInfo.imageView = directions;
		m_rtDSG.Bind(m_rtDescriptorSet, 6, { rayBufferImageInfo });

		// Copy the bound resource handles into the descriptor set
		m_rtDSG.UpdateSetContents(device, m_rtDescriptorSet);
	}

    void doRendering(
        int width,
        int height,
        int rayOrigins,
        int rayDirections,
        int lambertian,
        int glossy,
        int emissive,
        int shadingNormal,
        int position) {

        // TODO: TEST!!!
        if (!framebuffer) {
            // TODO: create this in a different spot.
            vk::FramebufferCreateInfo framebufferCreateInfo;
            framebufferCreateInfo.renderPass = renderPass;
            framebufferCreateInfo.attachmentCount = 0;
            framebufferCreateInfo.pAttachments = nullptr;
            framebufferCreateInfo.width = width;
            framebufferCreateInfo.height = height;
            framebufferCreateInfo.layers = 1;
            framebuffer = device.createFramebuffer(framebufferCreateInfo);
        }

        updateRTUniformBuffer(width, height);

        // Begin recording to the main loop command buffer.
        // If nothing about the render passes is changing structurally
        // we don't have to re-record them every frame (and it is slightly
        // faster not to). We do so here to avoid bugs in development.
        mainLoopCommandBuffer.begin(vk::CommandBufferBeginInfo{});

        vk::ClearValue clearValues[2];
        // Gross
        clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.2f, 1.0f }));
        clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };


        VkClearValue clearColor = { 0.0f, 0.5f, 0.0f, 1.0f };

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        // All these images have to be transitioned every frame because they come from GL.
        // Read only: ray origin and direction
		for (int i = 0; i < 2; ++i) {
			nv_helpers_vk::imageBarrier(mainLoopCommandBuffer, m_sharedResourceVector[i].texture.image, subresourceRange, 0,
				VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL);
		}

        //Write only: lambertian, glossy, emissive, normal, position.
        for (int i = 2; i < 7; ++i) {
            nv_helpers_vk::imageBarrier(mainLoopCommandBuffer, m_sharedResourceVector[i].texture.image, subresourceRange, 0,
                VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL);
        }

        // All textures to read from.
        for (int i = 7; i < m_sharedResourceVector.size(); ++i) {
            nv_helpers_vk::imageBarrier(mainLoopCommandBuffer, m_sharedResourceVector[i].texture.image, subresourceRange, 0,
                VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL);
        }

        // The first 7 texture view in the shared resource buffer are used for ray buffers
        // and GBuffer outputs.
		updateRayBuffers(m_sharedResourceVector[rayOrigins].texture.view, m_sharedResourceVector[rayDirections].texture.view);
        updateOutputBuffers(m_sharedResourceVector[lambertian].texture.view,  // lambertian
                            m_sharedResourceVector[glossy].texture.view,  // glossy
                            m_sharedResourceVector[emissive].texture.view,  // emissive
                            m_sharedResourceVector[shadingNormal].texture.view,  // shadingNormal
                            m_sharedResourceVector[position].texture.view); // position

        vk::RenderPassBeginInfo renderPassBeginInfo{ renderPass, framebuffer, vk::Rect2D{ vk::Offset2D{}, vk::Extent2D{uint32_t(width), uint32_t(height)} }, 2, clearValues };
        mainLoopCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        vkCmdBindPipeline(mainLoopCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_rtPipeline);

        vkCmdBindDescriptorSets(mainLoopCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
            m_rtPipelineLayout, 0, 1, &m_rtDescriptorSet,
            0, nullptr);

        VkDeviceSize rayGenOffset = m_sbtGen.GetRayGenOffset();
        VkDeviceSize missOffset = m_sbtGen.GetMissOffset();
        VkDeviceSize missStride = m_sbtGen.GetMissEntrySize();
        VkDeviceSize hitGroupOffset = m_sbtGen.GetHitGroupOffset();
        VkDeviceSize hitGroupStride = m_sbtGen.GetHitGroupEntrySize();

        mainLoopCommandBuffer.endRenderPass();

        // This command adds the full tray tracing we created to the command buffer
        vkCmdTraceRaysNV(mainLoopCommandBuffer, m_shaderBindingTableBuffer.buffer, rayGenOffset,
            m_shaderBindingTableBuffer.buffer, missOffset, missStride,
            m_shaderBindingTableBuffer.buffer, hitGroupOffset, hitGroupStride,
            VK_NULL_HANDLE, 0, 0, width,
            height, 1);

        mainLoopCommandBuffer.end();

        submitWork(mainLoopCommandBuffer, context.queue);

        // This call was used only to signal the glReady semaphore, which is
        // now done in submitWork() above.
        //m_sharedResource.transitionToGl(context.queue);
    }

    int allocateVulkanInteropTexture(const int width, const int height, const int mipLevels, bool buffer, uint64& allocatedMemory) {
        SharedResources resources;
        // TODO: support other texture formats
        // TODO: more parameters about the texture sampling method...or maybe just mipmapping?
        resources.init(context, width, height, vk::Format::eR32G32B32A32Sfloat, mipLevels, buffer);
        allocatedMemory = (uint64)resources.texture.allocSize;
        m_sharedResourceVector.push_back(resources);
        return (int)m_sharedResourceVector.size() - 1;
    }

    _VKBVH() {

        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "waveVK.lib";
        appInfo.pEngineName = "Vulkan Ray Tracing";
        appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

        /*
            Vulkan instance creation (without surface extensions)
        */
        context.requireExtensions({
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,    //
            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,  //
            });

        context.requireDeviceExtensions({
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,            //
                VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,     //
                VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,  //
#if defined(WIN32)
                VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,    //
                VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,  //

#else
                VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,    //
                VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,  //
#endif
                VK_NV_RAY_TRACING_EXTENSION_NAME,
                VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
                VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
            });

        vk::InstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.pApplicationInfo = &appInfo;

#if _DEBUG
        context.setValidationEnabled(true);
#endif
        context.createInstance(instanceCreateInfo);
        context.createDevice();

        {
            // Debugging;
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(context.physicalDevice, &features);


            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(context.physicalDevice, &props);
        }

        /*
            Create framebuffer attachments
        */
        static const vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;
        static const vk::Format depthFormat = context.getSupportedDepthFormat();

        //static const int SHARED_BUFFERS = 7;

		// Ray Origin and Direction Buffers
		//m_sharedResourceVector.resize(SHARED_BUFFERS);
        //
		//m_sharedResourceVector[0].init(context, width, height, vk::Format::eR32G32B32A32Sfloat, true);
		//m_sharedResourceVector[1].init(context, width, height, vk::Format::eR32G32B32A32Sfloat, true);

        // Initialize all shared buffers
        // Lambertian, Glossy, Emissive, Shading Normal, Position
        //for (int i = 2; i < SHARED_BUFFERS; ++i) {
        //    m_sharedResourceVector[i].init(context, width, height, vk::Format::eR32G32B32A32Sfloat, true);
        //}

        /*
            Create renderpass
        */
        {
            std::array<vk::AttachmentDescription, 1> attchmentDescriptions = {};
            // Color attachment
            attchmentDescriptions[0].format = colorFormat;
            attchmentDescriptions[0].loadOp = vk::AttachmentLoadOp::eClear;
            attchmentDescriptions[0].storeOp = vk::AttachmentStoreOp::eStore;
            attchmentDescriptions[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            attchmentDescriptions[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            attchmentDescriptions[0].initialLayout = vk::ImageLayout::eUndefined;

            // "Cannot use VKImage...with specific layout VK_IMAGE_LAYOUT_GENERAL that doesn't 
            // match the previous known layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL"
            //attchmentDescriptions[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
            attchmentDescriptions[0].finalLayout = vk::ImageLayout::eGeneral;

            // "Cannot use VKImage...with specific layout VK_IMAGE_LAYOUT_GENERAL that doesn't 
            // match the previous known layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL"
            //vk::AttachmentReference colorReference = { 0, vk::ImageLayout::eColorAttachmentOptimal };
            vk::AttachmentReference colorReference = { 0, vk::ImageLayout::eGeneral };

            vk::SubpassDescription subpassDescription;
            subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
            subpassDescription.colorAttachmentCount = 0;
            subpassDescription.pColorAttachments = nullptr;

            // Use subpass dependencies for layout transitions
            std::array<vk::SubpassDependency, 2> dependencies;

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
            dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
            dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
            dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
            dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
            dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
            dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

            // Create the actual renderpass
            vk::RenderPassCreateInfo renderPassInfo;
            renderPassInfo.attachmentCount = 0;
            renderPassInfo.pAttachments = nullptr;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();
            renderPass = device.createRenderPass(renderPassInfo);
        }

        // Create the uniform buffer
        {
            VkDeviceSize bufferSize = sizeof(UniformBufferObject);
            m_uniformBuffer = context.createBuffer(vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                bufferSize);
        }

        mainLoopCommandBuffer = context.allocateCommandBuffers(1)[0];

        //Ray tracing
        initRayTracing();
    }

    void finalizeAccelerationStructure(const std::vector<unsigned int>& raygenBytes,
        const std::vector<unsigned int>& missBytes,
        const std::vector<unsigned int>& closesthitBytes,
        const std::vector<unsigned int>& anyhitBytes,
        const std::vector<unsigned int>& shadowMissBytes) {

        // Testing new Builder from updated tutorial.
        createBottomLevelAS();
        createTopLevelAS();

        createInstanceTransformBuffer();
        createMaterialBuffer();

        createRaytracingDescriptorSet();
        createRaytracingPipeline(raygenBytes, missBytes, closesthitBytes, anyhitBytes, shadowMissBytes);
        createShaderBindingTable();
    }

    ~_VKBVH() {

        for (SharedResources& r : m_sharedResourceVector) {
            r.destroy();
        }
        depthAttachment.destroy();
        m_uniformBuffer.destroy();

        m_materialBuffer.destroy();
        m_transformInverseTranspose.destroy();

        for (int i = 0; i < m_geometryInstances.size(); ++i) {
            m_geometryInstances[i].vertexBuffer.destroy();
            m_geometryInstances[i].indexBuffer.destroy();
        }

        device.destroy(renderPass);
        device.destroy(framebuffer);
        device.destroy(descriptorSetLayout);

        m_rtBuilder.destroy();
        
        device.destroy(m_rtDescriptorPool);
        device.destroy(m_rtDescriptorSetLayout);

        m_shaderBindingTableBuffer.destroy();

        device.destroy(m_rtPipelineLayout);
        device.destroy(m_rtPipeline);

        context.destroy();
    }

	void* glReadyHandle(int index) {
		return m_sharedResourceVector[index].handles.glReady;
	}

	void* glCompleteHandle(int index) {
		return m_sharedResourceVector[index].handles.glComplete;
	}

	void* glMemoryHandle(int index) {
		return m_sharedResourceVector[index].handles.memory;
	}

    void doVulkanRendering(
        int width,
        int height,
        int rayOrigins,
        int rayDirections,
        int lambertian,
        int glossy,
        int emissive,
        int shadingNormal,
        int position) {
        doRendering(width, height, rayOrigins, rayDirections, lambertian, glossy, emissive, shadingNormal, position);
    }

};

//////////////////////////////////////////////////////////////////////////////////
// Trampoline BVH to the private _BVH API

    int VKBVH::createMaterial(
        const bool   hasAlpha,
        const int    textureNIndex,
        const float* scale_n,
        const float* bias_n,
        const int    texture0Index,
        const float* scale_0,
        const float* bias_0,
        const int    texture1Index,
        const float* scale_1,
        const float* bias_1,
        const int    texture2Index,
        const float* scale_2,
        const float* bias_2,
        const int    texture3Index,
        const float* scale_3,
        const float* bias_3,
        const float  materialConstant,
        uint8         flags) {
        return _bvh->createMaterial(
            hasAlpha,
            textureNIndex,
            scale_n,
            bias_n,
            texture0Index,
            scale_0,
            bias_0,
            texture1Index,
            scale_1,
            bias_1,
            texture2Index,
            scale_2,
            bias_2,
            texture3Index,
            scale_3,
            bias_3,
            materialConstant,
             flags);
    }

    int VKBVH::allocateVulkanInteropTexture(const int width, const int height, const int mipLevels, bool buffer, uint64& allocatedMemory) {
        return _bvh->allocateVulkanInteropTexture(width, height, mipLevels, buffer, allocatedMemory);
    }

	void* VKBVH::glReadyHandle(int index) {
		return _bvh->glReadyHandle(index);
	}

	void* VKBVH::glCompleteHandle(int index) {
		return _bvh->glCompleteHandle(index);
	}

	void* VKBVH::glMemoryHandle(int index) {
		return _bvh->glMemoryHandle(index);
	}

    void VKBVH::doVulkanRendering(
        int width, 
        int height,
        int rayOrigins,
        int rayDirections,
        int lambertian,
        int glossy,
        int emissive,
        int shadingNormal,
        int position) {
        _bvh->doVulkanRendering(width, height, rayOrigins, rayDirections, lambertian, glossy, emissive, shadingNormal, position);
    }
    void VKBVH::updateCameraFrame(const Matrix4& m) {
        _bvh->updateCameraFrame(m);
    }

    void VKBVH::finalizeAccelerationStructure(const std::vector<unsigned int>& raygenBytes,
        const std::vector<unsigned int>& missBytes,
        const std::vector<unsigned int>& closesthitBytes,
        const std::vector<unsigned int>& anyhitBytes,
        const std::vector<unsigned int>& shadowMissBytes) {
        _bvh->finalizeAccelerationStructure(raygenBytes, missBytes, closesthitBytes, anyhitBytes, shadowMissBytes);
    }

    int VKBVH::createGeometry(const std::vector<Vertex>&            vertices,
                               int                                  numVertices,
                               const std::vector<int>&              indices,
                               const int                            materialIndex,
                               int                                  numIndices,
                               const float                          geometryToWorldRowMajorMatrix[16],
                               bool                                 twoSided,
                               int                                  transformID) 
    {
        return _bvh->createGeometryInstance(vertices, numVertices, indices, numIndices, materialIndex, geometryToWorldRowMajorMatrix);
    }

    void VKBVH::setTransform(int geometryIndex, const float geometryToWorldRowMajorMatrix[16])
    {
        return _bvh->setTransform(geometryIndex, geometryToWorldRowMajorMatrix);
    }

    VKBVH::VKBVH() {
        _bvh = new _VKBVH();
    }

    VKBVH::~VKBVH() {
        delete _bvh;
    }
}