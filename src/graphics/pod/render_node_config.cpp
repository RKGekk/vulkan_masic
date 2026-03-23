#include "render_node_config.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"

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
            attachment.attachment_read_image_layout = getImageLayout(attachment_node.child("AttachmentWriteImageLayout").text().as_string());
            m_attachments.push_back(std::move(attachment));
		}
    }
    
    pugi::xml_node update_metadata_node = node_data.child("DescriptorResourcesUpdate");
    if(update_metadata_node) {
        for (pugi::xml_node layout_binding_node = update_metadata_node.first_child(); layout_binding_node; layout_binding_node = layout_binding_node.next_sibling()) {
            if(pugi::xml_node buffer_metadata_node = layout_binding_node.child("Buffer")) {
                UpdateMetadata metadata{};
                metadata.resource_type = RenderResource::Type::BUFFER;
                metadata.buffer_resource_type_name = buffer_metadata_node.child("BufferResourceType").text().as_string();
                m_bindings_metadata.push_back(std::move(metadata));
            }
            else if (pugi::xml_node image_metadata_node = layout_binding_node.child("Image")) {
                UpdateMetadata metadata{};
                metadata.resource_type = RenderResource::Type::IMAGE;
                metadata.image_resource_type_name = image_metadata_node.child("ImageBufferResourceType").text().as_string();
                metadata.image_view_type_name = image_metadata_node.child("ImageViewResourceType").text().as_string();
                metadata.read_image_layout = getImageLayout(image_metadata_node.child("ReadImageLayout").text().as_string());

                if(pugi::xml_node sampler_node = image_metadata_node.child("Sampler")) {
                    std::string sampler_type_str = sampler_node.child("Type").text().as_string();
                    if(sampler_type_str == "Inline"s) metadata.sampler_type = UpdateMetadata::SamplerType::INLINE;
                    else if(sampler_type_str == "Default"s) metadata.sampler_type = UpdateMetadata::SamplerType::DEFAULT;
                    else if(sampler_type_str == "FromImageBuffer"s) metadata.sampler_type = UpdateMetadata::SamplerType::FROM_IMAGEBUFFER;
                    else metadata.sampler_type = UpdateMetadata::SamplerType::NONE;

                    if(metadata.sampler_type == UpdateMetadata::SamplerType::INLINE) {
                        VkSamplerCreateInfo sampler_info = getSamplerCreateInfo(sampler_node.child("Inline"));
                        metadata.image_sampler = std::make_shared<VulkanSampler>(device, m_name + "_inline_sampler"s);
                        metadata.image_sampler->init(sampler_info);
                    }
                }
                m_bindings_metadata.push_back(std::move(metadata));
            }
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

const std::vector<RenderNodeConfig::UpdateMetadata>& RenderNodeConfig::getBindingsMetadata() const {
    return m_bindings_metadata;
}

std::shared_ptr<RenderNodeConfig> RenderNodeConfig::makeInstance(std::string name) const {
    std::shared_ptr<RenderNodeConfig> instance = std::make_shared<RenderNodeConfig>();
    *instance = *this;
    instance->m_name = std::move(name);

    return instance;
}