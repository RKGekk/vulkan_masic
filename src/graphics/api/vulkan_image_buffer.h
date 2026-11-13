 #pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../pod/render_resource.h"
#include "vulkan_command_buffer.h"

class VulkanDevice;
class ImageBufferConfig;
class CpuResource;

class VulkanImageBuffer : public RenderResource {
public:
    static const std::shared_ptr<VulkanImageBuffer> IMAGE_BUFFER_NULL_PTR;
    static const std::string STAGING_BUFFER_TYPE;

    VulkanImageBuffer(std::shared_ptr<VulkanDevice> device, std::string name);
    VulkanImageBuffer(std::shared_ptr<VulkanDevice> device);

    bool init(std::shared_ptr<ImageBufferConfig> image_buffer_config);
    bool init(VkImage image, std::shared_ptr<ImageBufferConfig> image_buffer_config);
    bool init(std::shared_ptr<CommandBatch>& command_buffer, std::shared_ptr<CpuResource> pixels, std::shared_ptr<ImageBufferConfig> image_buffer_config);
    bool init(std::shared_ptr<CommandBatch>& command_buffer, const std::shared_ptr<ImageBufferConfig>& image_buffer_config_template, const std::string& path_to_file);

    void destroy() override;

    VkImage getImageBuffer() const;
    VkImageView getImageBufferView() const;
    VkImageView getImageBufferView(const std::string& view_name) const;
    const std::unordered_map<std::string, VkImageView>& getImageViewMap() const;
    VkDeviceMemory getMemory() const;
    VkDeviceSize getSize() const;
    
    VkSubresourceLayout getSubresourceSizes(uint32_t mip_level, uint32_t array_layer) const;
    VkSubresourceLayout getSubresourceSizes(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer) const;

    std::shared_ptr<ImageBufferConfig>& getImageConfig();

    void changeLayout(std::shared_ptr<CommandBatch>& command_buffer, VkImageLayout old_layout, VkImageLayout new_layout);

    const ResourceName& getName() const override;
    Type getType() const override;
    
    void AddUpdateMetadataFn(UpdateMetadataFn fn) override;
    void ClearUpdateMetadataFn() override;

    static VkImage createImage(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<ImageBufferConfig>& img_cfg, const ResourceName& name);
    static VkMemoryAllocateInfo getImageSize(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<ImageBufferConfig>& img_cfg, VkImage image);
    static VkDeviceMemory allocateImageMemory(const std::shared_ptr<VulkanDevice>& device, VkImage image, const VkMemoryAllocateInfo& alloc_info);

protected:
    void createViews();

    std::shared_ptr<VulkanDevice> m_device;
    ResourceName m_name;

    std::vector<UpdateMetadataFn> m_update_functions;
    VkImage m_image;
    VkDeviceMemory m_memory;
    VkDeviceSize m_image_size;
    void* m_mapped;
    
    std::unordered_map<std::string, VkImageView> m_image_view_map;
    std::shared_ptr<ImageBufferConfig> m_image_config;
};