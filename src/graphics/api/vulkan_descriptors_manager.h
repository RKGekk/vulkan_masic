#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include "memory"
#include <string>
#include "unordered_map"

#include "../pod/descriptor_set_layout.h"

class VulkanDevice;

class VulkanDescriptorsManager {
public:
    using DescriptorSetName = std::string;
    using DescSetNameToLayoutMap = std::unordered_map<DescriptorSetName, std::shared_ptr<DescSetLayout>>;

    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name);
    void destroy();

    std::shared_ptr<DescSetLayout> getDescSetLayout(const std::string& desc_set_name) const;
    const DescSetNameToLayoutMap& getNameLayoutMap() const;
    VkDescriptorSet getDescriptorSet(const std::string& desc_set_name) const;

private:
    std::unordered_map<VkDescriptorType, size_t> getTypesCount();
    std::vector<VkDescriptorSetLayout> getVkDescriptorSetLayouts() const;

    std::shared_ptr<VulkanDevice> m_device;

    VkDescriptorPoolCreateInfo m_pool_info;
	VkDescriptorPool m_descriptor_pool;

    DescSetNameToLayoutMap m_name_layout_map;
    std::vector<VkDescriptorSet> m_desc_sets;
    std::unordered_map<DescriptorSetName, size_t> m_name_desc_idx_map;
};