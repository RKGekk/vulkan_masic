#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

class VulkanShader {
public:
    bool init(VkDevice device, const std::string& path, VkShaderStageFlagBits pipeline_stage);
    void destroy();

    VkPipelineShaderStageCreateInfo getShaderInfo() const;

private:
    VkShaderModule CreateShaderModule(const std::vector<char>& buffer) const;

    VkDevice m_device;
    VkPipelineShaderStageCreateInfo m_shader_info;
};