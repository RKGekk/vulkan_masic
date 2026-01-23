#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_pipeline.h"

#include <memory>
#include <stdexcept>
#include <vector>

class VulkanDevice;

class VulkanPipelinesManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device);
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path);
    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data);
    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<ShaderSignature> shader_signature);
    void destroy();
private:
    void saveCacheToFile(VkPipelineCache cache, const std::string& file_name);

    std::shared_ptr<VulkanDevice> m_device;
    VkPipelineCache m_pipeline_cache = VK_NULL_HANDLE;
};