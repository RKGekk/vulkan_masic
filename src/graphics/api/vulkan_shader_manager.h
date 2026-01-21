#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_shader.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class VulkanShaderManager {
public:

    bool init(VkDevice device, const std::string& rg_file_name);
    void destroy();
private:

    VkDevice m_device;
};