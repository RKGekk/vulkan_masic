#define STB_IMAGE_IMPLEMENTATION
#include "vulkan_renderer.h"

bool VulkanRenderer::init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, std::shared_ptr<VulkanCommandManager> command_manager, const std::string& texture_path, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) {
    m_device = std::move(device);
    m_command_manager = std::move(command_manager);

    m_swapchain.init(m_device, surface, window);

    createColorResources();
    createDepthResources();

    loadShaders();

    createRenderPass();
    m_desc_set_layout = createDescSetLayout();
    createPipeline(m_device->getDevice(), m_vert_shader_modeule, m_frag_shader_modeule, m_render_pass, m_swapchain.getSwapchainParams(), m_desc_set_layout, m_device->getMsaaSamples());
    m_out_framebuffers = createFramebuffers();

    m_texture_image = createImage(m_command_manager->getGrapicsCommandPool(), m_device->getGraphicsQueue(), texture_path);
    m_texture_view = createImageView(m_device->getDevice(), m_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mip_levels);
    m_texture_sampler = createTextureSampler(m_mip_levels);

    m_indices_count = indices.size();
    createAndTransferVertexBuffer(m_command_manager->getTransferCommandPool(), m_device->getTransferQueue(), vertices);
    createAndTransferIndexBuffer(m_command_manager->getTransferCommandPool(), m_device->getTransferQueue(), indices);

    createUniformBuffers(m_device->getDevice());
    
    m_desc_pool = createDescPool(m_device->getDevice());
    createDescSets(m_device->getDevice());

    m_command_buffers.resize(m_swapchain.getMaxFrames());
    VulkanCommandManager::allocCommandBuffer(m_device->getDevice(), m_command_manager->getGrapicsCommandPool(), m_command_buffers);
    createSyncObjects();

    return true;
}

void VulkanRenderer::destroy() {
    vkDeviceWaitIdle(m_device->getDevice());

    m_swapchain.destroy();
    
    size_t sz = m_out_framebuffers.size();
    for(size_t i = 0u; i < sz; ++i) {
        vkDestroyImageView(m_device->getDevice(), m_out_color_images[i].image_view, nullptr);
        vkDestroyImage(m_device->getDevice(), m_out_color_images[i].image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_out_color_images[i].memory, nullptr);
        
        vkDestroyImageView(m_device->getDevice(), m_out_depth_images[i].image_view, nullptr);
        vkDestroyImage(m_device->getDevice(), m_out_depth_images[i].image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_out_depth_images[i].memory, nullptr);
        
        vkDestroyFramebuffer(m_device->getDevice(), m_out_framebuffers[i], nullptr);

        vkDestroyBuffer(m_device->getDevice(), m_uniform_buffers[i], nullptr);
        vkFreeMemory(m_device->getDevice(), m_uniform_memory[i], nullptr);

        vkDestroySemaphore(m_device->getDevice(), m_image_available[i], nullptr);
        vkDestroySemaphore(m_device->getDevice(), m_render_finished[i], nullptr);
        vkDestroyFence(m_device->getDevice(), m_in_flight_frame[i], nullptr);
    }

    vkDestroyBuffer(m_device->getDevice(), m_vertex_buffer, nullptr);
    vkFreeMemory(m_device->getDevice(), m_vertex_memory, nullptr);
    
    vkDestroyBuffer(m_device->getDevice(), m_index_buffer, nullptr);
    vkFreeMemory(m_device->getDevice(), m_index_memory, nullptr);
     
    vkDestroySampler(m_device->getDevice(), m_texture_sampler, nullptr);
    vkDestroyImageView(m_device->getDevice(), m_texture_view, nullptr);
    vkDestroyImage(m_device->getDevice(), m_texture_image, nullptr);
    vkFreeMemory(m_device->getDevice(), m_texture_memory, nullptr);
    
    vkDestroyRenderPass(m_device->getDevice(), m_render_pass, nullptr);
    vkDestroyPipeline(m_device->getDevice(), m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device->getDevice(), m_pipeline_layout, nullptr);

    vkDestroyShaderModule(m_device->getDevice(), m_frag_shader_modeule, nullptr);
    vkDestroyShaderModule(m_device->getDevice(), m_vert_shader_modeule, nullptr);
}

