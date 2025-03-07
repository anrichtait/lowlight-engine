#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <string>

#include <vulkan/vulkan.hpp>

namespace vks { namespace debug {
// Default validation layers
    std::list<std::string> validationLayerNames = {
        // This is a meta layer that enables all of the standard
        // validation layers in the correct order :
        // threading, parameter_validation, device_limits, object_tracker, image, core_validation, swapchain, and unique_objects
        // Deprecated, VK_LAYER_KHRONOS_validation is now preferred.
        //"VK_LAYER_LUNARG_assistant_layer", "VK_LAYER_LUNARG_standard_validation"
        "VK_LAYER_KHRONOS_validation"
    };

static std::once_flag dispatcherInitFlag;
vk::DispatchLoaderDynamic dispatcher;
vk::DebugReportCallbackEXT msgCallback;

struct Message {
    vk::DebugReportFlagsEXT flags;
    vk::DebugReportObjectTypeEXT objType;
    uint64_t srcObject;
    size_t location;
    int32_t msgCode;
    const char* pLayerPrefix;
    const char* pMsg;
    void* pUserData;
};

using MessageHandler = std::function<VkBool32(const Message& message)>;

VkBool32 messageCallback(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject,
    size_t location,
    int32_t msgCode,
    const char* pLayerPrefix,
    const char* pMsg,
    void* pUserData) {
    std::string message;
    {
        std::stringstream buf;
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
            buf << "ERROR: ";
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            buf << "WARNING: ";
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
            buf << "PERF: ";
        }
        else {
            return false;
        }
        buf << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;
        message = buf.str();
    }

    std::cout << message << std::endl;

#ifdef __ANDROID__
    __android_log_write(ANDROID_LOG_DEBUG, LOG_TAG, message.c_str());
#endif
#ifdef _MSC_VER
    OutputDebugStringA(message.c_str());
    OutputDebugStringA("\n");
#endif
    return false;
}

// Load debug function pointers and set debug callback
// if callBack is NULL, default message callback will be used
void setupDebugging(const vk::Instance& instance, const vk::DebugReportFlagsEXT& flags, const MessageHandler& handler = [](const Message& m) {
	return VK_TRUE;
	}) {
	std::call_once(dispatcherInitFlag, [&] { dispatcher.init(instance, &vkGetInstanceProcAddr); });
	vk::DebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
	dbgCreateInfo.flags = flags;
	dbgCreateInfo.pNext = nullptr;
	msgCallback = instance.createDebugReportCallbackEXT(dbgCreateInfo, nullptr, dispatcher);
}

// Clear debug callback
void freeDebugCallback(const vk::Instance& instance) {
    std::call_once(dispatcherInitFlag, [&] { dispatcher.init(instance, &vkGetInstanceProcAddr); });
    instance.destroyDebugReportCallbackEXT(msgCallback, nullptr, dispatcher);
}

// Setup and functions for the VK_EXT_debug_marker_extension
// Extension spec can be found at https://github.com/KhronosGroup/Vulkan-Docs/blob/1.0-VK_EXT_debug_marker/doc/specs/vulkan/appendices/VK_EXT_debug_marker.txt
// Note that the extension will only be present if run from an offline debugging application
// The actual check for extension presence and enabling it on the device is done in the example base class
// See ExampleBase::createInstance and ExampleBase::createDevice (base/vkx::ExampleBase.cpp)

namespace marker {
// Set to true if function pointer for the debug marker are available
bool active = false;
static std::once_flag markerDispatcherInitFlag;
vk::DispatchLoaderDynamic markerDispatcher;

// Get function pointers for the debug report extensions from the device
void setup(const vk::Instance& instance, const vk::Device& device) {
    std::call_once(markerDispatcherInitFlag, [&] { markerDispatcher.init(instance, &vkGetInstanceProcAddr, device, &vkGetDeviceProcAddr); });
    // Set flag if at least one function pointer is present
    active = (markerDispatcher.vkDebugMarkerSetObjectNameEXT != VK_NULL_HANDLE);
}

// Sets the debug name of an object
// All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
// along with the object type
void setObjectName(const vk::Device& device, uint64_t object, vk::DebugReportObjectTypeEXT objectType, const char* name);

// Set the tag for an object
void setObjectTag(const vk::Device& device, uint64_t object, vk::DebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag);

// Start a new debug marker region
//void beginRegion(const vk::CommandBuffer& cmdbuffer, const std::string& pMarkerName, const G3D::Vector4& color);

// Insert a new debug marker into the command buffer
//void insert(const vk::CommandBuffer& cmdbuffer, const std::string& markerName, const G3D::Vector4& color);

// End the current debug marker region
void endRegion(const vk::CommandBuffer& cmdBuffer);

// Object specific naming functions
void setCommandBufferName(const vk::Device& device, const VkCommandBuffer& cmdBuffer, const char* name);
void setQueueName(const vk::Device& device, const VkQueue& queue, const char* name);
void setImageName(const vk::Device& device, const VkImage& image, const char* name);
void setSamplerName(const vk::Device& device, const VkSampler& sampler, const char* name);
void setBufferName(const vk::Device& device, const VkBuffer& buffer, const char* name);
void setDeviceMemoryName(const vk::Device& device, const VkDeviceMemory& memory, const char* name);
void setShaderModuleName(const vk::Device& device, const VkShaderModule& shaderModule, const char* name);
void setPipelineName(const vk::Device& device, const VkPipeline& pipeline, const char* name);
void setPipelineLayoutName(const vk::Device& device, const VkPipelineLayout& pipelineLayout, const char* name);
void setRenderPassName(const vk::Device& device, const VkRenderPass& renderPass, const char* name);
void setFramebufferName(const vk::Device& device, const VkFramebuffer& framebuffer, const char* name);
void setDescriptorSetLayoutName(const vk::Device& device, const VkDescriptorSetLayout& descriptorSetLayout, const char* name);
void setDescriptorSetName(const vk::Device& device, const VkDescriptorSet& descriptorSet, const char* name);
void setSemaphoreName(const vk::Device& device, const VkSemaphore& semaphore, const char* name);
void setFenceName(const vk::Device& device, const VkFence& fence, const char* name);
void setEventName(const vk::Device& device, const VkEvent& _event, const char* name);

//class Marker {
//public:
//    Marker(const vk::CommandBuffer& cmdBuffer, const std::string& name, const G3D::Vector4& color = G3D::Vector4(0.8f))
//        : cmdBuffer(cmdBuffer) {
//        if (active) {
//            beginRegion(cmdBuffer, name, color);
//        }
//    }
//    ~Marker() {
//        if (active) {
//            endRegion(cmdBuffer);
//        }
//    }
//
//private:
//    const vk::CommandBuffer& cmdBuffer;
//};
}  // namespace marker
}}  // namespace vks::debug
