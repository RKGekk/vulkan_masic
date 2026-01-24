#include "vulkan_renderer.h"

#include <algorithm>

bool VulkanRenderer::init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, std::shared_ptr<ThreadPool> thread_pool) {
    using namespace std::literals;

    m_device = std::move(device);
    m_thread_pool = std::move(thread_pool);
    m_swapchain = std::make_shared<VulkanSwapChain>();
    m_swapchain->init(m_device, surface, window);

    int max_frames = m_swapchain->getMaxFrames();
    m_command_buffers.reserve(max_frames);
    m_render_targets.reserve(max_frames);
    for(int i = 0; i < max_frames; ++i) {
        RenderTarget rt;
        rt.init(m_device, m_swapchain, i);
        m_render_targets.push_back(rt);
        m_command_buffers.push_back(m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::GRAPICS));
    }

    m_descriptors_manager = std::make_shared<VulkanDescriptorsManager>();
    m_descriptors_manager->init(m_device, "graphics_pipelines.xml"s);

    m_shaders_manager = std::make_shared<VulkanShadersManager>();
    m_shaders_manager->init(m_device, "graphics_pipelines.xml"s);

    m_pipelines_manager = std::make_shared<VulkanPipelinesManager>();
    m_pipelines_manager->init(m_device, "graphics_pipelines.xml"s);

    return true;
}

void VulkanRenderer::destroy() {
    vkDeviceWaitIdle(m_device->getDevice());
    size_t sz = m_swapchain->getMaxFrames();
    m_swapchain->destroy();
    for(size_t i = 0u; i < sz; ++i) {
        m_command_buffers[i].destroy();
        m_render_targets[i].destroy();
    }

    for (const std::shared_ptr<IVulkanDrawable>& drawable : m_drawable_list) {
        drawable->destroy();
    }
}

void VulkanRenderer::recreate() {
    vkDeviceWaitIdle(m_device->getDevice());
    m_swapchain->recreate();
    
    size_t sz = m_swapchain->getMaxFrames();
    for(size_t i = 0u; i < sz; ++i) {
        m_command_buffers[i].destroy();
        m_render_targets[i].destroy();
    }
    
    for(int i = 0; i < m_swapchain->getMaxFrames(); ++i) {
        m_render_targets[i].init(m_device, m_swapchain, i);
        m_command_buffers[i] = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::GRAPICS);
    }

    for (const std::shared_ptr<IVulkanDrawable>& drawable : m_drawable_list) {
        drawable->reset(m_render_targets[0]);
    }
}

const std::shared_ptr<VulkanSwapChain>& VulkanRenderer::getSwapchain() const {
    return m_swapchain;
}

const RenderTarget& VulkanRenderer::getRenderTarget(uint32_t image_index) const {
    return m_render_targets.at(image_index);
}

const std::shared_ptr<VulkanDevice>& VulkanRenderer::GetDevice() {
    return m_device;
}

void VulkanRenderer::recordCommandBuffer(CommandBatch& command_buffer) {
    VulkanCommandManager::beginCommandBuffer(command_buffer);

    uint32_t current_frame = m_swapchain->getCurrentFrame();
    for(const std::shared_ptr<IVulkanDrawable> renderable : m_drawable_list) {
        renderable->recordCommandBuffer(command_buffer, m_render_targets[current_frame], current_frame);
        m_render_targets[current_frame].incExecCounter();
    }
    
    VulkanCommandManager::endCommandBuffer(command_buffer);
}

void VulkanRenderer::drawFrame() {
    bool next_frame_available = m_swapchain->setNextFrame(m_command_buffers[m_swapchain->fetchNextSync()].getRenderFence());
    if (!next_frame_available || m_framebuffer_resized) {
        recreate();
        return;
    }

    m_command_buffers[m_swapchain->getCurrentSync()].reset();
    m_render_targets[m_swapchain->getCurrentFrame()].resetExecCounter();
    recordCommandBuffer(m_command_buffers[m_swapchain->getCurrentSync()]);
    
    CommandBatch::BatchWaitInfo wait_info;
    wait_info.wait_for_semaphores.push_back(m_swapchain->getImageAvailableSemaphore());
    wait_info.wait_for_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    VkSubmitInfo submit_info = m_command_buffers[m_swapchain->getCurrentSync()].getSubmitInfo(&wait_info);

    m_device->getCommandManager().submitCommandBuffer(m_command_buffers[m_swapchain->getCurrentSync()], VulkanCommandManager::SELECT_ALL_BUFFERS, &submit_info);
    
    VkSwapchainKHR swapchains[] = {m_swapchain->getSwapchain()};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1u;
    present_info.pWaitSemaphores = m_command_buffers[m_swapchain->getCurrentSync()].getInProgressSemaphorePtr();
    present_info.swapchainCount = 1u;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = m_swapchain->getCurrentFramePtr();
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
    auto pos = std::lower_bound(m_drawable_list.begin(), m_drawable_list.end(), drawable->order(), [](const std::shared_ptr<IVulkanDrawable>& dr, int order) { return dr->order() < order; });
    if(pos != m_drawable_list.end()) {
        m_drawable_list.insert(pos, std::move(drawable));
    }
    else {
        m_drawable_list.push_back(std::move(drawable));
    }
}