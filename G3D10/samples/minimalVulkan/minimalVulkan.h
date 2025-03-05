/**
  \file minimalVulkan.h

    Minimal Vulkan Example. Based on tutorial from: https://vulkan-tutorial.com
 */
#pragma once
#include "matrix.h"


#ifdef __APPLE__
#   define _OSX
#elif defined(_WIN64)
#   ifndef _WINDOWS
#       define _WINDOWS
#   endif
#elif defined(__linux__)
#   define _LINUX
#endif

#define NOMINMAX

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>
#include <limits>

#include <stdexcept>
#include <functional>
#include <set>
#include <chrono>


#define GLFW_INCLUDE_VULKAN


#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#ifdef _WINDOWS
#   include <GL/wglew.h>
#elif defined(_LINUX)
#   include <GL/xglew.h>
#endif
#include <GLFW/glfw3.h> 

#ifdef _WINDOWS
 // vulkan
#   pragma comment(lib, "vulkan-1")
#ifdef _DEBUG
#   pragma comment(lib, "glslangd")
#   pragma comment(lib, "SPIRVd")
#   pragma comment(lib, "SPVRemapperd")
#   pragma comment(lib, "OSDependentd")
#   pragma comment(lib, "OGLCompilerd")
#else
#   pragma comment(lib, "glslang")
#   pragma comment(lib, "SPIRV")
#   pragma comment(lib, "SPVRemapper")
#   pragma comment(lib, "OSDependent")
#   pragma comment(lib, "OGLCompiler")
#endif
#   pragma comment(lib, "glew")
#   pragma comment(lib, "glfw")

#ifdef _DEBUG
    // Some of the support libraries are always built in Release.
    // Make sure the debug runtime library is linked in
#   pragma comment(linker, "/NODEFAULTLIB:MSVCRT.LIB")
#   pragma comment(linker, "/NODEFAULTLIB:MSVCPRT.LIB")
#endif

#endif

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

struct UniformBufferObject {
    Matrix4x4 model;
    Matrix4x4 invTransModel;
    Matrix4x4 view;
    Matrix4x4 proj;
    Matrix4x4 modelViewProj;
};

struct SkyUBO {
    Vector4     light;
    Vector4     resolution;
    Matrix4x4   cameraToWorldMatrix;
    Matrix4x4   invProjectionMatrix;
};

namespace Cube {
    const float position[][3] = { -.5f, .5f, -.5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, -.5f, .5f };
    const float normal[][3] = { 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f };
    const float tangent[][4] = { 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f };
    const float texCoord[][2] = { 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f };
    const int   index[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23 };
};

struct Vertex {
    Vector3 pos;
    Vector3 color;
    Vector3 normal;
    Vector4 tangent;
    Vector2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions; attributeDescriptions.resize(5);

        // Position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        // Color
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        // Normal
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        // Tangent
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, tangent);

        // Texcoord
        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool isComplete() {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};



const float vExt = 0.5f;

