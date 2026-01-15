#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "vulkan_command_pool_type.h"

class QueueFamilyIndices {
public:
    struct QueueFamily {
        uint32_t index;
        bool present_support;
    };

    bool init(VkPhysicalDevice device, VkSurfaceKHR surface);
    
    bool isComplete() const;
    VkSharingMode getBufferSharingMode() const;
    std::unordered_set<uint32_t> getFamilies() const;
    std::vector<uint32_t> getIndices() const;

    const std::vector<VkQueueFamilyProperties>& getQueueFamilies() const;
    std::optional<uint32_t> getFamilyIdx(PoolTypeEnum pool_type) const;

    static std::vector<VkQueueFamilyProperties> getQueueFamilies(VkPhysicalDevice device);

private:
    std::vector<VkQueueFamilyProperties> m_queue_families;
    std::unordered_map<PoolTypeEnum, QueueFamily> m_families;
};