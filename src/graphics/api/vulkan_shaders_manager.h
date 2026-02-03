#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_shader.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

class VulkanDevice;

class VulkanShadersManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name);
    void destroy();

    std::shared_ptr<VulkanShader> getShader(const std::string name) const;

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::vector<std::shared_ptr<VulkanShader>> m_shaders;
    std::unordered_map<std::string, size_t> m_shader_name_map;
};