#include "vulkan_renderer.h"

bool VulkanRenderer::init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, std::shared_ptr<ThreadPool> thread_pool) {
    m_device = std::move(device);
    m_thread_pool = std::move(thread_pool);

    m_swapchain.init(m_device, surface, window);

    createColorResources();
    createDepthResources();

    m_command_buffers.reserve(m_swapchain.getMaxFrames());
    for(int i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        m_command_buffers.push_back(m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::GRAPICS));
    }

    m_image_available.resize(m_swapchain.getMaxFrames());
    VkSemaphoreCreateInfo image_available_sema_info{};
    image_available_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for(size_t i = 0u; i < m_swapchain.getMaxFrames(); ++i) {
        VkResult result = vkCreateSemaphore(m_device->getDevice(), &image_available_sema_info, nullptr, &m_image_available[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }
    }

    return true;
}

void VulkanRenderer::destroy() {
    vkDeviceWaitIdle(m_device->getDevice());
    size_t sz = m_swapchain.getMaxFrames();
    m_swapchain.destroy();
    for(size_t i = 0u; i < sz; ++i) {
        m_command_buffers[i].destroy();

        vkDestroyImageView(m_device->getDevice(), m_out_color_images[i].image_view, nullptr);
        vkDestroyImage(m_device->getDevice(), m_out_color_images[i].image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_out_color_images[i].memory, nullptr);
        
        vkDestroyImageView(m_device->getDevice(), m_out_depth_images[i].image_view, nullptr);
        vkDestroyImage(m_device->getDevice(), m_out_depth_images[i].image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_out_depth_images[i].memory, nullptr);
    }

    for (const std::shared_ptr<IVulkanDrawable>& drawable : m_drawable_list) {
        drawable->destroy();
    }
}

void VulkanRenderer::recreate() {
    vkDeviceWaitIdle(m_device->getDevice());
    m_swapchain.recreate();
    
    size_t sz = m_swapchain.getMaxFrames();
    for(size_t i = 0u; i < sz; ++i) {
        m_command_buffers[i].destroy();

        vkDestroyImageView(m_device->getDevice(), m_out_color_images[i].image_view, nullptr);
        vkDestroyImage(m_device->getDevice(), m_out_color_images[i].image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_out_color_images[i].memory, nullptr);
        
        vkDestroyImageView(m_device->getDevice(), m_out_depth_images[i].image_view, nullptr);
        vkDestroyImage(m_device->getDevice(), m_out_depth_images[i].image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_out_depth_images[i].memory, nullptr);
    }
    
    createColorResources();
    createDepthResources();

    for(int i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        m_command_buffers[i] = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::GRAPICS);
    }

    for (const std::shared_ptr<IVulkanDrawable>& drawable : m_drawable_list) {
        drawable->reset(getRenderTarget());
    }
}

const VulkanSwapChain& VulkanRenderer::getSwapchain() const {
    return m_swapchain;
}

const std::vector<ImageBufferAndView>& VulkanRenderer::getColorImages() const {
    return m_out_color_images;
}

const std::vector<ImageBufferAndView>& VulkanRenderer::getDepthImages() const {
    return m_out_depth_images;
}

RenderTarget VulkanRenderer::getRenderTarget() const {
    std::vector<RenderTarget::Attachment> frames;
    frames.reserve(m_swapchain.getMaxFrames());
    VkSampleCountFlagBits msaa_samples = m_device->getMsaaSamples();
    for (int i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        RenderTarget::Attachment attachment;
        if(msaa_samples == VK_SAMPLE_COUNT_1_BIT) {
            attachment.resize(2u);
            attachment[0u] = m_swapchain.getSwapchainImages()[i].view;
            attachment[1u] = m_out_depth_images[i].image_view;
        }
        else {
            attachment.resize(3u);
            attachment[0u] = m_out_color_images[i].image_view;
            attachment[1u] = m_out_depth_images[i].image_view;
            attachment[2u] = m_swapchain.getSwapchainImages()[i].view;
        }
        frames.push_back(std::move(attachment));
    }
    
    RenderTarget rt;
    rt.frame_count = m_swapchain.getMaxFrames();
    rt.render_target_fmt.colorAttachmentFormat = m_swapchain.getSwapchainParams().surface_format;
    rt.render_target_fmt.depthAttachmentFormat = m_out_depth_images[0u].image_info.format;
    rt.render_target_fmt.viewportExtent = m_swapchain.getSwapchainParams().extent;
    rt.frames = std::move(frames);

    return rt;
}