std::vector<Vertex> vertexData = {
    // Back Z-plane
    {{-vExt, -vExt, -vExt}, {1.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f}, {1.0f, 1.0f}},   // bottom left   0
    {{vExt,  -vExt, -vExt}, {0.0f, 1.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f}, {0.0f, 1.0f}},  // bottom right  1
    {{vExt,  vExt,  -vExt}, {0.0f, 0.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f}, {0.0f, 0.0f}}, // top right     2
    {{-vExt, vExt,  -vExt}, {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f}, {1.0f, 0.0f}},  // top left      3

    // Front Z-plane
    {{-vExt, -vExt,  vExt}, {1.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f}, {0.0f, 1.0f}},   // bottom left    4
    {{vExt,  -vExt,  vExt}, {0.0f, 1.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},  {1.0f, 1.0f}},  // bottom right   5
    {{vExt,  vExt,   vExt}, {0.0f, 0.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},   {1.0f, 0.0f}}, // top right      6
    {{-vExt, vExt,   vExt}, {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},  {0.0f, 0.0f}},   // top left       7

    // Back X-plane
    {{-vExt, -vExt, -vExt}, {1.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f}, {0.0f, 1.0f}},   // bottom left   0
    {{vExt,  -vExt, -vExt}, {0.0f, 1.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},  {1.0f, 1.0f}},  // bottom right  1
    {{vExt,  vExt,  -vExt}, {0.0f, 0.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},   {1.0f, 0.0f}}, // top right     2
    {{-vExt, vExt,  -vExt}, {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},  {0.0f, 0.0f}},  // top left      3

    // Front X-plane
    {{-vExt, -vExt,  vExt}, {1.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f}, {1.0f, 1.0f}},   // bottom left    4
    {{vExt,  -vExt,  vExt}, {0.0f, 1.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},  {0.0f, 1.0f}},  // bottom right   5
    {{vExt,  vExt,   vExt}, {0.0f, 0.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},   {0.0f, 0.0f}}, // top right      6
    {{-vExt, vExt,   vExt}, {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},  {1.0f, 0.0f}},   // top left       7

    // Back Y-plane
    {{-vExt, -vExt, -vExt}, {1.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f, 0.0f}, {1.0f, 1.0f}},   // bottom left   0
    {{vExt,  -vExt, -vExt}, {0.0f, 1.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f, 0.0f},  {0.0f, 1.0f}},  // bottom right  1
    {{vExt,  vExt,  -vExt}, {0.0f, 0.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f, 0.0f},   {1.0f, 0.0f}}, // top right     2
    {{-vExt, vExt,  -vExt}, {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f, 0.0f},  {0.0f, 0.0f}},  // top left      3

    // Front Y-plane
    {{-vExt, -vExt,  vExt}, {1.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f, 0.0f}, {0.0f, 1.0f}},   // bottom left    4
    {{vExt,  -vExt,  vExt}, {0.0f, 1.0f, 0.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f, 0.0f},  {1.0f, 1.0f}},  // bottom right   5
    {{vExt,  vExt,   vExt}, {0.0f, 0.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f, 0.0f},   {1.0f, 1.0f}}, // top right      6
    {{-vExt, vExt,   vExt}, {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f, 0.0f},  {0.0f, 1.0f}}   // top left       7

};

std::vector<uint16_t> indexData = {
    // +Z face
    7, 4, 6,
    6, 4, 5,

    // -Z face
    2, 1, 3,
    3, 1, 0,

    // +X face
    6 + 8, 5 + 8, 2 + 8,
    2 + 8, 5 + 8, 1 + 8,

    // -X face
    3 + 8, 0 + 8, 7 + 8,
    7 + 8, 0 + 8, 4 + 8,

    // +Y face
    3 + 16, 7 + 16, 2 + 16,
    2 + 16, 7 + 16, 6 + 16,

    // -Y face
    4 + 16, 0 + 16, 5 + 16,
    5 + 16, 0 + 16, 1 + 16
};

// List of required extensions
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME // required to render to the screen
};

const int MAX_FRAMES_IN_FLIGHT = 2;

// Vulkan Members
VkInstance m_instance;
VkDebugUtilsMessengerEXT callback;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSurfaceKHR surface;
// Swap chain members
VkSwapchainKHR swapChain;

// These are, essentially, the hardware framebuffers.
std::vector<VkImage> swapChainImages;
std::vector<VkImageView> swapChainImageViews;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
std::vector<VkFramebuffer> swapChainFramebuffers;

// Graphics Pipeline
VkRenderPass renderPass;
VkRenderPass skyRenderPass;
VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout pipelineLayout;
bool pipelineLayoutCreated = false;
VkPipeline graphicsPipeline;

VkPipeline skyPipeline;

//Vertices & Indices
VkBuffer vertexBuffer; //Handle to buffer memory, allocated in VkDeviceMemory below
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;

std::vector<VkBuffer> uniformBuffers;
std::vector<VkDeviceMemory> uniformBuffersMemory;

std::vector<VkBuffer> skyUniformBuffers;
std::vector<VkDeviceMemory> skyUniformBuffersMemory;

//Image & Views
VkImage textureImage;
VkDeviceMemory textureImageMemory;
VkImageView textureImageView;

// Depth
VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;

//Texture sampler
VkSampler textureSampler;

// Drawing
VkCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;

//Descriptors
VkDescriptorPool descriptorPool;
std::vector<VkDescriptorSet> descriptorSets;

// Semaphores
// imageAvailable and renderFinished semaphores for all possible frames in flight.
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence>     inFlightFences;
int currentFrame = 0;

bool framebufferResized = false;

const int WIDTH = 1280;
const int HEIGHT = 720;

const std::string TEXTURE_PATH = "vulkan2.bmp";

const std::string mainVertexShaderFile = "min.vert";
const std::string mainPixelShaderFile = "min.frag";

const std::string skyVertexShaderFile = "sky.vert";
const std::string skyPixelShaderFile = "sky.frag";

GLFWwindow* window;

