#include "render_node.h"

#include "../api/vulkan_render_pass.h"
#include "render_pass_config.h"
#include "render_node_config.h"
#include "image_buffer_config.h"
#include "buffer_config.h"
#include "../api/vulkan_image_buffer.h"
#include "../api/vulkan_buffer.h"
#include "../api/vulkan_device.h"
#include "../api/vulkan_swapchain.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_descriptor.h"
#include "../api/vulkan_pipelines_manager.h"
#include "../api/vulkan_descriptors_manager.h"
#include "../vulkan_renderer.h"
#include "../../application.h"
#include "format_config.h"

#include <utility>

bool RenderNode::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<RenderNodeConfig> node_config) {
    m_device = std::move(device);
    m_node_config = std::move(node_config);
    m_pipeline = Application::Get().GetRenderer().getPipelinesManager()->getPipeline(m_node_config->getPipelineName());

    return true;
}

void RenderNode::destroy() {
    
}

void RenderNode::addReadDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as, bool only_read) {
    AttachmentSlot attachment_slot = {std::move(resource), std::move(attached_as)};
    m_read_resources[attachment_slot.resource->getName()] = attachment_slot;
    m_read_attached[attachment_slot.attached_as] = std::move(attachment_slot);
}

void RenderNode::addWriteDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as) {
    AttachmentSlot attachment_slot = {resource, attached_as};
    m_written_resources[resource->getName()] = attachment_slot;
    m_written_attached[attached_as] = std::move(attachment_slot);
}

bool RenderNode::isReadGlobal(const GlobalName& name) const {
    return m_read_resources.count(name);
}

bool RenderNode::isWrittenGlobal(const GlobalName& name) const {
    return m_written_resources.count(name);
}

bool RenderNode::isReadAttached(const LocalName& name) const {
    return m_read_attached.count(name);
}

bool RenderNode::isWrittenAttached(const LocalName& name) const {
    return m_written_attached.count(name);
}

const RenderNode::ResourceMap& RenderNode::getReadResourcesMap() const {
    return m_read_resources;
}

const RenderNode::AttachMap& RenderNode::getReadAttachmentMap() const {
    return m_read_attached;
}

const RenderNode::ResourceMap& RenderNode::getWrittenResourcesMap() const {
    return m_written_resources;
}

const RenderNode::AttachMap& RenderNode::getWrittenAttachmentMap() const {
    return m_written_attached;
}

std::vector<VkImageView> RenderNode::getAttachments() const {
    std::vector<VkImageView> attachments;
    const std::vector<RenderNodeConfig::FrameBufferAttachment>& attachments_config = m_node_config->getAttachmentsConfig();
    size_t attachments_count = attachments_config.size();
    attachments.resize(attachments_count);
    for(size_t attachment_idx = 0u; attachment_idx < attachments_count; ++attachment_idx) {
        const RenderNodeConfig::FrameBufferAttachment& frame_buffer_attachment = attachments_config.at(attachment_idx);
        if(std::shared_ptr<VulkanImageBuffer> image_resource = std::dynamic_pointer_cast<VulkanImageBuffer>(m_written_attached.at(frame_buffer_attachment.attachment_name).resource)) {
            attachments[attachment_idx] = image_resource->getImageBufferView(frame_buffer_attachment.attachment_resource_view);
        }
    }

    return attachments;
}

