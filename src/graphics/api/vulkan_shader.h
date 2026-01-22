#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <string>
#include <vector>

#include "vulkan_descriptor.h"
#include "../pod/shader_signature.h"

class VulkanDevice;

class VulkanShader {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& path, VkShaderStageFlagBits pipeline_stage);
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path);
    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data);
    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<ShaderSignature> shader_signature);

    void destroy();

    const VkPipelineShaderStageCreateInfo& getShaderInfo() const;
    std::shared_ptr<ShaderSignature> getShaderSignature() const;

private:
    VkShaderModule CreateShaderModule(const std::vector<char>& buffer) const;

    std::shared_ptr<VulkanDevice> m_device;
    VkPipelineShaderStageCreateInfo m_shader_info;
    std::shared_ptr<ShaderSignature> m_shader_signature;
};