void VulkanRenderer::recordCommandBuffer(CommandBatch& command_buffer) {
    VulkanCommandManager::beginCommandBuffer(command_buffer);

    for(const std::shared_ptr<IVulkanDrawable> renderable : m_drawable_list) {
        renderable->recordCommandBuffer(command_buffer, m_swapchain.getCurrentFrame());
    }
    
    VulkanCommandManager::endCommandBuffer(command_buffer);
}

void VulkanRenderer::drawFrame() {
    bool next_frame_available = m_swapchain.setNextFrame(m_command_buffers[m_swapchain.fetchNextSync()].getRenderFence());
    if (!next_frame_available || m_framebuffer_resized) {
        recreate();
        return;
    }

    m_command_buffers[m_swapchain.getCurrentSync()].reset();
    recordCommandBuffer(m_command_buffers[m_swapchain.getCurrentSync()]);
    
    CommandBatch::BatchWaitInfo wait_info;
    wait_info.wait_for_semaphores.push_back(m_swapchain.getImageAvailableSemaphore());
    wait_info.wait_for_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    VkSubmitInfo submit_info = m_command_buffers[m_swapchain.getCurrentSync()].getSubmitInfo(&wait_info);

    m_device->getCommandManager().submitCommandBuffer(m_command_buffers[m_swapchain.getCurrentSync()], VulkanCommandManager::SELECT_ALL_BUFFERS, &submit_info);
    
    VkSwapchainKHR swapchains[] = {m_swapchain.getSwapchain()};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1u;
    present_info.pWaitSemaphores = m_command_buffers[m_swapchain.getCurrentSync()].getInProgressSemaphorePtr();
    present_info.swapchainCount = 1u;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = m_swapchain.getCurrentFramePtr();
    present_info.pResults = nullptr;
    VkResult result = vkQueuePresentKHR(m_device->getCommandManager().getQueue(), &present_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void VulkanRenderer::setFramebufferResized() {
    m_framebuffer_resized = true;
}

void VulkanRenderer::update_frame(const GameTimerDelta& delta, uint32_t image_index) {
    for(std::shared_ptr<IVulkanDrawable>& drawable : m_drawable_list) {
        drawable->update(delta, image_index);
    }
}

void VulkanRenderer::addDrawable(std::shared_ptr<IVulkanDrawable> drawable) {
    m_drawable_list.push_back(std::move(drawable));
}

void VulkanRenderer::createColorResources() {
    VkFormat color_format = m_swapchain.getSwapchainParams().surface_format.format;
    std::vector<uint32_t> families = m_device->getCommandManager().getQueueFamilyIndices().getIndices();
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
    image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    image_info.pQueueFamilyIndices = families.data();
    image_info.samples = m_device->getMsaaSamples();
    image_info.flags = 0u;

    m_out_color_images.resize(m_swapchain.getMaxFrames());
    for(uint32_t i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        m_out_color_images[i] = m_device->createImage(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1u);
    }
}

void VulkanRenderer::createDepthResources() {
    VkFormat depth_format = m_device->findDepthFormat();
    VkImageAspectFlags image_aspect = m_device->hasStencilComponent(depth_format) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
    std::vector<uint32_t> families = m_device->getCommandManager().getQueueFamilyIndices().getIndices();
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
    image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    image_info.pQueueFamilyIndices = families.data();
    image_info.samples = m_device->getMsaaSamples();
    image_info.flags = 0u;

    m_out_depth_images.resize(m_swapchain.getMaxFrames());
    for(uint32_t i = 0; i < m_swapchain.getMaxFrames(); ++i) {
        m_out_depth_images[i] = m_device->createImage(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_aspect, 1u);
        CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
        VulkanCommandManager::beginCommandBuffer(command_buffer);
        m_device->getCommandManager().transitionImageLayout(command_buffer.getCommandBufer(), m_out_depth_images[i].image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1u);
        VulkanCommandManager::endCommandBuffer(command_buffer);
        m_device->getCommandManager().submitCommandBuffer(command_buffer);
        m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);
    }
}