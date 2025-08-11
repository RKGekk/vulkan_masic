#include "vulkan_shader.h"
#include "../tools/string_tools.h"

bool VulkanShader::init(VkDevice device, const std::string& path, VkShaderStageFlagBits pipeline_stage) {
    m_device = device;

    std::vector<char> shader_buff = readFile(path);
    VkShaderModule shader_module = CreateShaderModule(shader_buff);


    m_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shader_info.pNext = nullptr;
    m_shader_info.flags = 0u;
    m_shader_info.stage = pipeline_stage;
    m_shader_info.module = shader_module;
    m_shader_info.pName = "main";
    m_shader_info.pSpecializationInfo = nullptr; // fill constants

    return true;
}


void VulkanShader::destroy() {
    vkDestroyShaderModule(m_device, m_shader_info.module, nullptr);
}

const VkPipelineShaderStageCreateInfo& VulkanShader::getShaderInfo() const {
    return m_shader_info;
}

VkShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& buffer) const {
    VkShaderModuleCreateInfo shader_module_info{};
    shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.codeSize = static_cast<uint32_t>(buffer.size());
    shader_module_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(m_device, &shader_module_info, nullptr, &shader_module);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader!");
    }
    return shader_module;
}