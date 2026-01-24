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

private:
    std::unordered_map<std::string, std::shared_ptr<DescSetLayout>> m_name_layout_map;
    std::shared_ptr<VulkanDevice> m_device;
    std::unique_ptr<DescriptorAllocator> m_descriptor_allocators[VK_DESCRIPTOR_TYPE_MAX_ENUM];
};