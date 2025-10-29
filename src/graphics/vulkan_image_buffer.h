#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

#include "render_resource.h"

class VulkanDevice;

class VulkanImageBuffer : public RenderResource {
public:
    bool init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, size_t width, size_t height, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);
    bool init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, VkImageCreateInfo image_info, VkImageAspectFlags aspect_flags);

    void destroy() override;

    VkImage getImageBuffer() const;
    VkImageView getImageBufferView() const;
    VkDeviceMemory getMemory() const;
    VkDeviceSize getSize() const;
    VkImageLayout getLayout() const;

protected:
    std::shared_ptr<VulkanDevice> m_device;

    VkImage m_image;
    VkImageView m_image_view;
    VkDeviceMemory m_memory;
    
    VkImageCreateInfo m_image_info;
    VkDeviceSize m_image_size;
    VkImageLayout m_layout;
};