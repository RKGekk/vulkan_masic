#include "vulkan_render_target.h"

bool RenderTarget::init(std::shared_ptr<VulkanDevice> device, const std::shared_ptr<VulkanSwapChain>& swapchain, int frame_index) {
    m_device = std::move(device);
    m_color_format = swapchain->getSwapchainParams().surface_format.format;
    m_depth_format = m_device->findDepthFormat();
    m_viewport_extent = swapchain->getSwapchainParams().extent;
    m_sharing_mode = swapchain->getSwapchainParams().images_sharing_mode;
    m_msaa_samples = m_device->getMsaaSamples();
    m_exec_counter = 0;

    m_out_swap_chain_image = swapchain->getSwapchainImages().at(frame_index);
    m_out_swap_chain_image.changeLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    m_out_color_image = createColorResource();
    m_out_depth_image = createDepthResource();

    m_attachments.reserve(m_msaa_samples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3);
    m_attachments.push_back(m_msaa_samples == VK_SAMPLE_COUNT_1_BIT ? m_out_swap_chain_image.getImageBufferView() : m_out_color_image.getImageBufferView());
    m_attachments.push_back(m_out_depth_image.getImageBufferView());
    if(m_msaa_samples != VK_SAMPLE_COUNT_1_BIT) {
        m_attachments.push_back(m_out_swap_chain_image.getImageBufferView());
    }

    m_clear_render_pass = createRenderPass(VK_ATTACHMENT_LOAD_OP_CLEAR);
    m_load_render_pass = createRenderPass(VK_ATTACHMENT_LOAD_OP_LOAD);

    m_clear_frame_buffer = createFramebuffer(m_clear_render_pass);
    m_load_frame_buffer = createFramebuffer(m_load_render_pass);

    return true;
}

void RenderTarget::destroy() {
    m_out_color_image.destroy();
    m_out_depth_image.destroy();
}

VkExtent2D RenderTarget::getViewportExtent() const {
    return m_viewport_extent;
}

float RenderTarget::getAspect() const {
    return (float)m_viewport_extent.width / (float)m_viewport_extent.height;
}

const VkFormat RenderTarget::getColorFormat() const {
    return m_color_format;
}

const VkFormat RenderTarget::getDepthFormat() const {
    return m_depth_format;
}

const std::vector<VkImageView>& RenderTarget::getAttachments() const {
    return m_attachments;
}

VkFramebuffer RenderTarget::getFrameBuffer(AttachmentLoadOp load_op) const {
    if(load_op == AttachmentLoadOp::CLEAR) {
        return m_clear_frame_buffer;
    }
    if(load_op == AttachmentLoadOp::LOAD) {
        return m_load_frame_buffer;
    }
    if(m_exec_counter) {
        return m_load_frame_buffer;
    }
    else {
        return m_clear_frame_buffer;
    }
}

VkRenderPass RenderTarget::getRenderPass(AttachmentLoadOp load_op) const {
    if(load_op == AttachmentLoadOp::CLEAR) {
        return m_clear_render_pass;
    }
    if(load_op == AttachmentLoadOp::LOAD) {
        return m_load_render_pass;
    }
    if(m_exec_counter) {
        return m_load_render_pass;
    }
    else {
        return m_clear_render_pass;
    }
    
}

RenderTarget::AttachmentLoadOp RenderTarget::getCurrentLoadOp() const {
    if(m_exec_counter) {
        return AttachmentLoadOp::LOAD;
    }
    return AttachmentLoadOp::CLEAR;
}

void RenderTarget::incExecCounter() {
    ++m_exec_counter;
}

void RenderTarget::resetExecCounter() {
    m_exec_counter = 0;
}

VkFramebuffer RenderTarget::createFramebuffer(VkRenderPass render_pass) const {
    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = m_attachments.size();
    framebuffer_info.pAttachments = m_attachments.data();
    framebuffer_info.width = m_viewport_extent.width;
    framebuffer_info.height = m_viewport_extent.height;
    framebuffer_info.layers = 1u;
    VkFramebuffer frame_buffer;
    VkResult result = vkCreateFramebuffer(m_device->getDevice(), &framebuffer_info, nullptr, &frame_buffer);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
    return frame_buffer;
}

