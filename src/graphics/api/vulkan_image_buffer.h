 #pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "../pod/render_resource.h"
#include "../pod/image_buffer_config.h"
#include "vulkan_command_buffer.h"

class VulkanDevice;

class VulkanImageBuffer : public RenderResource {
public:
    VulkanImageBuffer(std::shared_ptr<VulkanDevice> device, std::string name);
    VulkanImageBuffer(std::shared_ptr<VulkanDevice> device);

    bool init(VkImage image, std::shared_ptr<ImageBufferConfig> image_buffer_config);
    bool init(unsigned char* pixels, std::shared_ptr<ImageBufferConfig> image_buffer_config);

    void destroy() override;

    VkImage getImageBuffer() const;
    VkImageView getImageBufferView() const;
    const std::unordered_map<std::string, VkImageView>& getImageViewMap() const;
    VkDeviceMemory getMemory() const;
    VkDeviceSize getSize() const;
    
    VkSubresourceLayout getSubresourceSizes(uint32_t mip_level, uint32_t array_layer) const;
    VkSubresourceLayout getSubresourceSizes(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer) const;

    const std::shared_ptr<ImageBufferConfig>& getImageConfig() const;

    void changeLayout(VkImageLayout new_layout);
    void changeLayout(CommandBatch& command_buffer, VkImageLayout new_layout);

    const ResourceName& getName() const override;
    Type getType() const override;

protected:

    std::shared_ptr<VulkanDevice> m_device;
    ResourceName m_name;

    VkImage m_image;
    VkDeviceMemory m_memory;
    VkDeviceSize m_image_size;
    
    std::unordered_map<std::string, VkImageView> m_image_view_map;
    std::shared_ptr<ImageBufferConfig> m_image_config;
};