void RenderNode::finishRenderNode() {
    VulkanRenderer& renderer = Application::GetRenderer();

    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = m_pipeline->getRenderPass();
    m_framebuffers_attachments = getAttachments();

    m_viewport_extent = getWrittenAttachedImageResource(m_node_config->getAttachmentsConfig().front().attachment_name)->getImageConfig()->getFormat()->getExtent2D();

    m_framebuffer_info = VkFramebufferCreateInfo{};
    m_framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    m_framebuffer_info.renderPass = render_pass_ptr->getRenderPass();
    m_framebuffer_info.attachmentCount = static_cast<uint32_t>(m_framebuffers_attachments.size());
    m_framebuffer_info.pAttachments = m_framebuffers_attachments.data();
    m_framebuffer_info.width = m_viewport_extent.width;
    m_framebuffer_info.height = m_viewport_extent.height;
    m_framebuffer_info.layers = 1u;

    VkResult result = vkCreateFramebuffer(m_device->getDevice(), &m_framebuffer_info, nullptr, &m_frame_buffer);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }

    for (const auto&[slot, desc_set_layout] : m_pipeline->getDescLayouts()) {
        m_descs[slot] = renderer.getDescriptorsManager()->allocateDescriptorSet(desc_set_layout->getName());
        for(const VkDescriptorSetLayoutBinding& binding : desc_set_layout->getBindings()) {
            const std::string& binding_name = desc_set_layout->getBindingName(binding.binding);
            const std::shared_ptr<RenderNodeConfig::UpdateMetadata>& binding_metadata = m_node_config->getUpdateMetadata(binding_name);
            if(binding_metadata->resource_type == RenderResource::Type::IMAGE) {
                std::shared_ptr<VulkanImageBuffer> image_to_bind = getReadAttachedImageResource(binding_name);
                m_descs[slot]->updateDescImageInfo(
                    binding.binding,
                    image_to_bind->getImageConfig()->getSampler()->getSampler(),
                    image_to_bind->getImageBufferView(binding_metadata->image_view_type_name),
                    binding_metadata->read_image_layout
                );
            }
            else if(binding_metadata->resource_type == RenderResource::Type::BUFFER) {
                std::shared_ptr<VulkanBuffer> buffer_to_bind = getReadAttachedBufferResource(binding_name);
                m_descs[slot]->updateDescBuffer(binding.binding, buffer_to_bind->getBuffer());
            }
        }
    }
    
}

const std::shared_ptr<VulkanPipeline>& RenderNode::getPipeline() {
    return m_pipeline;
}

VkFramebuffer RenderNode::getFramebuffer() const {
    return m_frame_buffer;
}

VkExtent2D RenderNode::getViewportExtent() const {
    return m_viewport_extent;
}

std::shared_ptr<RenderResource> RenderNode::getAttachedResource(const RenderNode::LocalName& attached_as) const {
    if(isReadAttached(attached_as)) {
        return m_read_attached.at(attached_as).resource;
    }

    if(isWrittenAttached(attached_as)) {
        return m_written_attached.at(attached_as).resource;
    }

    return nullptr;
}

std::shared_ptr<VulkanImageBuffer> RenderNode::getAttachedImageResource(const RenderNode::LocalName& attached_as) {
    if(isReadAttached(attached_as)) {
        return getWrittenAttachedImageResource(attached_as);
    }

    if(isWrittenAttached(attached_as)) {
        return getReadAttachedImageResource(attached_as);
    }

    return nullptr;
}

std::shared_ptr<VulkanImageBuffer> RenderNode::getWrittenAttachedImageResource(const RenderNode::LocalName& name) {
    return std::dynamic_pointer_cast<VulkanImageBuffer>(m_written_attached.at(name).resource);
}

std::shared_ptr<VulkanImageBuffer> RenderNode::getReadAttachedImageResource(const RenderNode::LocalName& name) {
    return std::dynamic_pointer_cast<VulkanImageBuffer>(m_read_attached.at(name).resource);
}

std::shared_ptr<VulkanBuffer> RenderNode::getAttachedBufferResource(const RenderNode::LocalName& attached_as) {
    if(isReadAttached(attached_as)) {
        return getWrittenAttachedBufferResource(attached_as);
    }

    if(isWrittenAttached(attached_as)) {
        return getReadAttachedBufferResource(attached_as);
    }

    return nullptr;
}

std::shared_ptr<VulkanBuffer> RenderNode::getWrittenAttachedBufferResource(const RenderNode::LocalName& name) {
    return std::dynamic_pointer_cast<VulkanBuffer>(m_written_attached.at(name).resource);
}

std::shared_ptr<VulkanBuffer> RenderNode::getReadAttachedBufferResource(const RenderNode::LocalName& name) {
    return std::dynamic_pointer_cast<VulkanBuffer>(m_read_attached.at(name).resource);
}

const std::unordered_map<uint32_t, std::shared_ptr<VulkanDescriptor>>& RenderNode::getDescriptors() const {
    return m_descs;
}

const std::shared_ptr<RenderNodeConfig>& RenderNode::getRenderNodeConfig() const {
    return m_node_config;
}