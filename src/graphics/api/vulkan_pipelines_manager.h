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
struct Managers;

class VulkanPipelinesManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path);
    void destroy();

    std::shared_ptr<VulkanPipeline> getPipeline(std::string pipeline_name);

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::unordered_map<std::string, std::shared_ptr<VulkanPipeline>> m_pipeline_name_map;
};