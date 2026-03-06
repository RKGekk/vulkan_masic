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

    std::shared_ptr<VulkanBuffer> create_buffer(std::string resource_type_name);

protected:
    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanFormatManager> m_format_manager;

    std::unordered_map<std::string, std::shared_ptr<ImageBufferConfig>> m_image_buffer_config_map;
    std::unordered_map<std::string, std::shared_ptr<VulkanImageBuffer>> m_image_map;

    std::unordered_map<std::string, std::shared_ptr<BufferConfig>> m_buffer_config_map;
    std::unordered_map<std::string, std::shared_ptr<VulkanBuffer>> m_buffer_map;
};