#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

class QueueFamilyIndices {
public:
    void init(VkPhysicalDevice device, VkSurfaceKHR surface);
    
    bool isComplete() const;
    VkSharingMode getBufferSharingMode() const;
    std::unordered_set<uint32_t> getFamilies() const;
    std::vector<uint32_t> getIndices() const;

    const std::vector<VkQueueFamilyProperties>& getQueueFamilies() const;

    const std::optional<uint32_t>& getGraphicsFamily() const;
    const std::optional<uint32_t>& getPresentFamily() const;
    const std::optional<uint32_t>& getComputeFamily() const;
    const std::optional<uint32_t>& getTransferFamily() const;

    static std::vector<VkQueueFamilyProperties> getQueueFamilies(VkPhysicalDevice device);

private:
    std::vector<VkQueueFamilyProperties> m_queue_families;

    std::optional<uint32_t> m_graphics_family;
    std::optional<uint32_t> m_present_family;
    std::optional<uint32_t> m_compute_family;
    std::optional<uint32_t> m_transfer_family;
};