void VulkanRenderer::recreate() {
    vkDeviceWaitIdle(m_device->getDevice());

    m_swapchain.recreate();
    
    size_t sz = m_out_framebuffers.size();
    for(size_t i = 0u; i < sz; ++i) {
        vkDestroyImageView(m_device->getDevice(), m_out_color_images[i].image_view, nullptr);
        vkDestroyImage(m_device->getDevice(), m_out_color_images[i].image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_out_color_images[i].memory, nullptr);
        
        vkDestroyImageView(m_device->getDevice(), m_out_depth_images[i].image_view, nullptr);
        vkDestroyImage(m_device->getDevice(), m_out_depth_images[i].image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_out_depth_images[i].memory, nullptr);
        
        vkDestroyFramebuffer(m_device->getDevice(), m_out_framebuffers[i], nullptr);

        vkDestroySemaphore(m_device->getDevice(), m_image_available[i], nullptr);
        vkDestroySemaphore(m_device->getDevice(), m_render_finished[i], nullptr);
        vkDestroyFence(m_device->getDevice(), m_in_flight_frame[i], nullptr);
    }
    
    vkDestroyRenderPass(m_device->getDevice(), m_render_pass, nullptr);
    vkDestroyPipeline(m_device->getDevice(), m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device->getDevice(), m_pipeline_layout, nullptr);

    createColorResources();
    createDepthResources();

    createRenderPass();
    m_desc_set_layout = createDescSetLayout();
    createPipeline(m_device->getDevice(), m_vert_shader_modeule, m_frag_shader_modeule, m_render_pass, m_swapchain.getSwapchainParams(), m_desc_set_layout, m_device->getMsaaSamples());
    m_out_framebuffers = createFramebuffers();

    createSyncObjects();
}

const VulkanSwapChain& VulkanRenderer::getSwapchain() {
    return m_swapchain;
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0u;
    begin_info.pInheritanceInfo = nullptr;
    
    VulkanCommandManager::beginCommandBuffer(command_buffer);
    
    VkRenderPassBeginInfo renderpass_info{};
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_info.renderPass = m_render_pass;
    renderpass_info.framebuffer = m_out_framebuffers[image_index];
    renderpass_info.renderArea.offset = {0, 0};
    renderpass_info.renderArea.extent = m_swapchain.getSwapchainParams().extent;
    
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};
    renderpass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    renderpass_info.pClearValues = clear_values.data();
    
    vkCmdBeginRenderPass(command_buffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
    
    VkBuffer vertex_buffers[] = {m_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, m_index_buffer, 0u, VK_INDEX_TYPE_UINT16);
    
    VkViewport view_port{};
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = static_cast<float>(m_swapchain.getSwapchainParams().extent.width);
    view_port.height = static_cast<float>(m_swapchain.getSwapchainParams().extent.height);
    view_port.minDepth = 0.0f;
    view_port.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0u, 1u, &view_port);
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain.getSwapchainParams().extent;
    vkCmdSetScissor(command_buffer, 0u, 1u, &scissor);
    
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &m_desc_sets[m_swapchain.getCurrentFrame()], 0, nullptr);
    
    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(m_indices_count), 1u, 0u, 0u, 0u);
    vkCmdEndRenderPass(command_buffer);
    
    VulkanCommandManager::endCommandBuffer(command_buffer);
}

