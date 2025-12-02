#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_buffer.h"
#include "vulkan_image_buffer.h"
#include "vulkan_device.h"

class VulkanTexture : public VulkanImageBuffer {
public:

    bool init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, int width, int height, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    bool init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, int width, int height, VkSampler sampler, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& path_to_file);
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& path_to_file, VkSampler sampler);
    bool init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, size_t size);
    bool init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, size_t size, VkSampler sampler);

    void destroy() override;

    VkSampler getSampler() const;
    VkDescriptorImageInfo getDescImageInfo() const;

private:
    VkSampler createTextureSampler(uint32_t mip_levels) const;

    VkSampler m_texture_sampler = VK_NULL_HANDLE;
    VkDescriptorImageInfo m_image_desc_info;
};