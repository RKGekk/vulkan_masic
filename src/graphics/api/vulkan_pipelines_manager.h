#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_pipeline.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class VulkanDevice;
class VulkanShadersManager;
class VulkanDescriptorsManager;

class VulkanPipelinesManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shaders_manager);
    void destroy();
private:

    std::shared_ptr<VulkanDevice> m_device;

    std::vector<std::string, VulkanPipeline> m_pipeline_name_map;
};