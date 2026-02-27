#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../pod/format_config.h"
#include "../../window_surface.h"

class VulkanDevice;

class VulkanFormatManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::shared_ptr<WindowSurface>& window, const std::string& rg_file_path);
    
    std::shared_ptr<FormatConfig> getFormat(const std::string& name);

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::unordered_map<std::string, std::shared_ptr<FormatConfig>> m_format_map;
};