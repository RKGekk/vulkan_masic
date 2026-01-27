#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include "memory"
#include <string>
#include "unordered_map"
#include "unordered_set"

#include "../pod/descriptor_set_layout.h"

class VulkanDevice;
class DescriptorAllocator;

class VulkanDescriptorsManager {
public:
    using DescriptorSetName = std::string;
    using DescriptorAllocatorName = std::string;

    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name);
    void destroy();

    std::shared_ptr<DescSetLayout> getDescSetLayout(const std::string& desc_set_name) const;
    VkDescriptorSet allocateDescriptorSet(const std::string& desc_set_name) const;

private:
    std::shared_ptr<VulkanDevice> m_device;

    std::unordered_map<DescriptorSetName, std::shared_ptr<DescriptorAllocator>> m_desc_alloc_map;
};