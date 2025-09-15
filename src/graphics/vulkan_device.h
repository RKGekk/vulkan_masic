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
#include "../tools/thread_pool.h"

struct DeviceAbilities {
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
    bool host_visible_single_heap_memory;
    int score;
};

struct VulkanBuffer {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo bufferInfo;
    VkMemoryPropertyFlags properties;
};

struct ImageBufferAndView;
struct ImageBuffer {
    VkDeviceMemory memory;
    VkImage image;
    VkImageCreateInfo image_info;
    VkDeviceSize image_size;
    ImageBufferAndView getImageAndView() const;
};

struct ImageBufferAndView {
    VkDeviceMemory memory;
    VkImage image;
    VkImageCreateInfo image_info;
    VkDeviceSize image_size;
    VkImageView image_view;
    ImageBuffer getImage() const;
};

class VulkanDevice {
public:
    bool init(const VulkanInstance& instance, VkSurfaceKHR surface, std::shared_ptr<ThreadPool> thread_pool);
    void destroy();

    const VulkanDeviceExtensions& getExtensions() const ;

    VkDevice getDevice() const;
    const DeviceAbilities& getDeviceAbilities() const;
    VkSampleCountFlagBits getMsaaSamples() const;
    const VulkanCommandManager& getCommandManager() const;
    VulkanCommandManager& getCommandManager();

    VulkanBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip_levels = 1u) const;
    std::vector<VkImageView> createImageViews(const std::vector<VkImage>& images, VkFormat format, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip_levels = 1u) const;
    ImageBuffer createImage(const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties) const;
    ImageBufferAndView createImage(const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags, uint32_t mip_levels) const;
    ImageBuffer createImage(const std::string& path_to_file);
    ImageBufferAndView createImageAndView(const std::string& path_to_file);

    uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    VkFormat findDepthFormat();

    static bool hasStencilComponent(VkFormat format);

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
    static VkDevice createLogicalDevice(VkPhysicalDevice physical_device, const std::unordered_set<uint32_t>& family_indices, const VulkanDeviceExtensions& device_extensions, const VulkanInstanceLayersAndExtensions& instance_layers_and_extensions);
    static bool isHostVisibleSingleHeapMemory(VkPhysicalDevice physical_device);

    VulkanDeviceExtensions m_extensions;
    VkDevice m_device;
    DeviceAbilities m_device_abilities;
    VulkanCommandManager m_command_manager;
    std::shared_ptr<ThreadPool> m_thread_pool;

    VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
};