// Reads in shaders from bytes, and specifies shader stages (vertex and fragment for this app).
// Allows input of vertex data and assembly instructions (triangle list, triangle strip, etc.)
// Specifies a viewport and a region of pixels on framebuffer to draw on (scissor).
// Sets rasterizer options, like enabling the depth test or culling of backfaces or blend mode.

// Abstracted into a function to allow shader recompilation.
void createGraphicsPipeline(const std::string& vertexShaderFilename,
    const std::string& pixelShaderFilename,
    VkPipeline& gfxPipeline,
    bool fullscreenPass = false);


// Shader compilation using glslang from this guide: https://forestsharp.com/glslang-cpp/
bool glslangInitialized = false;
void compileGLSLShader(const char* inputFilename, std::vector<unsigned int> resultBytes);

VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void recordCommandBuffers();

// For depth texture formats
VkFormat findDepthFormat();
bool hasStencilComponent(VkFormat format);
VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

// Helper function for general buffer creation. Handles buffer generation and memory allocation.
// In a real application, we wouldn't want to invoke vkAllocateMemory for every buffer, as this could
// hit the physical device limit of separate memory allocation.
// Could be optimized by creating a custom memory allocator to combine many objects into a single allocation, 
// or use the VulkanMemoryAllocator provided by GPUOpen.
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

// Copying works like drawing, and requires the use of command buffers.
// Could be optimized by allocating a separate command pool to allow the implementation to optimize.
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

VkCommandBuffer beginSingleTimeCommands();
void endSingleTimeCommands(VkCommandBuffer commandBuffer);

bool checkValidationLayerSupport();

// Get the required extensions from GLFW
std::vector<const char*> getRequiredExtensions();

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);


// Check if a device is suitable for your application
// Note that you could also implement a rating of devices
// based on their supported properties, but for this tutorial there's no need.
bool isDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

bool checkDeviceExtensionSupport(VkPhysicalDevice device);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

VkShaderModule createShaderModule(const std::vector<char>& code);
VkShaderModule createShaderModule(const uint32_t* data, size_t numBytes);


/** Loads a 24- or 32-bit BMP file into memory. */
void loadBMP(const std::string& filename, int& width, int& height, int& channels, std::vector<std::uint8_t>& data) {
    std::fstream hFile(filename.c_str(), std::ios::in | std::ios::binary);
    if (!hFile.is_open()) { throw std::invalid_argument("Error: File Not Found."); }

    hFile.seekg(0, std::ios::end);
    size_t len = hFile.tellg();
    hFile.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> header(len);
    hFile.read(reinterpret_cast<char*>(header.data()), 54);

    if ((header[0] != 'B') && (header[1] != 'M')) {
        hFile.close();
        throw std::invalid_argument("Error: File is not a BMP.");
    }

    if ((header[28] != 24) && (header[28] != 32)) {
        hFile.close();
        throw std::invalid_argument("Error: File is not uncompressed 24 or 32 bits per pixel.");
    }

    const short bitsPerPixel = header[28];
    channels = bitsPerPixel / 8;
    width = header[18] + (header[19] << 8);
    height = header[22] + (header[23] << 8);
    std::uint32_t offset = header[10] + (header[11] << 8);
    std::uint32_t size = ((width * bitsPerPixel + 31) / 32) * 4 * height;
    data.resize(size);

    hFile.seekg(offset, std::ios::beg);
    hFile.read(reinterpret_cast<char*>(data.data()), size);
    hFile.close();

    // Flip the y axis
    std::vector<std::uint8_t> tmp;
    const size_t rowBytes = width * channels;
    tmp.resize(rowBytes);
    for (int i = height / 2 - 1; i >= 0; --i) {
        const int j = height - 1 - i;
        // Swap the rows
        memcpy(tmp.data(), &data[i * rowBytes], rowBytes);
        memcpy(&data[i * rowBytes], &data[j * rowBytes], rowBytes);
        memcpy(&data[j * rowBytes], tmp.data(), rowBytes);
    }

    // Convert [A]BGR format to [A]RGB format
    // Photoshop outputs in this format
    if (channels == 4) {
        std::uint32_t* p = reinterpret_cast<std::uint32_t*>(data.data());
        for (int i = width * height - 1; i >= 0; --i) {
            const unsigned int x = p[i];
            p[i] = (((x >> 24) & 0xFF) < 24) | (((x >> 16) & 0xFF)) | (((x >> 8) & 0xFF) << 8) | ((x & 0xFF) << 16);
        }
    }
    else {
        // BGR
        for (int i = (width * height - 1) * 3; i >= 0; i -= 3) {
            std::swap(data[i], data[i + 2]);
        }
    }
}