void VulkanRenderer::drawFrame() {
    vkWaitForFences(m_device->getDevice(), 1u, &m_in_flight_frame[m_swapchain.getCurrentFrame()], VK_TRUE, UINT64_MAX);
        
    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(m_device->getDevice(), m_swapchain.getSwapchain(), UINT64_MAX, m_image_available[m_swapchain.getCurrentFrame()], VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized) {
        recreate();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Only reset the fence if we are submitting work
    vkResetFences(m_device->getDevice(), 1u, &m_in_flight_frame[m_swapchain.getCurrentFrame()]);
    
    vkResetCommandBuffer(m_command_buffers[m_swapchain.getCurrentFrame()], 0u);
    recordCommandBuffer(m_command_buffers[m_swapchain.getCurrentFrame()], image_index);
    
    VkSemaphore render_end_semaphores[] = {m_render_finished[m_swapchain.getCurrentFrame()]};
    VkSemaphore wait_semaphores[] = {m_image_available[m_swapchain.getCurrentFrame()]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1u;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = &m_command_buffers[m_swapchain.getCurrentFrame()];
    submit_info.signalSemaphoreCount = 1u;
    submit_info.pSignalSemaphores = render_end_semaphores;
    
    VulkanCommandManager::submitCommandBuffer(m_device->getGraphicsQueue(), submit_info, m_in_flight_frame[m_swapchain.getCurrentFrame()]);
    
    VkSwapchainKHR swapchains[] = {m_swapchain.getSwapchain()};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1u;
    present_info.pWaitSemaphores = render_end_semaphores;
    present_info.swapchainCount = 1u;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    result = vkQueuePresentKHR(m_device->getPresentQueue(), &present_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    
    m_swapchain.setNextFrame();
}

void VulkanRenderer::setFramebufferResized() {
    m_framebuffer_resized = true;
}

void VulkanRenderer::update_frame(const UniformBufferObject& ubo) {
    memcpy(m_uniform_mapped[m_swapchain.getCurrentFrame()], &ubo, sizeof(ubo));
}

std::vector<VkImageView> VulkanRenderer::getImageViews(VkDevice device, const std::vector<VkImage>& images, VkSurfaceFormatKHR surface_format) {
    uint32_t sz = static_cast<uint32_t>(images.size());
    std::vector<VkImageView> image_views(sz);
    for(uint32_t i = 0u; i < sz; ++i) {
        image_views[i] = createImageView(device, images[i], surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1u);
    }
    
    return image_views;
}

VkImageView VulkanRenderer::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0u;
    view_info.subresourceRange.levelCount = mip_levels;
    view_info.subresourceRange.baseMipLevel = 0u;
    view_info.subresourceRange.layerCount = 1u;
    
    VkImageView image_view;
    VkResult result = vkCreateImageView(device, &view_info, nullptr, &image_view);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    
    return image_view;
}

void VulkanRenderer::createImage(const std::shared_ptr<VulkanDevice>& device_ptr, VkDeviceMemory& memory, VkImage& image, const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties) {
    VkResult result = vkCreateImage(device_ptr->getDevice(), &image_info, nullptr, &image);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    
    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(device_ptr->getDevice(), image, &mem_req);
    
    uint32_t mem_type_idx = findMemoryType(device_ptr, mem_req.memoryTypeBits, properties);
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;
    
    result = vkAllocateMemory(device_ptr->getDevice(), &alloc_info, nullptr, &memory);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    vkBindImageMemory(device_ptr->getDevice(), image, memory, 0u);
}

ImageBuffer VulkanRenderer::createImage(const std::shared_ptr<VulkanDevice>& device_ptr, const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags, uint32_t mip_levels) {
    ImageBuffer image_buffer;
    createImage(device_ptr, image_buffer.memory, image_buffer.image, image_info, properties);
    image_buffer.image_view = createImageView(device_ptr->getDevice(), image_buffer.image, image_info.format, VK_IMAGE_ASPECT_COLOR_BIT, 1u);
    return image_buffer;
}

uint32_t VulkanRenderer::findMemoryType(const std::shared_ptr<VulkanDevice>& device_ptr, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_prop{};
    vkGetPhysicalDeviceMemoryProperties(device_ptr->getDeviceAbilities().physical_device, &mem_prop);
    for(uint32_t i = 0u; i < mem_prop.memoryTypeCount; ++i) {
        bool is_type_suit = type_filter & (1 << i);
        bool is_type_adequate = mem_prop.memoryTypes[i].propertyFlags & properties;
        if(is_type_suit && is_type_adequate) {
            return i;
        }
    }
            
    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanRenderer::transitionImageLayout(VkDevice device, VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) {
    VkCommandBuffer command_buffer = VulkanCommandManager::beginSingleTimeCommands(device, cmd_pool);
    
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;
    
    VkImageMemoryBarrier barrier{};
    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0u;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0u;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    //barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0u;
    barrier.subresourceRange.levelCount = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0u;
    barrier.subresourceRange.layerCount = 1u;
    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
    
    VulkanCommandManager::endSingleTimeCommands(device, command_buffer, queue, cmd_pool);
}

bool VulkanRenderer::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat VulkanRenderer::findDepthFormat(const std::shared_ptr<VulkanDevice>& device_ptr) {
    return device_ptr->findSupportedFormat(
        {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkShaderModule VulkanRenderer::CreateShaderModule(VkDevice device, const std::string& path) {
    auto shader_buff = readFile(path);
    VkShaderModule shader_modeule = CreateShaderModule(device, shader_buff);
    return shader_modeule;
}

VkShaderModule VulkanRenderer::CreateShaderModule(VkDevice device, const std::vector<char>& buffer) {
    VkShaderModuleCreateInfo shader_module_info{};
    shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.codeSize = static_cast<uint32_t>(buffer.size());
    shader_module_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(device, &shader_module_info, nullptr, &shader_module);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader!");
    }
    return shader_module;
}

VkRenderPass VulkanRenderer::createRenderPass(const std::shared_ptr<VulkanDevice>& device_ptr, const SwapchainParams& swapchain_params, VkSubpassDependency subpass_dependency) {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain_params.surface_format.format;
    color_attachment.samples = device_ptr->getMsaaSamples();
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0u;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = findDepthFormat(device_ptr);
    depth_attachment.samples = device_ptr->getMsaaSamples();
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1u;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format = swapchain_params.surface_format.format;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 2u;
    color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass_desc{};
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1u;
    subpass_desc.pColorAttachments = &color_attachment_ref;
    subpass_desc.pDepthStencilAttachment = &depth_attachment_ref;
    subpass_desc.pResolveAttachments = &color_attachment_resolve_ref;
    
    std::array<VkAttachmentDescription, 3> attachments = {color_attachment, depth_attachment, color_attachment_resolve};
    std::array<VkSubpassDescription, 1> subpases = {subpass_desc};
    std::array<VkSubpassDependency, 1> subdependencies = {subpass_dependency};
    
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = static_cast<uint32_t>(subpases.size());
    render_pass_info.pSubpasses = subpases.data();
    render_pass_info.dependencyCount = static_cast<uint32_t>(subdependencies.size());
    render_pass_info.pDependencies = subdependencies.data();
    
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkResult result = vkCreateRenderPass(device_ptr->getDevice(), &render_pass_info, nullptr, &render_pass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    
    return render_pass;
}

void VulkanRenderer::createBuffer(const std::shared_ptr<VulkanDevice>& device_ptr, VkDeviceSize size, QueueFamilyIndices queue_family_indices, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory) {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = queue_family_indices.getBufferSharingMode();
    
    VkResult result = vkCreateBuffer(device_ptr->getDevice(), &buffer_info, nullptr, &buffer);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }
    
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device_ptr->getDevice(), buffer, &mem_req);
    uint32_t mem_type_idx = findMemoryType(device_ptr, mem_req.memoryTypeBits, properties);
    
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;
    
    result = vkAllocateMemory(device_ptr->getDevice(), &alloc_info, nullptr, &memory);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    
    vkBindBufferMemory(device_ptr->getDevice(), buffer, memory, 0u);
}

void VulkanRenderer::copyBufferToImage(VkDevice device, VkCommandPool cmd_pool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer command_buffer = VulkanCommandManager::beginSingleTimeCommands(device, cmd_pool);
    
    VkBufferImageCopy region{};
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0u;
    region.imageSubresource.baseArrayLayer = 0u;
    region.imageSubresource.layerCount = 1u;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
    
    VulkanCommandManager::endSingleTimeCommands(device, command_buffer, queue, cmd_pool);
}

void VulkanRenderer::createColorResources() {
    VkFormat color_format = m_swapchain.getSwapchainParams().surface_format.format;
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(m_swapchain.getSwapchainParams().extent.width);
    image_info.extent.height = static_cast<uint32_t>(m_swapchain.getSwapchainParams().extent.height);
    image_info.extent.depth = 1u;
    image_info.mipLevels = 1u;
    image_info.arrayLayers = 1u;
    image_info.format = color_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.sharingMode = m_swapchain.getSwapchainParams().images_sharing_mode;
    image_info.samples = m_device->getMsaaSamples();
    image_info.flags = 0u;

    m_out_color_images.resize(m_swapchain.getMaxFrames());
    for(uint32_t i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        m_out_color_images[i] = createImage(m_device, image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1u);
    }
}

void VulkanRenderer::createDepthResources() {
    VkFormat depth_format = findDepthFormat(m_device);
    
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(m_swapchain.getSwapchainParams().extent.width);
    image_info.extent.height = static_cast<uint32_t>(m_swapchain.getSwapchainParams().extent.height);
    image_info.extent.depth = 1u;
    image_info.mipLevels = 1u;
    image_info.arrayLayers = 1u;
    image_info.format = depth_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_info.sharingMode = m_swapchain.getSwapchainParams().images_sharing_mode;
    image_info.samples = m_device->getMsaaSamples();
    image_info.flags = 0u;

    m_out_depth_images.resize(m_swapchain.getMaxFrames());
    for(uint32_t i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        m_out_depth_images[i] = createImage(m_device, image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, 1u);
        transitionImageLayout(m_device->getDevice(), m_command_manager->getGrapicsCommandPool(), m_device->getGraphicsQueue(), m_out_depth_images[i].image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1u);
    }
}

void VulkanRenderer::loadShaders() {
    m_frag_shader_modeule = CreateShaderModule(m_device->getDevice(), "shaders/shader.frag.spv");
    m_vert_shader_modeule = CreateShaderModule(m_device->getDevice(), "shaders/shader.vert.spv");
}

VkDescriptorSetLayout VulkanRenderer::createDescSetLayout() {
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0u;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1u;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1u;
    sampler_layout_binding.descriptorCount = 1u;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding, sampler_layout_binding};
 
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();
    
    VkDescriptorSetLayout desc_set_layout = VK_NULL_HANDLE;
    VkResult result = vkCreateDescriptorSetLayout(m_device->getDevice(), &layout_info, nullptr, &desc_set_layout);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    
    return desc_set_layout;
}

void VulkanRenderer::createRenderPass() {
    VkSubpassDependency pass_dependency{};
    pass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    pass_dependency.dstSubpass = 0;
    pass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    pass_dependency.srcAccessMask = 0u;
    pass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    pass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    m_render_pass = createRenderPass(m_device, m_swapchain.getSwapchainParams(), pass_dependency);
}

void VulkanRenderer::createPipeline(VkDevice device, VkShaderModule vert_shader_module, VkShaderModule frag_shader_module, VkRenderPass render_pass, SwapchainParams swapchain_params, VkDescriptorSetLayout desc_set_layout, VkSampleCountFlagBits msaa_samples) {
    VkPipelineShaderStageCreateInfo frag_shader_info{};
    frag_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_info.module = frag_shader_module;
    frag_shader_info.pName = "main";
    frag_shader_info.pSpecializationInfo = nullptr; // fill constants

    VkPipelineShaderStageCreateInfo vertex_shader_info{};
    vertex_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_info.module = vert_shader_module;
    vertex_shader_info.pName = "main";
    vertex_shader_info.pSpecializationInfo = nullptr;
    VkPipelineShaderStageCreateInfo shader_stages[] = {frag_shader_info, vertex_shader_info};
    
    std::array<VkDynamicState, 2> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamic_state_info{};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_info.pDynamicStates = dynamic_states.data();
    
    VkVertexInputBindingDescription binding_desc = Vertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attribute_desc = Vertex::getAttributeDescritpions();
    
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
    depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_info.depthTestEnable = VK_TRUE;
    depth_stencil_info.depthWriteEnable = VK_TRUE;
    depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_info.minDepthBounds = 0.0f;
    depth_stencil_info.maxDepthBounds = 1.0f;
    depth_stencil_info.stencilTestEnable = VK_FALSE;
    depth_stencil_info.front = {};
    depth_stencil_info.back = {};
    
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1u;
    vertex_input_info.pVertexBindingDescriptions = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();
    
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain_params.extent;
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchain_params.extent.width;
    viewport.height = (float)swapchain_params.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkPipelineViewportStateCreateInfo viewport_state_info{};
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.viewportCount = 1u;
    viewport_state_info.pViewports = &viewport;
    viewport_state_info.scissorCount = 1u;
    viewport_state_info.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterizer_info{};
    rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_info.depthBiasClamp = VK_FALSE;
    rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
    //rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_info.depthBiasEnable = VK_FALSE;
    rasterizer_info.depthBiasConstantFactor = 0.0f;
    rasterizer_info.depthBiasClamp = 0.0f;
    rasterizer_info.depthBiasSlopeFactor = 0.0f;
    rasterizer_info.lineWidth = 1.0f;
    
    VkPipelineMultisampleStateCreateInfo multisample_info{};
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.sampleShadingEnable = VK_FALSE;
    multisample_info.rasterizationSamples = msaa_samples;
    //multisample_info.minSampleShading = 1.0f;
    //multisample_info.pSampleMask = nullptr;
    //multisample_info.alphaToCoverageEnable = VK_FALSE;
    //multisample_info.alphaToOneEnable = VK_FALSE;
    
    VkPipelineColorBlendAttachmentState color_blend_state{};
    color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_state.blendEnable = VK_FALSE;
    color_blend_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_state.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo color_blend_info{};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount = 1u;
    color_blend_info.pAttachments = &color_blend_state;
    color_blend_info.blendConstants[0] = 0.0f;
    color_blend_info.blendConstants[1] = 0.0f;
    color_blend_info.blendConstants[2] = 0.0f;
    color_blend_info.blendConstants[3] = 0.0f;
     
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1u;
    pipeline_layout_info.pSetLayouts = &desc_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0u;
    pipeline_layout_info.pPushConstantRanges = nullptr;
    
    VkResult result = vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &m_pipeline_layout);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterizer_info;
    pipeline_info.pMultisampleState = &multisample_info;
    pipeline_info.pDepthStencilState = &depth_stencil_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDynamicState = &dynamic_state_info;
    pipeline_info.layout = m_pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0u;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline);
    
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

std::vector<VkFramebuffer> VulkanRenderer::createFramebuffers() {
    size_t ct = m_out_color_images.size();
    std::vector<VkFramebuffer> result_framebuffers(ct);
    for(size_t i = 0u; i < ct; ++i) {
        std::array<VkImageView, 3> attachments = {m_out_color_images[i].image_view, m_out_depth_images[i].image_view, m_swapchain.getSwapchainImages()[i].view};
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_render_pass;
        framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = m_swapchain.getSwapchainParams().extent.width;
        framebuffer_info.height = m_swapchain.getSwapchainParams().extent.height;
        framebuffer_info.layers = 1u;
        
        VkResult result = vkCreateFramebuffer(m_device->getDevice(), &framebuffer_info, nullptr, &result_framebuffers[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
    
    return result_framebuffers;
}

VkImage VulkanRenderer::createImage(VkCommandPool cmd_pool, VkQueue queue, const std::string& path_to_file) {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    VkDeviceSize image_size = tex_width * tex_height * 4;
    
    m_mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1u;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    createBuffer(m_device, image_size, m_device->getQueueFamilyIndices(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_memory);
    
    void* data;
    vkMapMemory(m_device->getDevice(), staging_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(m_device->getDevice(), staging_memory);
    
    stbi_image_free(pixels);
    
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(tex_width);
    image_info.extent.height = static_cast<uint32_t>(tex_height);
    image_info.extent.depth = 1u;
    image_info.mipLevels = m_mip_levels;
    image_info.arrayLayers = 1u;
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0u;
    
    VkImage image;
    createImage(m_device, m_texture_memory, image, image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    transitionImageLayout(m_device->getDevice(), cmd_pool, queue, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mip_levels);
    copyBufferToImage(m_device->getDevice(), cmd_pool, queue, staging_buffer, image, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));
    //transitionImageLayout(device, cmd_pool, queue, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mip_levels);
    generateMipmaps(m_device, cmd_pool, queue, image, VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, m_mip_levels);
    
    vkDestroyBuffer(m_device->getDevice(), staging_buffer, nullptr);
    vkFreeMemory(m_device->getDevice(), staging_memory, nullptr);
    
    return image;
}

void VulkanRenderer::generateMipmaps(const std::shared_ptr<VulkanDevice>& device_ptr, VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels) {
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(device_ptr->getDeviceAbilities().physical_device, image_format, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer command_buffer = VulkanCommandManager::beginSingleTimeCommands(device_ptr->getDevice(), cmd_pool);
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0u;
    barrier.subresourceRange.layerCount = 1u;
    barrier.subresourceRange.levelCount = 1u;
    
    int32_t mip_width = tex_width;
    int32_t mip_height = tex_height;
    for (uint32_t i = 1u; i < mip_levels; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1u;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
        
        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mip_width, mip_height, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1u;
        blit.srcSubresource.baseArrayLayer = 0u;
        blit.srcSubresource.layerCount = 1u;
        
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0u;
        blit.dstSubresource.layerCount = 1u;
        
        vkCmdBlitImage(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
        
        if (mip_width > 1) {
            mip_width /= 2;
        }
        if (mip_height > 1) {
            mip_height /= 2;
        }
    }
    
    barrier.subresourceRange.baseMipLevel = mip_levels - 1u;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
    
    VulkanCommandManager::endSingleTimeCommands(device_ptr->getDevice(), command_buffer, queue, cmd_pool);
}

void VulkanRenderer::copyBuffer(VkDevice device, VkCommandPool cmd_pool, VkQueue queue, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
    VkCommandBuffer command_buffer = VulkanCommandManager::beginSingleTimeCommands(device, cmd_pool);
    
    VkBufferCopy copy_regions{};
    copy_regions.srcOffset = 0u;
    copy_regions.dstOffset = 0u;
    copy_regions.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_regions);
    
    VulkanCommandManager::endSingleTimeCommands(device, command_buffer, queue, cmd_pool);
}

VkSampler VulkanRenderer::createTextureSampler(uint32_t mip_levels) {
    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(m_device->getDeviceAbilities().physical_device, &supported_features);

    VkPhysicalDeviceProperties device_props{};
    vkGetPhysicalDeviceProperties(m_device->getDeviceAbilities().physical_device, &device_props);
    
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = supported_features.samplerAnisotropy;
    sampler_info.maxAnisotropy = device_props.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = static_cast<float>(mip_levels);;
    
    VkSampler texture_sampler;
    VkResult result = vkCreateSampler(m_device->getDevice(), &sampler_info, nullptr, &texture_sampler);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    return texture_sampler;
}

void VulkanRenderer::createAndTransferVertexBuffer(VkCommandPool cmd_pool, VkQueue queue, const std::vector<Vertex>& vertices) {
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    createBuffer(m_device, buffer_size, m_device->getQueueFamilyIndices(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_memory);
    
    void* data;
    vkMapMemory(m_device->getDevice(), staging_memory, 0, buffer_size, 0u, &data);
    memcpy(data, vertices.data(), buffer_size);
    vkUnmapMemory(m_device->getDevice(), staging_memory);
    
    createBuffer(m_device, buffer_size, m_device->getQueueFamilyIndices(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertex_buffer, m_vertex_memory);
    copyBuffer(m_device->getDevice(), cmd_pool, queue, staging_buffer, m_vertex_buffer, buffer_size);
    
    vkDestroyBuffer(m_device->getDevice(), staging_buffer, nullptr);
    vkFreeMemory(m_device->getDevice(), staging_memory, nullptr);
}

void VulkanRenderer::createAndTransferIndexBuffer(VkCommandPool cmd_pool, VkQueue queue, const std::vector<uint16_t>& indices) {
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    
    createBuffer(m_device, buffer_size, m_device->getQueueFamilyIndices(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, staging_buffer, staging_memory);
    void* data;
    vkMapMemory(m_device->getDevice(), staging_memory, 0u, buffer_size, 0u, &data);
    memcpy(data, indices.data(), buffer_size);
    vkUnmapMemory(m_device->getDevice(), staging_memory);
    
    createBuffer(m_device, buffer_size, m_device->getQueueFamilyIndices(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_index_buffer, m_index_memory);
    copyBuffer(m_device->getDevice(), cmd_pool, queue, staging_buffer, m_index_buffer, buffer_size);
    
    vkDestroyBuffer(m_device->getDevice(), staging_buffer, nullptr);
    vkFreeMemory(m_device->getDevice(), staging_memory, nullptr);
}

void VulkanRenderer::createUniformBuffers(VkDevice device) {
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    
    m_uniform_buffers.resize(m_swapchain.getMaxFrames());
    m_uniform_memory.resize(m_swapchain.getMaxFrames());
    m_uniform_mapped.resize(m_swapchain.getMaxFrames());
    
    for(size_t i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        createBuffer(m_device, buffer_size, m_device->getQueueFamilyIndices(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniform_buffers[i], m_uniform_memory[i]);
        vkMapMemory(m_device->getDevice(), m_uniform_memory[i], 0u, buffer_size, 0u, &m_uniform_mapped[i]);
    }
}

void VulkanRenderer::createDescSets(VkDevice device) {
    std::vector<VkDescriptorSetLayout> layouts(m_swapchain.getMaxFrames(), m_desc_set_layout);

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_desc_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(m_swapchain.getMaxFrames());
    alloc_info.pSetLayouts = layouts.data();
    
    m_desc_sets.resize(m_swapchain.getMaxFrames());
    VkResult result = vkAllocateDescriptorSets(device, &alloc_info, m_desc_sets.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
        
    for(size_t i = 0u; i < m_swapchain.getMaxFrames(); ++i) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = m_uniform_buffers[i];
        buffer_info.offset = 0u;
        buffer_info.range = sizeof(UniformBufferObject);
        
        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = m_texture_view;
        image_info.sampler = m_texture_sampler;
        
        std::array<VkWriteDescriptorSet, 2u> desc_writes{};
        desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[0].dstSet = m_desc_sets[i];
        desc_writes[0].dstBinding = 0u;
        desc_writes[0].dstArrayElement = 0u;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc_writes[0].descriptorCount = 1u;
        desc_writes[0].pBufferInfo = &buffer_info;
        desc_writes[0].pImageInfo = nullptr;
        desc_writes[0].pTexelBufferView = nullptr;
        
        desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[1].dstSet = m_desc_sets[i];
        desc_writes[1].dstBinding = 1u;
        desc_writes[1].dstArrayElement = 0u;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc_writes[1].descriptorCount = 1u;
        desc_writes[1].pImageInfo = &image_info;
        desc_writes[1].pBufferInfo = nullptr;
        desc_writes[1].pTexelBufferView = nullptr;
        
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0u, nullptr);
    }
}

VkDescriptorPool VulkanRenderer::createDescPool(VkDevice device) {
    std::array<VkDescriptorPoolSize, 2u> pool_sizes{};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>(m_swapchain.getMaxFrames());
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = static_cast<uint32_t>(m_swapchain.getMaxFrames());
    
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = static_cast<uint32_t>(m_swapchain.getMaxFrames());
    pool_info.flags = 0u;
 
    VkDescriptorPool desc_pool;
    VkResult result = vkCreateDescriptorPool(device, &pool_info, nullptr, &desc_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    return desc_pool;
}

void VulkanRenderer::createSyncObjects(){
    m_image_available.resize(m_swapchain.getMaxFrames());
    m_render_finished.resize(m_swapchain.getMaxFrames());
    m_in_flight_frame.resize(m_swapchain.getMaxFrames());

    VkSemaphoreCreateInfo image_available_sema_info{};
    image_available_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkSemaphoreCreateInfo render_finished_sem_info{};
    render_finished_sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo in_flight_fen_info{};
    in_flight_fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    in_flight_fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for(size_t i = 0u; i < m_swapchain.getMaxFrames(); ++i) {
        VkResult result = vkCreateSemaphore(m_device->getDevice(), &image_available_sema_info, nullptr, &m_image_available[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }
        
        result = vkCreateSemaphore(m_device->getDevice(), &render_finished_sem_info, nullptr, &m_render_finished[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }
        
        result = vkCreateFence(m_device->getDevice(), &in_flight_fen_info, nullptr, &m_in_flight_frame[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create fence!");
        }
    }
}