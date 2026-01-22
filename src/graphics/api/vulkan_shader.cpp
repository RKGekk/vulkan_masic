#include "vulkan_shader.h"

#include "vulkan_device.h"
#include "../../tools/string_tools.h"

bool VulkanShader::init(std::shared_ptr<VulkanDevice> device, const std::string& path, VkShaderStageFlagBits pipeline_stage) {
    m_device = device;

    std::vector<char> shader_buff = readFile(path);
    VkShaderModule shader_module = CreateShaderModule(shader_buff);

    m_shader_info = VkPipelineShaderStageCreateInfo{};
    m_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shader_info.pNext = nullptr;
    m_shader_info.flags = 0u;
    m_shader_info.stage = pipeline_stage;
    m_shader_info.module = shader_module;
    m_shader_info.pName = "main";
    m_shader_info.pSpecializationInfo = nullptr; // fill constants

    return true;
}

bool VulkanShader::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path) {
    m_device = device;

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node shaders_node = root_node.child("Shaders");
	if (shaders_node) {
		for (pugi::xml_node shader_node = shaders_node.first_child(); shader_node; shader_node = shader_node.next_sibling()) {
			return init(device, shader_node);
		}
	}

    return true;
}

bool VulkanShader::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data) {
    m_device = device;

    m_shader_signature.init(device, shader_data);

    std::vector<char> shader_buff = readFile(m_shader_signature.getFileName());
    VkShaderModule shader_module = CreateShaderModule(shader_buff);

    m_shader_info = VkPipelineShaderStageCreateInfo{};
    m_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shader_info.pNext = nullptr;
    m_shader_info.flags = m_shader_signature.getPipelineShaderStageCreateFlags();
    m_shader_info.stage = m_shader_signature.getStage();
    m_shader_info.module = shader_module;
    m_shader_info.pName = m_shader_signature.getEntryPointName().c_str();
    m_shader_info.pSpecializationInfo = &(m_shader_signature.getSpecializationInfo()); // fill constants
}

bool VulkanShader::init(std::shared_ptr<VulkanDevice> device, const ShaderSignature& shader_signature) {
    m_device = device;

    m_shader_signature = shader_signature;

    std::vector<char> shader_buff = readFile(m_shader_signature.getFileName());
    VkShaderModule shader_module = CreateShaderModule(shader_buff);

    m_shader_info = VkPipelineShaderStageCreateInfo{};
    m_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shader_info.pNext = nullptr;
    m_shader_info.flags = m_shader_signature.getPipelineShaderStageCreateFlags();
    m_shader_info.stage = m_shader_signature.getStage();
    m_shader_info.module = shader_module;
    m_shader_info.pName = m_shader_signature.getEntryPointName().c_str();
    m_shader_info.pSpecializationInfo = &(m_shader_signature.getSpecializationInfo()); // fill constants
}

void VulkanShader::destroy() {
    m_shader_signature.destroy();
    vkDestroyShaderModule(m_device->getDevice(), m_shader_info.module, nullptr);
}

const VkPipelineShaderStageCreateInfo& VulkanShader::getShaderInfo() const {
    return m_shader_info;
}

const ShaderSignature& VulkanShader::getShaderSignature() const {
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