#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "../pod/render_resource.h"
#include "vulkan_command_buffer.h"

class VulkanDevice;
class ImageBufferConfig;
class BufferConfig;
class VulkanImageBuffer;
class VulkanBuffer;
class VulkanSampler;
class VulkanFormatManager;

class VulkanResourcesManager {
public:
    VulkanResourcesManager(std::shared_ptr<VulkanDevice> device, std::shared_ptr<VulkanFormatManager> format_manager);

    bool init(const std::string& rg_file_path);
    void destroy();

    std::shared_ptr<VulkanImageBuffer> create_image(const std::string& path_to_file);
    std::shared_ptr<VulkanImageBuffer> create_image(VkImage image, std::string image_name, std::string resource_type_name);
    std::shared_ptr<VulkanImageBuffer> create_image(VkImage image, VkExtent2D extent, std::string image_name, std::string resource_type_name);
    std::shared_ptr<VulkanImageBuffer> create_image(unsigned char* pixels, VkExtent2D extent, std::string image_name, std::string resource_type_name);
    std::shared_ptr<VulkanImageBuffer> create_image(std::string image_name, std::string resource_type_name);
    void delete_image(const std::string& image_name);
    void delete_image(std::shared_ptr<VulkanImageBuffer> image_ptr);

    std::shared_ptr<VulkanBuffer> create_buffer(const void* data, VkDeviceSize buffer_size, std::string resource_type_name);
    std::shared_ptr<VulkanBuffer> create_buffer(const void* data, VkDeviceSize buffer_size, std::string buffer_name, std::string resource_type_name);
    void delete_buffer(const std::string& buffer_name);
    void delete_buffer(std::shared_ptr<VulkanBuffer> buffer_ptr);

    const std::shared_ptr<VulkanImageBuffer>& getImageResource(const std::string& resource_global_name);
    const std::shared_ptr<VulkanBuffer>& getBufferResource(const std::string& resource_global_name);
    std::shared_ptr<RenderResource> getResource(const std::string& resource_global_name);
    const std::shared_ptr<ImageBufferConfig> getImageBufferConfigTemplate(const std::string& template_name) const;
    const std::shared_ptr<BufferConfig> getBufferConfigTemplate(const std::string& template_name) const;

protected:
    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanFormatManager> m_format_manager;

    std::unordered_map<std::string, std::shared_ptr<ImageBufferConfig>> m_image_buffer_config_map;
    std::unordered_map<std::string, std::shared_ptr<VulkanImageBuffer>> m_image_map;

    std::unordered_map<std::string, std::shared_ptr<BufferConfig>> m_buffer_config_map;
    std::unordered_map<std::string, std::shared_ptr<VulkanBuffer>> m_buffer_map;
};
