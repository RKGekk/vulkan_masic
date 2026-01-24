#include "vulkan_descriptors_manager.h"

#include "vulkan_device.h"

bool VulkanDescriptorsManager::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name) {
    m_device = device;

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_name.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node dscriptors_node = root_node.child("Descriptors");
	if (dscriptors_node) {
		for (pugi::xml_node dscriptor_node = dscriptors_node.first_child(); dscriptor_node; dscriptor_node = dscriptor_node.next_sibling()) {
            std::shared_ptr<DescSetLayout> layout;
			layout->init(m_device, dscriptor_node);
			m_name_layout_map.insert({layout->getName(), layout});
		}
	}

    return true;
}