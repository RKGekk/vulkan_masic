#include "vulkan_framebuffer.h"

#include "vulkan_device.h"
#include "../pod/framebuffer_config.h"
#include "vulkan_render_pass.h"
#include "vulkan_image_buffer.h"
#include "vulkan_buffer.h"

bool VulkanFramebuffer::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<FramebufferConfig> framebuffer_config, std::shared_ptr<VulkanRenderPass> render_pass_ptr, std::function<const std::shared_ptr<RenderResource>&(const LocalName&)> attach_map) {
    m_device = std::move(device);
    m_framebuffer_config = std::move(framebuffer_config);
    m_render_pass_ptr = std::move(render_pass_ptr);
    m_framebuffers_attachments = getAttachments(attach_map);

    m_framebuffer_info = VkFramebufferCreateInfo{};
    m_framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    m_framebuffer_info.renderPass = m_render_pass_ptr->getRenderPass();
    m_framebuffer_info.attachmentCount = static_cast<uint32_t>(m_framebuffers_attachments.size());
    m_framebuffer_info.pAttachments = m_framebuffers_attachments.data();
    m_framebuffer_info.width = m_framebuffer_config->getExtent3D().width;
    m_framebuffer_info.height = m_framebuffer_config->getExtent3D().height;
    m_framebuffer_info.layers = m_framebuffer_config->getExtent3D().depth;

    VkResult result = vkCreateFramebuffer(m_device->getDevice(), &m_framebuffer_info, nullptr, &m_frame_buffer);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void VulkanFramebuffer::destroy() {
    vkDestroyFramebuffer(m_device->getDevice(), m_frame_buffer, nullptr);
}

VkFramebuffer VulkanFramebuffer::getFramebuffer() const {
    return m_frame_buffer;
}

const std::shared_ptr<FramebufferConfig>& VulkanFramebuffer::getFramebufferConfig() const {
    return m_framebuffer_config;
}

const std::vector<VkImageView>& VulkanFramebuffer::getFramebufferAttachments() const {
    return m_framebuffers_attachments;
}

const VkFramebufferCreateInfo& VulkanFramebuffer::getFramebufferInfo() const {
    return m_framebuffer_info;
}

std::vector<VkImageView> VulkanFramebuffer::getAttachments(std::function<const std::shared_ptr<RenderResource>&(const LocalName&)> attach_map) const {
    std::vector<VkImageView> attachments;
    const std::vector<std::shared_ptr<FramebufferConfig::FrameBufferAttachment>>& attachments_config = m_framebuffer_config->getAttachmentsConfig();
    size_t attachments_count = attachments_config.size();
    attachments.resize(attachments_count);
    for(size_t attachment_idx = 0u; attachment_idx < attachments_count; ++attachment_idx) {
        const std::shared_ptr<FramebufferConfig::FrameBufferAttachment>& frame_buffer_attachment = attachments_config.at(attachment_idx);
        if(std::shared_ptr<VulkanImageBuffer> image_resource = std::dynamic_pointer_cast<VulkanImageBuffer>(attach_map(frame_buffer_attachment->attachment_name))) {
            attachments[attachment_idx] = image_resource->getImageBufferView(frame_buffer_attachment->attachment_resource_view);
        }
    }

    return attachments;
}