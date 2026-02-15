#include "vulkan_renderer.h"

#include <algorithm>

bool VulkanRenderer::init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, std::shared_ptr<ThreadPool> thread_pool) {
    using namespace std::literals;

    m_device = std::move(device);
    m_thread_pool = std::move(thread_pool);
    m_swapchain = std::make_shared<VulkanSwapChain>();
    m_swapchain->init(m_device, surface, window);

    int max_frames = m_swapchain->getMaxFrames();

    m_managers = std::make_shared<Managers>();

    m_managers->command_manager = m_device->getCommandManager();

    m_per_frame.reserve(max_frames);
    for(int i = 0; i < max_frames; ++i) {
        std::shared_ptr<PerFrame> per_frame = std::make_shared<PerFrame>();
        per_frame->init(m_device, i);

        std::shared_ptr<RenderTarget> rt = std::make_shared<RenderTarget>();
        rt->init(m_device, m_swapchain, i);

        per_frame->command_buffer = m_managers->command_manager->allocCommandBufferPtr(PoolTypeEnum::GRAPICS);

        m_per_frame.push_back(std::move(per_frame));
    }

    m_managers->descriptors_manager = std::make_shared<VulkanDescriptorsManager>();
    m_managers->descriptors_manager->init(m_device, "graphics_pipelines.xml"s);

    m_managers->render_passes_manager = std::make_shared<VulkanRenderPassesManager>();
    m_managers->render_passes_manager->init(m_device, "graphics_pipelines.xml"s, m_swapchain);

    m_managers->shaders_manager = std::make_shared<VulkanShadersManager>();
    m_managers->shaders_manager->init(m_device, "graphics_pipelines.xml"s);

    m_managers->pipelines_manager = std::make_shared<VulkanPipelinesManager>();
    m_managers->pipelines_manager->init(m_device, "graphics_pipelines.xml"s);

    m_managers->fence_manager = std::make_shared<VulkanFenceManager>();
    m_managers->fence_manager->init(m_device);

    m_managers->semaphore_manager = std::make_shared<VulkanSemaphoresManager>();
    m_managers->semaphore_manager->init(m_device);

    m_render_graph = std::make_shared<RenderGraph>();
    m_render_graph->init(m_device);

    return true;
}

void VulkanRenderer::destroy() {
    vkDeviceWaitIdle(m_device->getDevice());
    size_t sz = m_swapchain->getMaxFrames();
    m_swapchain->destroy();
    for(size_t i = 0u; i < sz; ++i) {
        m_per_frame[i]->command_buffer->destroy();
    }
}

const std::shared_ptr<VulkanSwapChain>& VulkanRenderer::getSwapchain() const {
    return m_swapchain;
}

std::shared_ptr<VulkanDevice>& VulkanRenderer::GetDevice() {
    return m_device;
}

std::shared_ptr<Managers>& VulkanRenderer::getManagers() {
    return m_managers;
}

void VulkanRenderer::TransitionResourcesToProperState(const std::shared_ptr<RenderNode>& render_node_ptr, CommandBatch& command_buffer) {
    const std::shared_ptr<VulkanPipeline>& pipeline_ptr = render_node_ptr->getPipeline();
    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = pipeline_ptr->getRenderPass();

    for(const auto&[gloabal_name, att_slot] : render_node_ptr->getReadResourcesMap()){
        if(std::shared_ptr<RenderNode> last_written_by_node = m_render_graph->getLastWritten(render_node_ptr, gloabal_name)) {

        }
        else {
            
        }
    }

    for(const auto&[gloabal_name, att_slot] : render_node_ptr->getWrittenResourcesMap()){
        
    }
}

void VulkanRenderer::recordCommandBuffer(CommandBatch& command_buffer) {
    VulkanCommandManager::beginCommandBuffer(command_buffer);

    uint32_t current_frame = m_swapchain->getCurrentFrame();
    for(const std::shared_ptr<RenderNode> render_node_ptr : m_render_graph->getTopologicallySortedNodes()) {
        const std::shared_ptr<VulkanPipeline>& pipeline_ptr = render_node_ptr->getPipeline();
        const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = pipeline_ptr->getRenderPass();

        TransitionResourcesToProperState(render_node_ptr, command_buffer);

        VkRenderPassBeginInfo renderpass_info{};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_info.renderPass = render_pass_ptr->getRenderPass();
        renderpass_info.framebuffer = render_node_ptr->getFramebuffer(m_swapchain->getCurrentSync());
        renderpass_info.renderArea.offset = {0, 0};
        renderpass_info.renderArea.extent = render_node_ptr->getViewportExtent();
    }
    
    VulkanCommandManager::endCommandBuffer(command_buffer);
}

void VulkanRenderer::drawFrame() {
    bool next_frame_available = m_swapchain->setNextFrame(m_per_frame[m_swapchain->fetchNextSync()]->command_buffer->getRenderFence());
    if (!next_frame_available) {
        return;
    }

    m_per_frame[m_swapchain->getCurrentSync()]->command_buffer->reset();
    recordCommandBuffer(*m_per_frame[m_swapchain->getCurrentSync()]->command_buffer);
    
    CommandBatch::BatchWaitInfo wait_info;
    wait_info.wait_for_semaphores.push_back(m_swapchain->getImageAvailableSemaphore());
    wait_info.wait_for_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    VkSubmitInfo submit_info = m_per_frame[m_swapchain->getCurrentSync()]->command_buffer->getSubmitInfo(&wait_info);

    m_managers->command_manager->submitCommandBuffer(*m_per_frame[m_swapchain->getCurrentSync()]->command_buffer, VulkanCommandManager::SELECT_ALL_BUFFERS, &submit_info);
    
    VkSwapchainKHR swapchains[] = {m_swapchain->getSwapchain()};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1u;
    present_info.pWaitSemaphores = m_per_frame[m_swapchain->getCurrentSync()]->command_buffer->getInProgressSemaphorePtr();
    present_info.swapchainCount = 1u;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = m_swapchain->getCurrentFramePtr();
    present_info.pResults = nullptr;
    VkResult result = vkQueuePresentKHR(m_managers->command_manager->getQueue(), &present_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void VulkanRenderer::update_frame(const GameTimerDelta& delta, uint32_t image_index) {
    
}

void VulkanRenderer::addRenderNode(std::shared_ptr<RenderNode> render_node) {
    m_render_graph->add_pass(std::move(render_node));
}