#include "render_node.h"

#include "../api/vulkan_render_pass.h"
#include "render_pass_config.h"
#include "graphics_render_node_config.h"
#include "image_buffer_config.h"
#include "buffer_config.h"
#include "render_graph.h"
#include "../api/vulkan_image_buffer.h"
#include "../api/vulkan_buffer.h"
#include "../api/vulkan_device.h"
#include "../api/vulkan_swapchain.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_descriptor.h"
#include "../api/vulkan_pipelines_manager.h"
#include "../api/vulkan_descriptors_manager.h"
#include "../api/vulkan_command_buffer.h"
#include "../vulkan_renderer.h"
#include "../../application.h"
#include "format_config.h"

#include <utility>

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

void RenderNode::changeWriteDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as) {
    AttachmentSlot attachment_slot = {resource, attached_as};
    m_written_resources[resource->getName()] = attachment_slot;
    m_written_attached[attached_as] = std::move(attachment_slot);
    finishRenderNode();
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

std::shared_ptr<RenderResource> RenderNode::getAttachedResource(const RenderNode::LocalName& attached_as) const {
    if(isReadAttached(attached_as)) {
        return m_read_attached.at(attached_as).resource;
    }

    if(isWrittenAttached(attached_as)) {
        return m_written_attached.at(attached_as).resource;
    }

    return nullptr;
}

std::shared_ptr<VulkanImageBuffer> RenderNode::getAttachedImageResource(const RenderNode::LocalName& attached_as) const {
    if(isReadAttached(attached_as)) {
        return getReadAttachedImageResource(attached_as);
    }

    if(isWrittenAttached(attached_as)) {
        return getWrittenAttachedImageResource(attached_as);
    }

    return nullptr;
}

std::shared_ptr<VulkanImageBuffer> RenderNode::getWrittenAttachedImageResource(const RenderNode::LocalName& name) const {
    return std::dynamic_pointer_cast<VulkanImageBuffer>(m_written_attached.at(name).resource);
}

std::shared_ptr<VulkanImageBuffer> RenderNode::getReadAttachedImageResource(const RenderNode::LocalName& name) const {
    return std::dynamic_pointer_cast<VulkanImageBuffer>(m_read_attached.at(name).resource);
}

std::shared_ptr<VulkanBuffer> RenderNode::getAttachedBufferResource(const RenderNode::LocalName& attached_as) const {
    if(isReadAttached(attached_as)) {
        return getWrittenAttachedBufferResource(attached_as);
    }

    if(isWrittenAttached(attached_as)) {
        return getReadAttachedBufferResource(attached_as);
    }

    return nullptr;
}

std::shared_ptr<VulkanBuffer> RenderNode::getWrittenAttachedBufferResource(const RenderNode::LocalName& name) const {
    return std::dynamic_pointer_cast<VulkanBuffer>(m_written_attached.at(name).resource);
}

std::shared_ptr<VulkanBuffer> RenderNode::getReadAttachedBufferResource(const RenderNode::LocalName& name) const {
    return std::dynamic_pointer_cast<VulkanBuffer>(m_read_attached.at(name).resource);
}