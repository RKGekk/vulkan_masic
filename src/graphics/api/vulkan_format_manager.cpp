#include "vulkan_format_manager.h"

#include "vulkan_device.h"
#include "vulkan_swapchain.h"

#include <utility>

bool VulkanFormatManager::init(std::shared_ptr<VulkanDevice> device, const std::shared_ptr<WindowSurface>& window, const std::string& rg_file_path) {
	m_device = std::move(device);

    SwapchainSupportDetails swapchain_support_details = VulkanSwapChain::querySwapChainSupport(m_device->getDeviceAbilities().physical_device, m_device->getSurface());

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node formats_node = root_node.child("Formats");
	if (!formats_node) return false;
    
	for (pugi::xml_node format_node = formats_node.first_child(); format_node; format_node = format_node.next_sibling()) {
		std::shared_ptr<FormatConfig> format_cfg_ptr = std::make_shared<FormatConfig>();
	    format_cfg_ptr->init(m_device, window, swapchain_support_details, format_node);
		m_format_map[format_cfg_ptr->getName()] = std::move(format_cfg_ptr);
	}

    return true;
}

std::shared_ptr<FormatConfig> VulkanFormatManager::getFormat(const std::string& name) {
	return m_format_map.at(name);
}