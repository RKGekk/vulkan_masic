#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_buffer.h"
#include "vulkan_device.h"

class VulkanTexture : public IVulkanImageBuffer {
public:

    bool init(std::shared_ptr<VulkanDevice> device, const std::string& path_to_file);
    void destroy() override;

    ImageBuffer getImageBuffer() const override;
    ImageBufferAndView getImageBufferAndView() const override;
    VkSampler getSampler() const override;
    VkDeviceSize getSize() const override;
    VkDescriptorImageInfo getDescImageInfo() const override;

private:
    VkSampler createTextureSampler(uint32_t mip_levels) const;

    std::shared_ptr<VulkanDevice> m_device;

    VkSampler m_texture_sampler = VK_NULL_HANDLE;
    ImageBufferAndView m_texture_image;

    VkDescriptorImageInfo m_image_info;
};