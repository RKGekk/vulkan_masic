#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_instance.h"
#include "vulkan_instance_layers_and_extensions.h"
#include "vulkan_device_extensions.h"
#include "vulkan_command_manager.h"
#include "vulkan_buffer.h"
#include "../tools/thread_pool.h"

struct DeviceAbilities {
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
    bool host_visible_single_heap_memory;
    int score;
};

enum class PhysicalDeviceFeaturesFlag : uint64_t {
    robustBufferAccess = 1 << 0,
    fullDrawIndexUint32 = 1 << 1,
    imageCubeArray = 1 << 2,
    independentBlend = 1 << 3,
    geometryShader = 1 << 4,
    tessellationShader = 1 << 5,
    sampleRateShading = 1 << 6,
    dualSrcBlend = 1 << 7,
    logicOp = 1 << 8,
    multiDrawIndirect = 1 << 9,
    drawIndirectFirstInstance = 1 << 10,
    depthClamp = 1 << 11,
    depthBiasClamp = 1 << 12,
    fillModeNonSolid = 1 << 13,
    depthBounds = 1 << 14,
    wideLines = 1 << 15,
    largePoints = 1 << 16,
    alphaToOne = 1 << 17,
    multiViewport = 1 << 18,
    samplerAnisotropy = 1 << 19,
    textureCompressionETC2 = 1 << 20,
    textureCompressionASTC_LDR = 1 << 21,
    textureCompressionBC = 1 << 22,
    occlusionQueryPrecise = 1 << 23,
    pipelineStatisticsQuery = 1 << 24,
    vertexPipelineStoresAndAtomics = 1 << 25,
    fragmentStoresAndAtomics = 1 << 26,
    shaderTessellationAndGeometryPointSize = 1 << 27,
    shaderImageGatherExtended = 1 << 28,
    shaderStorageImageExtendedFormats = 1 << 29,
    shaderStorageImageMultisample = 1 << 30,
    shaderStorageImageReadWithoutFormat = 1 << 31,
    shaderStorageImageWriteWithoutFormat = 1 << 32,
    shaderUniformBufferArrayDynamicIndexing = 1 << 33,
    shaderSampledImageArrayDynamicIndexing = 1 << 34,
    shaderStorageBufferArrayDynamicIndexing = 1 << 35,
    shaderStorageImageArrayDynamicIndexing = 1 << 36,
    shaderClipDistance = 1 << 37,
    shaderCullDistance = 1 << 38,
    shaderFloat64 = 1 << 39,
    shaderInt64 = 1 << 40,
    shaderInt16 = 1 << 41,
    shaderResourceResidency = 1 << 42,
    shaderResourceMinLod = 1 << 43,
    sparseBinding = 1 << 44,
    sparseResidencyBuffer = 1 << 45,
    sparseResidencyImage2D = 1 << 46,
    sparseResidencyImage3D = 1 << 47,
    sparseResidency2Samples = 1 << 48,
    sparseResidency4Samples = 1 << 49,
    sparseResidency8Samples = 1 << 50,
    sparseResidency16Samples = 1 << 51,
    sparseResidencyAliased = 1 << 52,
    variableMultisampleRate = 1 << 53,
    inheritedQueries = 1 << 54
};

class VulkanDevice {
public:
    bool init(const VulkanInstance& instance, VkSurfaceKHR surface, std::shared_ptr<ThreadPool> thread_pool);
    void destroy();

    const VulkanDeviceExtensions& getExtensions() const;
    const VkPhysicalDeviceFeatures& getPhysicalDeviceFeatures() const;
    VkDevice getDevice() const;
    const DeviceAbilities& getDeviceAbilities() const;
    VkSampleCountFlagBits getMsaaSamples() const;
    bool checkFeaturesSupported(const VkPhysicalDeviceFeatures& features);
    const VulkanCommandManager& getCommandManager() const;
    VulkanCommandManager& getCommandManager();

    uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    VkFormat findDepthFormat();

    static bool hasStencilComponent(VkFormat format);
    static VkAccessFlags getDstAccessMask(VkBufferUsageFlags usage);
    static size_t getBytesCount(VkFormat format);

private:
    template<typename Container>
    static Container getRequiredDeviceExtensions() {
        Container extensions;
#ifdef __APPLE__
        insert_in_container(extensions, VK_KHR_portability_subset);
#endif
        insert_in_container(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifndef NDEBUG
        //insert_in_container(extensions, VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
#endif
        return extensions;
    }

    static DeviceAbilities pickPhysicalDevice(VkInstance vk_instance, VkSurfaceKHR surface);
    static std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance vk_instance);
    static DeviceAbilities rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface);
    static uint64_t getDeviceMaxMemoryLimit(VkPhysicalDeviceMemoryProperties phys_device_mem_prop);
    static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
    static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDeviceProperties physical_device_properties);
    static VkDevice createLogicalDevice(const DeviceAbilities& physical_device, const std::unordered_set<uint32_t>& family_indices, const VulkanDeviceExtensions& device_extensions, const VulkanInstanceLayersAndExtensions& instance_layers_and_extensions);
    static bool checkFeatures(const VkPhysicalDeviceFeatures& device_features, const VkPhysicalDeviceFeatures& features_to_check);
    static bool isHostVisibleSingleHeapMemory(VkPhysicalDevice physical_device);
    static uint64_t getFeaturesVector(const VkPhysicalDeviceFeatures& device_features);

    VulkanDeviceExtensions m_extensions;
    VkDevice m_device;
    DeviceAbilities m_device_abilities;
    VulkanCommandManager m_command_manager;
    std::shared_ptr<ThreadPool> m_thread_pool;

    VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
};