#include "vulkan_sampler.h"

#include "vulkan_device.h"

#include <stdexcept>

bool VulkanSampler::init(std::shared_ptr<VulkanDevice> device, const VkSamplerCreateInfo& sampler_info) {
    m_device = device->getDevice();
    m_sampler_info = sampler_info;

    VkResult result = vkCreateSampler(m_device, &sampler_info, nullptr, &m_sampler);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    return true;
}

bool VulkanSampler::init(std::shared_ptr<VulkanDevice> device, uint32_t mip_levels) {
    m_device = device->getDevice();

    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(device->getDeviceAbilities().physical_device, &supported_features);

    VkPhysicalDeviceProperties device_props{};
    vkGetPhysicalDeviceProperties(device->getDeviceAbilities().physical_device, &device_props);
    
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = supported_features.samplerAnisotropy;
    sampler_info.maxAnisotropy = device_props.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = static_cast<float>(mip_levels);

    init(device, sampler_info);
}

void VulkanSampler::destroy() {
    vkDestroySampler(m_device, m_sampler, nullptr);
}

const VkSamplerCreateInfo& VulkanSampler::getSamplerInfo() const {
    return m_sampler_info;
}

VkSampler VulkanSampler::getSampler() const {
    return m_sampler;
}