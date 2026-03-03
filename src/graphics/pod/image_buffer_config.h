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

struct ImageBufferViewConfig {
    VkImageViewCreateInfo image_viwe_info;
    std::shared_ptr<FormatConfig> format;
};

class ImageBufferConfig {
public:
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::string& rg_file_path, const std::shared_ptr<VulkanFormatManager>& format_manager);
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::string& name, const pugi::xml_node& image_buffer_data, const std::shared_ptr<VulkanFormatManager>& format_manager);
    void destroy();

    const VkImageCreateInfo& getImageInfo() const;

private:
    std::string m_name;
    VkImageCreateInfo m_image_info;
    std::shared_ptr<FormatConfig> m_format;
    std::unordered_map<std::string, std::shared_ptr<ImageBufferViewConfig>> m_image_view_info_map;
    VkMemoryPropertyFlags m_memory_properties;
    std::vector<std::shared_ptr<VulkanSampler>> m_samplers;
};