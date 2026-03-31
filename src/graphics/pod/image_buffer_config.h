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
    VkImageViewCreateInfo image_view_info;
    std::shared_ptr<FormatConfig> format;
};

class ImageBufferConfig {
public:
    bool init(const std::shared_ptr<VulkanDevice>& device, std::string name, const pugi::xml_node& image_buffer_data, const std::shared_ptr<VulkanFormatManager>& format_manager);
    void destroy();

    const VkImageCreateInfo& getImageInfo() const;
    VkImageLayout getAfterInitLayout() const;
    void setAfterInitLayout(VkImageLayout new_layout);
    const std::vector<std::shared_ptr<VulkanSampler>>& getSamplers() const;
    void setSampler(std::shared_ptr<VulkanSampler> sampler);
    const std::shared_ptr<VulkanSampler>& getSampler() const;
    const std::shared_ptr<FormatConfig>& getFormat() const;
    const std::unordered_map<std::string, std::shared_ptr<ImageBufferViewConfig>>& getViewInfoMap() const;
    VkMemoryPropertyFlags getMemoryProperties() const;
    void setMemoryProperties(VkMemoryPropertyFlags props);
    bool isMemoryPropertiesByMemoryRequirements() const;

    std::shared_ptr<ImageBufferConfig> makeInstance(std::string name, VkExtent2D extent = {0, 0}) const;

private:
    std::string m_name;
    VkImageCreateInfo m_image_info;
    VkImageLayout m_after_init_layout;
    std::shared_ptr<FormatConfig> m_format;
    std::unordered_map<std::string, std::shared_ptr<ImageBufferViewConfig>> m_image_view_info_map;
    VkMemoryPropertyFlags m_memory_properties;
    bool m_memory_properties_by_memory_requirements;
    std::vector<std::shared_ptr<VulkanSampler>> m_samplers;
};