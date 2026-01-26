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
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name);
    void destroy();

    std::shared_ptr<DescSetLayout> getDescSetLayout(const std::string& name) const;
    const std::unordered_map<std::string, std::shared_ptr<DescSetLayout>>& getNameLayoutMap() const;

private:
    std::unordered_map<VkDescriptorType, size_t> getTypesCount();

    std::shared_ptr<VulkanDevice> m_device;

    VkDescriptorPoolCreateInfo m_pool_info;
	VkDescriptorPool m_descriptor_pool;

    std::unordered_map<std::string, std::shared_ptr<DescSetLayout>> m_name_layout_map;
};