#include "render_node_config.h"

#include "../api/vulkan_device.h"

bool RenderNodeConfig::init(const std::shared_ptr<VulkanDevice>& device, const std::string& rg_file_path) {
    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node render_nodes_node = root_node.child("RenderNodes");
	if (!render_nodes_node) return false;
    
	for (pugi::xml_node render_node = render_nodes_node.first_child(); render_node; render_node = render_node.next_sibling()) {
	    return init(device, render_node);
	}

    return true;
}

bool RenderNodeConfig::init(const std::shared_ptr<VulkanDevice>& device, const pugi::xml_node& node_data) {
    using namespace std::literals;

    m_name = node_data.attribute("name").as_string();

    m_pipeline_name = node_data.child("Pipeline").text().as_string();
    m_render_pass_name = node_data.child("FrameBuffer").child("RenderPassName").text().as_string();

    pugi::xml_node attachments_node = node_data.child("FrameBuffer").child("Attachments");
    if(attachments_node) {
        for (pugi::xml_node attachment_node = attachments_node.first_child(); attachment_node; attachment_node = attachment_node.next_sibling()) {
            FrameBufferAttachment attachment;
            attachment.attachment_name = attachment_node.child("AttachmentName").text().as_string();
            attachment.attachment_resource_type = attachment_node.child("AttachmentResourceType").text().as_string();
            attachment.attachment_resource_view = attachment_node.child("AttachmentResourceTypeView").text().as_string();

            m_attachments.push_back(std::move(attachment));
		}
    }

    return true;
}


const std::string& RenderNodeConfig::getName() const {
    return m_name;
}

const std::string& RenderNodeConfig::getPipelineName() const {
    return m_pipeline_name;
}

const std::string& RenderNodeConfig::getRenderPassName() const {
    return m_render_pass_name;
}

const std::vector<RenderNodeConfig::FrameBufferAttachment>& RenderNodeConfig::getAttachmentsConfig() const {
    return m_attachments;
}

std::shared_ptr<RenderNodeConfig> RenderNodeConfig::makeInstance(std::string name) const {
    std::shared_ptr<RenderNodeConfig> instance = std::make_shared<RenderNodeConfig>();
    *instance = *this;
    instance->m_name = std::move(name);

    return instance;
}