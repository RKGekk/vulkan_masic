#include "vulkan_renderer.h"

bool VulkanRenderer::init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, const std::string& texture_path) {
    m_device = std::move(device);

    m_swapchain.init(m_device, surface, window);

    createColorResources();
    createDepthResources();

    loadShaders();

    createRenderPass();
    m_desc_set_layout = createDescSetLayout();
    createPipeline(m_device->getDevice(), m_vert_shader_modeule, m_frag_shader_modeule, m_render_pass, m_swapchain.getSwapchainParams(), m_desc_set_layout, m_device->getMsaaSamples());
    m_out_framebuffers = createFramebuffers();

    m_texture_image = m_device->createImageAndView(texture_path);
    m_texture_sampler = createTextureSampler(m_texture_image.image_info.mipLevels);
    
    createUniformBuffers(m_device->getDevice());
    
    m_desc_pool = createDescPool(m_device->getDevice());
    createDescSets(m_device->getDevice());

    m_command_buffers = m_device->getCommandManager().allocCommandBuffer(m_device->getCommandManager().getGrapicsCommandPool(), m_swapchain.getMaxFrames());
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

        vkDestroyBuffer(m_device->getDevice(), m_uniform_buffers[i].buf, nullptr);
        vkFreeMemory(m_device->getDevice(), m_uniform_buffers[i].mem, nullptr);

        vkDestroySemaphore(m_device->getDevice(), m_image_available[i], nullptr);
        vkDestroySemaphore(m_device->getDevice(), m_render_finished[i], nullptr);
        vkDestroyFence(m_device->getDevice(), m_in_flight_frame[i], nullptr);
    }

    vkDestroySampler(m_device->getDevice(), m_texture_sampler, nullptr);

    vkDestroyImageView(m_device->getDevice(), m_texture_image.image_view, nullptr);
    vkDestroyImage(m_device->getDevice(), m_texture_image.image, nullptr);
    vkFreeMemory(m_device->getDevice(), m_texture_image.memory, nullptr);
    
    vkDestroyRenderPass(m_device->getDevice(), m_render_pass, nullptr);
    vkDestroyPipeline(m_device->getDevice(), m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device->getDevice(), m_pipeline_layout, nullptr);

    vkDestroyDescriptorPool(m_device->getDevice(), m_desc_pool, nullptr);
    vkDestroyDescriptorSetLayout(m_device->getDevice(), m_desc_set_layout, nullptr);

    for (const std::shared_ptr<VulkanDrawable>& drawable : m_drawable_list) {
        drawable->destroy();
    }

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
    
    for(const std::shared_ptr<VulkanDrawable> renderable : m_drawable_list) {
        vkCmdBeginRenderPass(command_buffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
        
        VkBuffer vertex_buffers[] = {renderable->getVertexBuffer().buf};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(command_buffer, renderable->getIndexBuffer().buf, 0u, VK_INDEX_TYPE_UINT16);
        
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
        
        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(renderable->getIndicesCount()), 1u, 0u, 0u, 0u);
        vkCmdEndRenderPass(command_buffer);
    }
    
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
    
    m_device->getCommandManager().submitCommandBuffer(m_device->getCommandManager().getGraphicsQueue(), submit_info, m_in_flight_frame[m_swapchain.getCurrentFrame()]);
    
    VkSwapchainKHR swapchains[] = {m_swapchain.getSwapchain()};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1u;
    present_info.pWaitSemaphores = render_end_semaphores;
    present_info.swapchainCount = 1u;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    result = vkQueuePresentKHR(m_device->getCommandManager().getPresentQueue(), &present_info);
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

void VulkanRenderer::addDrawable(std::shared_ptr<VulkanDrawable> drawable) {
    m_drawable_list.push_back(std::move(drawable));
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
    depth_attachment.format = device_ptr->findDepthFormat();
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
        m_out_color_images[i] = m_device->createImage(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1u);
    }
}

void VulkanRenderer::createDepthResources() {
    VkFormat depth_format = m_device->findDepthFormat();
    
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
        m_out_depth_images[i] = m_device->createImage(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, 1u);
        m_device->getCommandManager().transitionImageLayout(m_out_depth_images[i].image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1u);
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
    std::vector<VkVertexInputAttributeDescription> attribute_desc = Vertex::getAttributeDescritpions();
    
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

void VulkanRenderer::createUniformBuffers(VkDevice device) {
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    
    m_uniform_buffers.resize(m_swapchain.getMaxFrames());
    m_uniform_mapped.resize(m_swapchain.getMaxFrames());
    
    for(size_t i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        m_uniform_buffers[i] = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(m_device->getDevice(), m_uniform_buffers[i].mem, 0u, buffer_size, 0u, &m_uniform_mapped[i]);
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
        buffer_info.buffer = m_uniform_buffers[i].buf;
        buffer_info.offset = 0u;
        buffer_info.range = sizeof(UniformBufferObject);
        
        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = m_texture_image.image_view;
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