VkRenderPass RenderTarget::createRenderPass(VkAttachmentLoadOp load_op) const {

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0u;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription color_attachment{};
    color_attachment.format = m_color_format;
    color_attachment.samples = m_msaa_samples;
    color_attachment.loadOp = load_op;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = load_op;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = m_msaa_samples == VK_SAMPLE_COUNT_1_BIT ? m_out_swap_chain_image.getLayout() : m_out_color_image.getLayout();
    color_attachment.finalLayout = m_msaa_samples == VK_SAMPLE_COUNT_1_BIT ? m_out_swap_chain_image.getLayout() : m_out_color_image.getLayout();
    
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = m_depth_format;
    depth_attachment.samples = m_msaa_samples;
    depth_attachment.loadOp = load_op;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = load_op;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = m_out_depth_image.getLayout();
    depth_attachment.finalLayout = m_out_depth_image.getLayout();

    VkAttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format = m_color_format;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = load_op;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout = load_op == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR ? VK_IMAGE_LAYOUT_UNDEFINED : m_out_swap_chain_image.getLayout();
    color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0u;
    color_attachment_ref.layout = m_msaa_samples == VK_SAMPLE_COUNT_1_BIT ? m_out_swap_chain_image.getLayout() : m_out_color_image.getLayout(); 
    
    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1u;
    depth_attachment_ref.layout = m_out_depth_image.getLayout();
    
    VkAttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 2u;
    color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass_desc{};
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1u;
    subpass_desc.pColorAttachments = &color_attachment_ref;
    subpass_desc.pDepthStencilAttachment = &depth_attachment_ref;
    subpass_desc.pResolveAttachments = m_msaa_samples == VK_SAMPLE_COUNT_1_BIT ? nullptr : &color_attachment_resolve_ref;
    
    std::vector<VkAttachmentDescription> attachments;
    if(m_msaa_samples != VK_SAMPLE_COUNT_1_BIT) {
        attachments = {color_attachment, depth_attachment, color_attachment_resolve};
    }
    else {
        attachments = {color_attachment, depth_attachment};
    }
    
    std::vector<VkSubpassDescription> subpases = {subpass_desc};
    std::vector<VkSubpassDependency> subdependencies = {subpass_dependency};
    
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = static_cast<uint32_t>(subpases.size());
    render_pass_info.pSubpasses = subpases.data();
    render_pass_info.dependencyCount = static_cast<uint32_t>(subdependencies.size());
    render_pass_info.pDependencies = subdependencies.data();
    
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkResult result = vkCreateRenderPass(m_device->getDevice(), &render_pass_info, nullptr, &render_pass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    
    return render_pass;
}

VulkanImageBuffer RenderTarget::createColorResource() const {
    std::vector<uint32_t> families = m_device->getCommandManager().getQueueFamilyIndices().getIndices();
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(m_viewport_extent.width);
    image_info.extent.height = static_cast<uint32_t>(m_viewport_extent.height);
    image_info.extent.depth = 1u;
    image_info.mipLevels = 1u;
    image_info.arrayLayers = 1u;
    image_info.format = m_color_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.sharingMode = m_sharing_mode;
    image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    image_info.pQueueFamilyIndices = families.data();
    image_info.samples = m_device->getMsaaSamples();
    image_info.flags = 0u;

    VulkanImageBuffer out_color_image;
    out_color_image.init(m_device, nullptr, image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    out_color_image.changeLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    return out_color_image;
}

VulkanImageBuffer RenderTarget::createDepthResource() const {
    VkFormat depth_format = m_device->findDepthFormat();
    VkImageAspectFlags image_aspect = m_device->hasStencilComponent(depth_format) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
    std::vector<uint32_t> families = m_device->getCommandManager().getQueueFamilyIndices().getIndices();
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(m_viewport_extent.width);
    image_info.extent.height = static_cast<uint32_t>(m_viewport_extent.height);
    image_info.extent.depth = 1u;
    image_info.mipLevels = 1u;
    image_info.arrayLayers = 1u;
    image_info.format = depth_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_info.sharingMode = m_sharing_mode;
    image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    image_info.pQueueFamilyIndices = families.data();
    image_info.samples = m_device->getMsaaSamples();
    image_info.flags = 0u;

    VulkanImageBuffer out_depth_image;
    out_depth_image.init(m_device, nullptr, image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_aspect);
    out_depth_image.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    return out_depth_image;
}