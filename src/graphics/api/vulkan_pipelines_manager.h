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
    void destroy();
private:
    void saveCacheToFile(VkPipelineCache cache, const std::string& file_name);

    std::shared_ptr<VulkanDevice> m_device;
    VkPipelineCache m_pipeline_cache = VK_NULL_HANDLE;
};