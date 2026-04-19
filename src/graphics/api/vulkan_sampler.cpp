#include "vulkan_sampler.h"

#include "vulkan_device.h"
#include "../../application.h"

#include <stdexcept>

VulkanSampler::VulkanSampler(std::shared_ptr<VulkanDevice> device, std::string name) : m_device(std::move(device)), m_name(std::move(name)) {}

VulkanSampler::VulkanSampler(std::shared_ptr<VulkanDevice> device) : m_device(std::move(device)), m_name(std::to_string(rand())) {}

bool VulkanSampler::init(const VkSamplerCreateInfo& sampler_info) {
    m_sampler_info = sampler_info;

    VkResult result = vkCreateSampler(m_device->getDevice(), &sampler_info, nullptr, &m_sampler);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

#ifndef NDEBUG
    std::string sampler_name = "sampler_"s + m_name;
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_SAMPLER;
    name_info.objectHandle = (uint64_t)m_sampler;
    name_info.pObjectName = sampler_name.c_str();

    vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif    

    return true;
}

bool VulkanSampler::init(uint32_t mip_levels) {

    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(m_device->getDeviceAbilities().physical_device, &supported_features);

    VkPhysicalDeviceProperties device_props{};
    vkGetPhysicalDeviceProperties(m_device->getDeviceAbilities().physical_device, &device_props);
    
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

    return init(sampler_info);
}

void VulkanSampler::destroy() {
    if(m_sampler != VK_NULL_HANDLE) {
        vkDestroySampler(m_device->getDevice(), m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
}

const VkSamplerCreateInfo& VulkanSampler::getSamplerInfo() const {
    return m_sampler_info;
}

VkSampler VulkanSampler::getSampler() const {
    return m_sampler;
}