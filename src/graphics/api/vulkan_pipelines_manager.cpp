#include "vulkan_pipelines_manager.h"

#include "../../application.h"
#include "../../tools/string_tools.h"
#include "vulkan_device.h"
#include "vulkan_descriptors_manager.h"
#include "vulkan_shaders_manager.h"

bool VulkanPipelinesManager::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shaders_manager) {
    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node pipelines_node = root_node.child("Pipelines");
	if (pipelines_node) {
        VkExtent2D viewport_extent = Application::Get().GetRenderer().getSwapchain()->getSwapchainParams().extent;
        //VkRenderPass render_pass;
		for (pugi::xml_node pipeline_node = pipelines_node.first_child(); pipeline_node; pipeline_node = pipeline_node.next_sibling()) {
            std::shared_ptr<VulkanPipeline> pipeline = std::make_shared<VulkanPipeline>();
			pipeline->init(device, pipeline_node, viewport_extent, render_pass, desc_manager, shaders_manager);
            m_pipeline_name_map.insert({pipeline_node.attribute("name").as_string(), std::move(pipeline)});
		}
	}

    return true;
}

void VulkanPipelinesManager::destroy() {
    for(auto&[pipeline_name, pipeline] : m_pipeline_name_map) {
        pipeline->destroy();
    }
}