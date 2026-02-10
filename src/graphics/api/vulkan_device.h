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
#include "../../tools/thread_pool.h"

struct DeviceAbilities {
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
    bool host_visible_single_heap_memory;
    int score;
};

enum class PhysicalDeviceFeaturesFlag : uint64_t {
    robustBufferAccess = 1ULL << 0,
    fullDrawIndexUint32 = 1ULL << 1,
    imageCubeArray = 1ULL << 2,
    independentBlend = 1ULL << 3,
    geometryShader = 1ULL << 4,
    tessellationShader = 1ULL << 5,
    sampleRateShading = 1ULL << 6,
    dualSrcBlend = 1ULL << 7,
    logicOp = 1ULL << 8,
    multiDrawIndirect = 1ULL << 9,
    drawIndirectFirstInstance = 1ULL << 10,
    depthClamp = 1ULL << 11,
    depthBiasClamp = 1ULL << 12,
    fillModeNonSolid = 1ULL << 13,
    depthBounds = 1ULL << 14,
    wideLines = 1ULL << 15,
    largePoints = 1ULL << 16,
    alphaToOne = 1ULL << 17,
    multiViewport = 1ULL << 18,
    samplerAnisotropy = 1ULL << 19,
    textureCompressionETC2 = 1ULL << 20,
    textureCompressionASTC_LDR = 1ULL << 21,
    textureCompressionBC = 1ULL << 22,
    occlusionQueryPrecise = 1ULL << 23,
    pipelineStatisticsQuery = 1ULL << 24,
    vertexPipelineStoresAndAtomics = 1ULL << 25,
    fragmentStoresAndAtomics = 1ULL << 26,
    shaderTessellationAndGeometryPointSize = 1ULL << 27,
    shaderImageGatherExtended = 1ULL << 28,
    shaderStorageImageExtendedFormats = 1ULL << 29,
    shaderStorageImageMultisample = 1ULL << 30,
    shaderStorageImageReadWithoutFormat = 1ULL << 31,
    shaderStorageImageWriteWithoutFormat = 1ULL << 32,
    shaderUniformBufferArrayDynamicIndexing = 1ULL << 33,
    shaderSampledImageArrayDynamicIndexing = 1ULL << 34,
    shaderStorageBufferArrayDynamicIndexing = 1ULL << 35,
    shaderStorageImageArrayDynamicIndexing = 1ULL << 36,
    shaderClipDistance = 1ULL << 37,
    shaderCullDistance = 1ULL << 38,
    shaderFloat64 = 1ULL << 39,
    shaderInt64 = 1ULL << 40,
    shaderInt16 = 1ULL << 41,
    shaderResourceResidency = 1ULL << 42,
    shaderResourceMinLod = 1ULL << 43,
    sparseBinding = 1ULL << 44,
    sparseResidencyBuffer = 1ULL << 45,
    sparseResidencyImage2D = 1ULL << 46,
    sparseResidencyImage3D = 1ULL << 47,
    sparseResidency2Samples = 1ULL << 48,
    sparseResidency4Samples = 1ULL << 49,
    sparseResidency8Samples = 1ULL << 50,
    sparseResidency16Samples = 1ULL << 51,
    sparseResidencyAliased = 1ULL << 52,
    variableMultisampleRate = 1ULL << 53,
    inheritedQueries = 1ULL << 54
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
    const std::shared_ptr<VulkanCommandManager>& getCommandManager() const;
    std::shared_ptr<VulkanCommandManager>& getCommandManager();

    uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkImageUsageFlags usage, VkExtent2D extent, uint32_t mip_levels, VkSampleCountFlags sample_count, VkImageCreateFlags flags = 0u) const;
    std::vector<VkSparseImageFormatProperties> findSparseFormatAbilities(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) const;
    VkImageFormatProperties findFormatAbilities(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags) const;
    bool checkFormatSupported(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkExtent2D extent, uint32_t mip_levels, VkSampleCountFlags sample_count, VkImageCreateFlags flags = 0u) const;
    VkFormat findDepthFormat(VkImageUsageFlags usage, VkExtent2D extent, uint32_t mip_levels, VkSampleCountFlags sample_count, VkImageCreateFlags flags = 0u) const;

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
    std::shared_ptr<VulkanCommandManager> m_command_manager;
    std::shared_ptr<ThreadPool> m_thread_pool;

    VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
};