#include "vulkan_queue_family.h"

bool QueueFamilyIndices::init(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    m_queue_families = getQueueFamilies(physical_device);
    
    int i = 0;
    for (const VkQueueFamilyProperties& queue_family : m_queue_families) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
        QueueFamily qf{};
        qf.index = i;
        qf.present_support = static_cast<bool>(present_support);
        if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_families[PoolTypeEnum::GRAPICS] = qf;
        }
        if(queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            if(m_families.contains(PoolTypeEnum::TRANSFER) && !(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(queue_family.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) && !(queue_family.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)) {
                m_families[PoolTypeEnum::TRANSFER] = qf;
            }
            else if (!m_families.contains(PoolTypeEnum::TRANSFER)) {
                m_families[PoolTypeEnum::TRANSFER] = qf;
            }
        }
        if(queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            if(m_families.contains(PoolTypeEnum::COMPUTE) && !(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_family.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) && !(queue_family.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)) {
                m_families[PoolTypeEnum::COMPUTE] = qf;
            }
            else if(!m_families.contains(PoolTypeEnum::COMPUTE)) {
                m_families[PoolTypeEnum::COMPUTE] = qf;
            }
        }
        ++i;
    }
    
    return isComplete();
}

bool QueueFamilyIndices::isComplete() const {
    return m_families.contains(PoolTypeEnum::GRAPICS) && m_families.contains(PoolTypeEnum::COMPUTE) && m_families.contains(PoolTypeEnum::TRANSFER);
}

VkSharingMode QueueFamilyIndices::getBufferSharingMode() const {
    if (m_families.contains(PoolTypeEnum::GRAPICS) && m_families.contains(PoolTypeEnum::TRANSFER) && m_families.at(PoolTypeEnum::GRAPICS).index != m_families.at(PoolTypeEnum::TRANSFER).index) {
        return VK_SHARING_MODE_CONCURRENT;
    }
    return VK_SHARING_MODE_EXCLUSIVE;
}

std::unordered_set<uint32_t> QueueFamilyIndices::getFamilies() const {
    std::unordered_set<uint32_t> family_indices;
    if(m_families.contains(PoolTypeEnum::GRAPICS)) {
        family_indices.insert(m_families.at(PoolTypeEnum::GRAPICS).index);
    }
    if(m_families.contains(PoolTypeEnum::COMPUTE)) {
        family_indices.insert(m_families.at(PoolTypeEnum::COMPUTE).index);
    }
    if(m_families.contains(PoolTypeEnum::TRANSFER)) {
        family_indices.insert(m_families.at(PoolTypeEnum::TRANSFER).index);
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

std::optional<uint32_t> QueueFamilyIndices::getFamilyIdx(PoolTypeEnum pool_type) const {
    if(m_families.contains(PoolTypeEnum::GRAPICS)) {
        return m_families.at(PoolTypeEnum::GRAPICS).index;
    }
    return std::nullopt;
}

std::vector<VkQueueFamilyProperties> QueueFamilyIndices::getQueueFamilies(VkPhysicalDevice device) {
    uint32_t queue_family_count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    return queue_families;
}