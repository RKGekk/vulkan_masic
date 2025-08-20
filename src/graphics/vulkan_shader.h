#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "vulkan_descriptor.h"

class VulkanShader {
public:
    bool init(VkDevice device, const std::string& path, VkShaderStageFlagBits pipeline_stage);
    void destroy();

    const VkPipelineShaderStageCreateInfo& getShaderInfo() const;

private:
    VkShaderModule CreateShaderModule(const std::vector<char>& buffer) const;

    VkDevice m_device;
    VkPipelineShaderStageCreateInfo m_shader_info;
    VulkanDescriptor m_descriptor;
};