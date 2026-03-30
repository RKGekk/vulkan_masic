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
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::string& name, const pugi::xml_node& buffer_data, const std::shared_ptr<VulkanFormatManager>& format_manager);
    void destroy();

    const std::string& getName() const;
    bool isSizeDynamic() const;
    void setSizeDynamic(bool is_dynamic);
    bool isSizeDeffered() const;
    void setAlignedSize(VkDeviceSize sz);
    void setNotAlignedSize(VkDeviceSize sz);
    VkDeviceSize getNotAlignedSize() const;
    void setAlignment(VkDeviceSize alignment);
    VkDeviceSize getAlignment() const;
    const VkBufferCreateInfo& getBufferInfo() const;
    VkMemoryPropertyFlags getMemoryProperties() const;
    const std::unordered_map<std::string, std::shared_ptr<BufferViewConfig>>& getViewMap() const;
    const std::shared_ptr<BufferViewConfig>& getView() const;
    const std::shared_ptr<BufferViewConfig>& getView(const std::string& view_name) const;

    std::shared_ptr<BufferConfig> makeInstance(std::string name, VkDeviceSize buffer_size) const;

private:
    std::string m_name;
    VkDeviceSize m_not_aligned_size;
    VkDeviceSize m_alignment;
    VkBufferCreateInfo m_buffer_info;
    bool m_dynamic_size;
    bool m_deffered_size;
    VkMemoryPropertyFlags m_memory_properties;
    std::unordered_map<std::string, std::shared_ptr<BufferViewConfig>> m_view_info_map;
    
};