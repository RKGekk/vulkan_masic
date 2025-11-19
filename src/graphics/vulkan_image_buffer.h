#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

#include "render_resource.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device.h"

class VulkanDevice;

class VulkanImageBuffer : public RenderResource {
public:
    bool init(std::shared_ptr<VulkanDevice> device, VkImage image, VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip_levels = 1u);
    bool init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);
    bool init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, VkImageCreateInfo image_info, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);

    void destroy() override;

    VkImage getImageBuffer() const;
    VkImageView getImageBufferView() const;
    VkDeviceMemory getMemory() const;
    VkDeviceSize getSize() const;
    VkImageLayout getLayout() const;
    const VkImageCreateInfo& getImageInfo() const;

    void changeLayout(VkImageLayout new_layout);
    void changeLayout(CommandBatch& command_buffer, VkImageLayout new_layout);

protected:
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) const;

    std::shared_ptr<VulkanDevice> m_device;

    VkImage m_image;
    VkImageView m_image_view;
    VkDeviceMemory m_memory;
    
    VkImageCreateInfo m_image_info;
    VkDeviceSize m_image_size;
    VkImageLayout m_layout;
    VkMemoryPropertyFlags m_properties;
};