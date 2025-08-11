#include "vulkan_queue_family.h"

void QueueFamilyIndices::init(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    m_queue_families = getQueueFamilies(physical_device);
    
    int i = 0;
    for (const VkQueueFamilyProperties& queue_family : m_queue_families) {
        if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_graphics_family = i;
        }
        if(queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            m_transfer_family = i;
        }
        if(queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            m_compute_family = i;
        }
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
        if(present_support) {
            m_present_family = i;
        }
        
        if (isComplete()) {
            break;
        }
        ++i;
    }
}

bool QueueFamilyIndices::isComplete() const {
    return m_graphics_family.has_value() && m_present_family.has_value() && m_compute_family.has_value() && m_transfer_family.has_value();
}

VkSharingMode QueueFamilyIndices::getBufferSharingMode() const {
    if (m_graphics_family.has_value() && m_transfer_family.has_value() && m_graphics_family.value() != m_transfer_family.value()) {
        return VK_SHARING_MODE_CONCURRENT;
    }
    return VK_SHARING_MODE_EXCLUSIVE;
}

std::unordered_set<uint32_t> QueueFamilyIndices::getFamilies() const {
    std::unordered_set<uint32_t> family_indices;
    if(m_graphics_family.has_value()) {
        family_indices.insert(m_graphics_family.value());
    }
    if(m_present_family.has_value()) {
        family_indices.insert(m_present_family.value());
    }
    if(m_compute_family.has_value()) {
        family_indices.insert(m_compute_family.value());
    }
    if(m_transfer_family.has_value()) {
        family_indices.insert(m_transfer_family.value());
    }
    return family_indices;
}

std::vector<uint32_t> QueueFamilyIndices::getIndices() const {
    const auto& families = getFamilies();
    std::vector<uint32_t> result(families.cbegin(), families.cend());
    return result;
}

const std::vector<VkQueueFamilyProperties>& QueueFamilyIndices::getQueueFamilies() const {
    return m_queue_families;
}

const std::optional<uint32_t>& QueueFamilyIndices::getGraphicsFamily() const {
    return m_graphics_family;
}

const std::optional<uint32_t>& QueueFamilyIndices::getPresentFamily() const {
    return m_present_family;
}

const std::optional<uint32_t>& QueueFamilyIndices::getComputeFamily() const {
    return m_compute_family;
}

const std::optional<uint32_t>& QueueFamilyIndices::getTransferFamily() const {
    return m_transfer_family;
}

std::vector<VkQueueFamilyProperties> QueueFamilyIndices::getQueueFamilies(VkPhysicalDevice device) {
    uint32_t queue_family_count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    return queue_families;
}