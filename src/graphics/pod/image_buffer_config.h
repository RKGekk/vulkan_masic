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

class ImageBufferConfig {
public:
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::string& rg_file_path);
    bool init(const std::shared_ptr<VulkanDevice>& device, const pugi::xml_node& image_buffer_data);
    void destroy();

private:
    std::string m_name;
    VkImageCreateInfo m_image_info;
    std::unordered_map<std::string, VkImageViewCreateInfo> m_image_view_info_map;
    std::unordered_map<std::string, bool> m_auto_subresource_mip_levels_map;
    bool m_has_extent;
    std::string m_extent_source;
    bool m_auto_mip_levels;
    VkMemoryPropertyFlags m_memory_properties;
    std::vector<std::shared_ptr<VulkanSampler>> m_samplers;
};