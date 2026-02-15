#include "render_node.h"

#include "../api/vulkan_render_pass.h"
#include "render_pass_config.h"
#include "../api/vulkan_image_buffer.h"
#include "../api/vulkan_device.h"
#include "../api/vulkan_swapchain.h"
#include "../vulkan_renderer.h"
#include "../../application.h"

#include <utility>

bool RenderNode::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<VulkanPipeline> pipeline) {
    m_device = std::move(device);
    m_pipeline = std::move(pipeline);

    return true;
}

void RenderNode::destroy() {

}

void RenderNode::addReadDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as) {
    AttachmentSlot attachment_slot = {std::move(resource), std::move(attached_as)};
    m_read_resources[resource->getName()] = attachment_slot;
    m_read_attached[attached_as] = std::move(attachment_slot);
}

void RenderNode::addWriteDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as) {
    AttachmentSlot attachment_slot = {std::move(resource), std::move(attached_as)};
    m_written_resources[resource->getName()] = attachment_slot;
    m_written_attached[attached_as] = std::move(attachment_slot);
}

bool RenderNode::isRead(const RenderResource::ResourceName& name) const {
    return m_read_resources.count(name);
}

bool RenderNode::isWritten(const RenderResource::ResourceName& name) const {
    return m_written_resources.count(name);
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

void RenderNode::finishRenderNode() {
    VulkanRenderer& renderer = Application::GetRenderer();


    int max_frames = renderer.getSwapchain()->getMaxFrames();
    m_frame_buffers.resize(max_frames);
    for(int current_frame = 0; current_frame < max_frames; ++current_frame) {
        const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = m_pipeline->getRenderPass();
        const std::shared_ptr<RenderPassConfig>& render_pass_cfg_ptr = render_pass_ptr->getRenderPassConfig();
        std::vector<VkImageView> attachments;
        attachments.resize(render_pass_cfg_ptr->getAttachmentNameMap().size());
        for(const auto&[attach_name, attach_idx] : render_pass_cfg_ptr->getAttachmentNameMap()) {
            if(std::shared_ptr<VulkanImageBuffer> image_resource = std::dynamic_pointer_cast<VulkanImageBuffer>(m_written_attached.at(attach_name).resource)) {
                attachments[attach_idx] = image_resource->getImageBufferView();
                viewport_extent = {image_resource->getImageInfo().extent.width, image_resource->getImageInfo().extent.height};
            }
            else if(std::shared_ptr<VulkanSwapChain> swapchain_resource = std::dynamic_pointer_cast<VulkanSwapChain>(m_written_attached.at(attach_name).resource)) {
                attachments[attach_idx] = swapchain_resource->getSwapchainImages().at(current_frame).getImageBufferView();
                viewport_extent = swapchain_resource->getSwapchainParams().extent;
            }
        }

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass_ptr->getRenderPass();
        framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = viewport_extent.width;
        framebuffer_info.height = viewport_extent.height;
        framebuffer_info.layers = 1u;

        VkResult result = vkCreateFramebuffer(m_device->getDevice(), &framebuffer_info, nullptr, &m_frame_buffers[current_frame]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

const std::shared_ptr<VulkanPipeline>& RenderNode::getPipeline() {
    return m_pipeline;
}

VkFramebuffer RenderNode::getFramebuffer(uint32_t frame_index) const {
    return m_frame_buffers.at(frame_index);
}

VkExtent2D RenderNode::getViewportExtent() const {
    return m_viewport_extent;
}