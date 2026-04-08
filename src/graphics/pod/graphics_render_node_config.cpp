#include "graphics_render_node_config.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"
#include "../api/vulkan_resources_manager.h"
#include "image_buffer_config.h"
#include "format_config.h"

bool GraphicsRenderNodeConfig::init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<VulkanResourcesManager>& resources_manager, const pugi::xml_node& node_data) {
    using namespace std::literals;

    m_name = node_data.attribute("name").as_string();

    m_pipeline_name = node_data.child("Pipeline").text().as_string();
    m_render_pass_name = node_data.child("FrameBuffer").child("RenderPassName").text().as_string();

    pugi::xml_node attachments_node = node_data.child("FrameBuffer").child("Attachments");
    if(attachments_node) {
        for (pugi::xml_node attachment_node = attachments_node.first_child(); attachment_node; attachment_node = attachment_node.next_sibling()) {
            std::shared_ptr<FrameBufferAttachment> attachment = std::make_shared<FrameBufferAttachment>();
            attachment->attachment_name = attachment_node.attribute("name").as_string();
            attachment->attachment_resource_type = attachment_node.child("AttachmentResourceType").text().as_string();
            attachment->attachment_resource_view = attachment_node.child("AttachmentResourceTypeView").text().as_string();

            size_t attach_idx = m_attachments.size();
            m_name_attach_map[attachment->attachment_name] = attach_idx;
            m_attachments.push_back(std::move(attachment));
		}
    }
    
    pugi::xml_node update_metadata_node = node_data.child("DescriptorResourcesUpdate");
    if(update_metadata_node) {
        for (pugi::xml_node layout_binding_node = update_metadata_node.first_child(); layout_binding_node; layout_binding_node = layout_binding_node.next_sibling()) {
            std::string layout_binding_name = layout_binding_node.attribute("name").as_string();
            std::shared_ptr<UpdateMetadata> metadata = std::make_shared<UpdateMetadata>();
            metadata->name = layout_binding_name;
            if(pugi::xml_node buffer_metadata_node = layout_binding_node.child("Buffer")) {
                metadata->resource_type = RenderResource::Type::BUFFER;
                metadata->buffer_resource_type_name = buffer_metadata_node.child("BufferResourceType").text().as_string();
            }
            else if (pugi::xml_node image_metadata_node = layout_binding_node.child("Image")) {
                metadata->resource_type = RenderResource::Type::IMAGE;
                metadata->image_resource_type_name = image_metadata_node.child("ImageBufferResourceType").text().as_string();
                metadata->image_view_type_name = image_metadata_node.child("ImageViewResourceType").text().as_string();
                metadata->read_image_layout = getImageLayout(image_metadata_node.child("ReadImageLayout").text().as_string());

                if(pugi::xml_node sampler_node = image_metadata_node.child("Sampler")) {
                    std::string sampler_type_str = sampler_node.child("Type").text().as_string();
                    if(sampler_type_str == "Inline"s) metadata->sampler_type = UpdateMetadata::SamplerType::INLINE;
                    else if(sampler_type_str == "Default"s) metadata->sampler_type = UpdateMetadata::SamplerType::DEFAULT;
                    else if(sampler_type_str == "FromImageBuffer"s) metadata->sampler_type = UpdateMetadata::SamplerType::FROM_IMAGEBUFFER;
                    else metadata->sampler_type = UpdateMetadata::SamplerType::NONE;

                    if(metadata->sampler_type == UpdateMetadata::SamplerType::INLINE) {
                        VkSamplerCreateInfo sampler_info = getSamplerCreateInfo(sampler_node.child("Inline"));
                        metadata->image_sampler = std::make_shared<VulkanSampler>(device, m_name + "_inline_sampler"s);
                        metadata->image_sampler->init(sampler_info);
                    }
                }
            }
            else {
                metadata = nullptr;
            }
            m_bindings_metadata[layout_binding_name] = std::move(metadata);
        }
    }

    return true;
}


const std::string& GraphicsRenderNodeConfig::getName() const {
    return m_name;
}

const std::string& GraphicsRenderNodeConfig::getPipelineName() const {
    return m_pipeline_name;
}

const std::string& GraphicsRenderNodeConfig::getRenderPassName() const {
    return m_render_pass_name;
}

const std::vector<std::shared_ptr<GraphicsRenderNodeConfig::FrameBufferAttachment>>& GraphicsRenderNodeConfig::getAttachmentsConfig() const {
    return m_attachments;
}

const std::shared_ptr<GraphicsRenderNodeConfig::FrameBufferAttachment>& GraphicsRenderNodeConfig::getAttachmentData(const std::string& attached_name) const {
    return m_attachments.at(m_name_attach_map.at(attached_name));
}

const std::unordered_map<std::string, std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>>& GraphicsRenderNodeConfig::getBindingsMetadata() const {
    return m_bindings_metadata;
}

std::shared_ptr<GraphicsRenderNodeConfig> GraphicsRenderNodeConfig::makeInstance(std::string name) const {
    std::shared_ptr<GraphicsRenderNodeConfig> instance = std::make_shared<GraphicsRenderNodeConfig>();
    *instance = *this;
    instance->m_name = std::move(name);

    return instance;
}

const std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>& GraphicsRenderNodeConfig::getUpdateMetadata(const std::string& binding_name) {
    return m_bindings_metadata.at(binding_name);
}