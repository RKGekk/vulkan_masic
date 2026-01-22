#include "vulkan_shaders_manager.h"

bool VulkanShadersManager::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name) {
    m_device = device;

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_name.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node shaders_node = root_node.child("Shaders");
	if (shaders_node) {
		for (pugi::xml_node shader_node = shaders_node.first_child(); shader_node; shader_node = shader_node.next_sibling()) {
            size_t shader_pos = m_shaders.size();
            m_shaders.push_back(std::make_shared<VulkanShader>());
            m_shaders[shader_pos]->init(device, shader_node);
            m_shader_name_map.insert({m_shaders[shader_pos]->getShaderSignature()->getName(), shader_pos});
		}
	}

    return true;
}

void VulkanShadersManager::destroy() {
    for(std::shared_ptr<VulkanShader>& shader : m_shaders) {
        shader->destroy();
    }
}

std::shared_ptr<VulkanShader> VulkanShadersManager::getShader(const std::string name) const {
    if(!m_shader_name_map.contains(name)) return nullptr;
    size_t shader_pos = m_shader_name_map.at(name);
    return m_shaders.at(shader_pos);
}