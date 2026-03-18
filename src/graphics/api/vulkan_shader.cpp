#include "vulkan_shader.h"

#include "vulkan_device.h"
#include "../../tools/string_tools.h"

#include <filesystem>

bool VulkanShader::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data) {
    std::shared_ptr<ShaderSignature> shader_signature = std::make_shared<ShaderSignature>();
    shader_signature->init(shader_data);

    return init(std::move(device), std::move(shader_signature));
}

bool VulkanShader::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<ShaderSignature> shader_signature) {
    using namespace std::literals;

    m_device = std::move(device);
    m_shader_signature = std::move(shader_signature);

    std::filesystem::path shader_file_path(m_shader_signature->getFileName());
    if(shader_file_path.extension() != ".spv") {
        shader_file_path += ".spv";
    }

    if(!std::filesystem::exists(shader_file_path)) {
        std::filesystem::path shader_file_directory("shaders/");
        shader_file_path = shader_file_directory / shader_file_path;
    }

    std::vector<char> shader_buff = readFile(shader_file_path.string());
    VkShaderModule shader_module = CreateShaderModule(shader_buff);

    m_shader_info = VkPipelineShaderStageCreateInfo{};
    m_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shader_info.pNext = nullptr;
    m_shader_info.flags = m_shader_signature->getPipelineShaderStageCreateFlags();
    m_shader_info.stage = m_shader_signature->getStage();
    m_shader_info.module = shader_module;
    m_shader_info.pName = m_shader_signature->getEntryPointName().c_str();
    m_shader_info.pSpecializationInfo = &(m_shader_signature->getSpecializationInfo()); // fill constants

    return true;
}

void VulkanShader::destroy() {
    
}

const VkPipelineShaderStageCreateInfo& VulkanShader::getShaderInfo() const {
    return m_shader_info;
}

std::shared_ptr<ShaderSignature> VulkanShader::getShaderSignature() const {
    return m_shader_signature;
}

VkShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& buffer) const {
    VkShaderModuleCreateInfo shader_module_info{};
    shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.codeSize = static_cast<uint32_t>(buffer.size());
    shader_module_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(m_device->getDevice(), &shader_module_info, nullptr, &shader_module);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader!");
    }
    return shader_module;
}