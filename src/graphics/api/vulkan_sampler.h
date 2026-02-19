#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <string>

class VulkanDevice;

class VulkanSampler {
public:
    VulkanSampler(std::shared_ptr<VulkanDevice> device, std::string name);
    VulkanSampler(std::shared_ptr<VulkanDevice> device);

    bool init(const VkSamplerCreateInfo& sampler_info);
    bool init(uint32_t mip_levels);
    
    void destroy();

    const VkSamplerCreateInfo& getSamplerInfo() const;
    VkSampler getSampler() const;

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::string m_name;
    VkSampler m_sampler;
    VkSamplerCreateInfo m_sampler_info;
};