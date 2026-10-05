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
    static const std::shared_ptr<DescSetLayout> NULL_DESC_LAYOUT_PTR;

    using DescSetBindings = std::vector<VkDescriptorSetLayoutBinding>;
    using BindingNum = uint32_t;
    using BindingIndex = int;
    using BindingName = std::string;

    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& descriptor_sets_node);
    void destroy();
    
    const std::string& getName() const;
    const std::string& getAllocatorName() const;
    const DescSetBindings& getBindings() const;
    VkDescriptorSetLayoutBinding getBinding(VkDescriptorType desc_type) const;
    VkDescriptorSetLayoutBinding getBinding(BindingNum binding_num) const;
    VkDescriptorSetLayoutBinding getBinding(const BindingName& binding_name) const;
    bool haveBindingType(VkDescriptorType desc_type) const;
    bool haveBindingNum(BindingNum binding_num) const;
    bool haveBindingName(const BindingName& binding_name) const;
    const BindingName& getBindingName(VkDescriptorType desc_type) const;
    const BindingName& getBindingName(BindingNum binding_num) const;
    BindingNum getBindingNum(const BindingName& binding_name) const;
    const std::unordered_map<BindingName, BindingNum>& getBindingMap() const;
    const std::vector<std::shared_ptr<VulkanSampler>>& getImmutableSamplers() const;
    const std::vector<VkSampler>& getImmutableSamplersPtr() const;
    VkDescriptorSetLayoutCreateInfo getDescriptorSetLayoutInfo() const;
    VkDescriptorSetLayout getDescriptorSetLayout() const;

private:
    std::shared_ptr<VulkanDevice> m_device;

    std::string m_name;
    std::string m_allocator_name;

    DescSetBindings m_bindings;
    std::unordered_map<BindingName, BindingNum> m_binding_name_map;
    std::unordered_map<BindingNum, BindingName> m_binding_num_to_name_map;
    std::unordered_map<BindingNum, BindingIndex> m_binding_num_to_idx_map;
    
    std::vector<std::shared_ptr<VulkanSampler>> m_immutable_samplers;
    std::vector<VkSampler> m_immutable_samplers_ptr;
    VkDescriptorSetLayoutCreateInfo m_desc_layout_info;
    VkDescriptorSetLayout m_desc_layout;
};