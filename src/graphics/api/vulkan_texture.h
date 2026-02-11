#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <string>

#include "vulkan_buffer.h"
#include "vulkan_image_buffer.h"
#include "vulkan_sampler.h"

class VulkanDevice;

class VulkanTexture : public VulkanImageBuffer {
public:
    VulkanTexture(std::shared_ptr<VulkanDevice> device, std::string name);
    VulkanTexture(std::shared_ptr<VulkanDevice> device);

    bool init(unsigned char* pixels, int width, int height, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    bool init(unsigned char* pixels, int width, int height, std::shared_ptr<VulkanSampler> sampler, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    bool init(const std::string& path_to_file);
    bool init(const std::string& path_to_file, std::shared_ptr<VulkanSampler> sampler);
    bool init(unsigned char* pixels, size_t size);
    bool init(unsigned char* pixels, size_t size, std::shared_ptr<VulkanSampler> sampler);

    void destroy() override;

    std::shared_ptr<VulkanSampler> getSampler() const;
    VkDescriptorImageInfo getDescImageInfo() const;

private:

    std::shared_ptr<VulkanSampler> m_texture_sampler;
    VkDescriptorImageInfo m_image_desc_info;
};