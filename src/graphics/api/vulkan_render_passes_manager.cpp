#include "vulkan_render_passes_manager.h"

#include "vulkan_swapchain.h"
#include "../pod/render_pass_config.h"

bool VulkanRenderPassesManager::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path, const std::shared_ptr<VulkanSwapChain>& swapchain) {
    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node passes_node = root_node.child("RenderPases");
	if (passes_node) {
		for (pugi::xml_node pass_node = passes_node.first_child(); pass_node; pass_node = pass_node.next_sibling()) {
            std::shared_ptr<RenderPassConfig> pass_cfg = std::make_shared<RenderPassConfig>();
            pass_cfg->init(device, swapchain, pass_node);

            std::shared_ptr<VulkanRenderPass> pass = std::make_shared<VulkanRenderPass>();
			pass->init(device, pass_cfg);
            m_render_pass_name_map.insert({pass_node.attribute("name").as_string(), std::move(pass)});
		}
	}

    return true;
}

void VulkanRenderPassesManager::destroy() {
    for(auto& [name, pass] : m_render_pass_name_map) {
        pass->destroy();
    }
}

std::shared_ptr<VulkanRenderPass> VulkanRenderPassesManager::getRenderPass(std::string pass_name) {
    return m_render_pass_name_map.at(pass_name);
}