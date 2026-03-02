#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class VulkanDevice;
class VulkanSampler;
class VulkanFormatManager;
class FormatConfig;

struct BufferViewConfig {
    VkBufferViewCreateInfo view_info;
    std::shared_ptr<FormatConfig> format;
};

class BufferConfig {
public:
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::string& rg_file_path, const std::shared_ptr<VulkanFormatManager>& format_manager);
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::string& name, const pugi::xml_node& buffer_data, const std::shared_ptr<VulkanFormatManager>& format_manager);
    void destroy();

private:
    std::string m_name;
    VkBufferCreateInfo m_buffer_info;
    VkMemoryPropertyFlags m_memory_properties;
    std::unordered_map<std::string, std::shared_ptr<BufferViewConfig>> m_view_info_map;
    
};