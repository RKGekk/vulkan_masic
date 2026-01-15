#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_pipeline.h"

#include <memory>
#include <stdexcept>
#include <vector>

class VulkanPipelinesManager {
public:
    bool init(VkDevice device);
    void destroy();
private:
    void saveCacheToFile(VkPipelineCache cache, const std::string& file_name);

    VkDevice m_device;
    VkPipelineCache m_pipeline_cache = VK_NULL_HANDLE;
};