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
#include "vulkan_swapchain.h"

struct QueueFamilyIndices {
    std::vector<VkQueueFamilyProperties> queue_families;

    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    std::optional<uint32_t> compute_family;
    std::optional<uint32_t> transfer_family;

    void init(VkPhysicalDevice device, VkSurfaceKHR surface);
    
    bool isComplete() const;
    VkSharingMode getBufferSharingMode() const;
    std::unordered_set<uint32_t> getFamilies() const;
    std::vector<uint32_t> getIndices() const;

    static std::vector<VkQueueFamilyProperties> getQueueFamilies(VkPhysicalDevice device);
};

struct DeviceAbilities {
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
    int score;
};

class VulkanDevice {
public:
    bool init(const VulkanInstance& instance, VkSurfaceKHR surface);
    void destroy();

    const VulkanDeviceExtensions& getExtensions() const ;

    VkDevice getDevice() const;
    const DeviceAbilities& getDeviceAbilities() const;
    const QueueFamilyIndices& getQueueFamilyIndices() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getComputeQueue() const;
    VkQueue getTransferQueue() const;
    VkQueue getPresentQueue() const;
    VkSampleCountFlagBits getMsaaSamples() const;

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

    static VkDevice createLogicalDevice(VkPhysicalDevice physical_device, const QueueFamilyIndices& queue_family_indices, const VulkanDeviceExtensions& device_extensions, const VulkanInstanceLayersAndExtensions& instance_layers_and_extensions);

    VulkanDeviceExtensions m_extensions;
    VkDevice m_device;
    DeviceAbilities m_device_abilities;
    QueueFamilyIndices m_queue_family_indices;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_compute_queue = VK_NULL_HANDLE;
    VkQueue m_transfer_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;

    VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
};