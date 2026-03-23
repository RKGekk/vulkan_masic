#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "render_resource.h"

class VulkanDevice;
class VulkanSampler;

class DescSetLayout {
public:

    using DescSetBindings = std::vector<VkDescriptorSetLayoutBinding>;
    using DescSetBindingsMetadata = std::vector<UpdateMetadata>;
    using BindingNum = uint32_t;
    using BindingIndex = int;

    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& descriptor_sets_node);
    void destroy();
    
    const std::string& getName() const;
    const std::string& getAllocatorName() const;
    const DescSetBindings& getBindings() const;
    VkDescriptorSetLayoutBinding getBinding(VkDescriptorType desc_type) const;
    VkDescriptorSetLayoutBinding getBinding(BindingNum binding_num) const;
    VkDescriptorSetLayoutBinding getBinding(const std::string& binding_name) const;
    bool haveBindingType(VkDescriptorType desc_type) const;
    bool haveBindingNum(BindingNum binding_num) const;
    bool haveBindingName(const std::string& binding_name) const;
    const std::string& getBindingName(VkDescriptorType desc_type) const;
    const std::string& getBindingName(BindingNum binding_num) const;
    const std::vector<std::shared_ptr<VulkanSampler>>& getImmutableSamplers() const;
    const std::vector<VkSampler>& getImmutableSamplersPtr() const;
    VkDescriptorSetLayoutCreateInfo getDescriptorSetLayoutInfo() const;
    VkDescriptorSetLayout getDescriptorSetLayout() const;

private:
    std::shared_ptr<VulkanDevice> m_device;

    std::string m_name;
    std::string m_allocator_name;

    DescSetBindings m_bindings;
    std::unordered_map<std::string, BindingNum> m_binding_name_map;
    std::unordered_map<BindingNum, std::string> m_binding_num_to_name_map;
    std::unordered_map<BindingNum, BindingIndex> m_binding_num_to_idx_map;

    
    std::vector<std::shared_ptr<VulkanSampler>> m_immutable_samplers;
    std::vector<VkSampler> m_immutable_samplers_ptr;
    VkDescriptorSetLayoutCreateInfo m_desc_layout_info;
    VkDescriptorSetLayout m_desc_layout;
};