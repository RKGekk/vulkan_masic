#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

class VulkanDevice;

class VulkanSampler {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const VkSamplerCreateInfo& sampler_info);
    bool init(std::shared_ptr<VulkanDevice> device, uint32_t mip_levels);
    
    void destroy();

    const VkSamplerCreateInfo& getSamplerInfo() const;
    VkSampler getSampler() const;

private:
    VkDevice m_device;
    VkSampler m_sampler;
    VkSamplerCreateInfo m_sampler_info;
};