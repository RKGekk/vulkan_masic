#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <string>

#include "../pod/render_resource.h"
#include "vulkan_command_buffer.h"

class VulkanDevice;

class VulkanImageBuffer : public RenderResource {
public:
    VulkanImageBuffer(std::shared_ptr<VulkanDevice> device, std::string name);
    VulkanImageBuffer(std::shared_ptr<VulkanDevice> device);

    bool init(VkImage image, VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip_levels = 1u);
    bool init(unsigned char* pixels, VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);
    bool init(unsigned char* pixels, VkImageCreateInfo image_info, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);

    void destroy() override;

    VkImage getImageBuffer() const;
    VkImageView getImageBufferView() const;
    VkDeviceMemory getMemory() const;
    VkDeviceSize getSize() const;
    VkImageLayout getLayout() const;
    const VkImageCreateInfo& getImageInfo() const;
    VkSubresourceLayout getSubresourceSizes(uint32_t mip_level, uint32_t array_layer) const;
    VkSubresourceLayout getSubresourceSizes(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer) const;

    void changeLayout(VkImageLayout new_layout);
    void changeLayout(CommandBatch& command_buffer, VkImageLayout new_layout);

    VkImageView createImageView(VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) const;

    const ResourceName& getName() const override;
    Type getType() const override;

protected:
    std::shared_ptr<VulkanDevice> m_device;
    ResourceName m_name;

    VkImageView createImageView(VkImageViewCreateInfo view_create_info) const;
    VkImageViewCreateInfo createImageViewInfo(VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) const;

    std::shared_ptr<VulkanDevice> m_device;

    VkImage m_image;
    VkImageView m_image_view;
    VkDeviceMemory m_memory;
    
    VkImageCreateInfo m_image_info;
    VkImageViewCreateInfo m_image_view_info;
    VkDeviceSize m_image_size;
    VkImageLayout m_layout;
    VkMemoryPropertyFlags m_properties;
};