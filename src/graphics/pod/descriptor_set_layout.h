#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

class VulkanDevice;
class VulkanSampler;

class DescSetLayout {
public:
    using DescSetBindings = std::vector<VkDescriptorSetLayoutBinding>;

    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path);
    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& descriptor_sets_node);
    void destroy();
    
    const std::string& getName() const;
    const std::string& getAllocatorName() const;
    const DescSetBindings& getBindings() const;
    const std::vector<std::shared_ptr<VulkanSampler>>& getImmutableSamplers() const;
    const std::vector<VkSampler>& getImmutableSamplersPtr() const;
    VkDescriptorSetLayoutCreateInfo getDescriptorSetLayoutInfo() const;
    VkDescriptorSetLayout getDescriptorSetLayout() const;

private:
    std::shared_ptr<VulkanDevice> m_device;

    std::string m_name;
    std::string m_allocator_name;
    DescSetBindings m_bindings;
    std::vector<std::shared_ptr<VulkanSampler>> m_immutable_samplers;
    std::vector<VkSampler> m_immutable_samplers_ptr;
    VkDescriptorSetLayoutCreateInfo m_desc_layout_info;
    VkDescriptorSetLayout m_desc_layout;
};