#include "vulkan_renderer.h"

#include "api/vulkan_device.h"
#include "api/vulkan_image_buffer.h"
#include "api/vulkan_command_buffer.h"
#include "api/vulkan_shader.h"
#include "api/vulkan_pipeline.h"
#include "api/vulkan_swapchain.h"
#include "api/vulkan_render_pass.h"
#include "api/vulkan_descriptors_manager.h"
#include "api/vulkan_shaders_manager.h"
#include "api/vulkan_pipelines_manager.h"
#include "api/vulkan_fence_manager.h"
#include "api/vulkan_semaphores_manager.h"
#include "api/vulkan_command_manager.h"
#include "api/vulkan_render_passes_manager.h"
#include "api/vulkan_format_manager.h"
#include "api/vulkan_resources_manager.h"
#include "pod/render_node.h"
#include "pod/render_graph.h"
#include "pod/graphics_render_node_config.h"
#include "pod/image_buffer_config.h"
#include "pod/render_pass_config.h"

#include <algorithm>

bool PerFrame::init(std::shared_ptr<VulkanDevice> device, unsigned index) {
    frame_index = index;
    return true;
}

void PerFrame::destroy(VulkanRenderer& renderer) {
    renderer.getSemaphoreManager()->returnSemaphore(swapchain_available_sem);
    for(VkSemaphore sem : cmd_submit_wait_sem) {
        renderer.getSemaphoreManager()->returnSemaphore(sem);
    }
    renderer.getResourcesManager()->delete_image(out_color_image);
    renderer.getResourcesManager()->delete_image(out_depth_image);
    command_buffer->destroy();
    render_graph->destroy();
}
	
void PerFrame::begin(VulkanRenderer& renderer, uint32_t image_index) {
    frame_index = image_index;
    for(VkSemaphore sem : cmd_submit_wait_sem) {
        renderer.getSemaphoreManager()->returnSemaphore(sem);
    }
    cmd_submit_wait_sem.clear();
    present_wait_sem.clear();
}

void PerFrame::end(VulkanRenderer& renderer) {
    renderer.getSemaphoreManager()->returnSemaphore(swapchain_available_sem);
    swapchain_available_sem = renderer.getSemaphoreManager()->getSemaphore("swapchain_available_sem");
}

bool VulkanRenderer::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<WindowSurface> window, std::shared_ptr<ThreadPool> thread_pool) {
    using namespace std::literals;

    m_device = std::move(device);
    m_thread_pool = std::move(thread_pool);

    m_format_manager = std::make_shared<VulkanFormatManager>();
    m_format_manager->init(m_device, window, "graphics_pipelines.xml"s);

    m_fence_manager = std::make_shared<VulkanFenceManager>();
    m_fence_manager->init(m_device);

    m_semaphore_manager = std::make_shared<VulkanSemaphoresManager>();
    m_semaphore_manager->init(m_device);

    m_resources_manager = std::make_shared<VulkanResourcesManager>(m_device, m_format_manager);
    m_resources_manager->init("graphics_pipelines.xml"s);

    m_swapchain = std::make_shared<VulkanSwapChain>();
    m_swapchain->init(m_device, std::move(window), "graphics_pipelines.xml"s);

    m_command_manager = m_device->getCommandManager();

    m_frame = 0u;
    m_prev_frame = 0u;

    int max_frames = m_swapchain->getMaxFrames();
    m_per_frame.reserve(max_frames);
    for(int i = 0; i < max_frames; ++i) {
        std::shared_ptr<PerFrame> per_frame = std::make_shared<PerFrame>();

        per_frame->out_color_image = m_resources_manager->create_image("render_target_color", "render_target_color_resource");
        per_frame->out_depth_image = m_resources_manager->create_image("render_target_depth", "render_target_depth_resource");

        per_frame->swapchain_available_sem = m_semaphore_manager->getSemaphore("swapchain_available_sem");
        //per_frame->swapchain_available_fen = m_fence_manager->getFence();

        per_frame->render_graph = std::make_shared<RenderGraph>();
        per_frame->render_graph->init(m_device);

        per_frame->init(m_device, i);
        per_frame->command_buffer = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::GRAPICS);
        per_frame->cmd_submit_finish_fence = per_frame->command_buffer->getRenderFence();

        m_per_frame.push_back(std::move(per_frame));
    }

    m_descriptors_manager = std::make_shared<VulkanDescriptorsManager>();
    m_descriptors_manager->init(m_device, "graphics_pipelines.xml"s);

    m_render_passes_manager = std::make_shared<VulkanRenderPassesManager>();
    m_render_passes_manager->init(m_device, "graphics_pipelines.xml"s, m_swapchain);

    m_shaders_manager = std::make_shared<VulkanShadersManager>();
    m_shaders_manager->init(m_device, "graphics_pipelines.xml"s);

    m_pipelines_manager = std::make_shared<VulkanPipelinesManager>();
    m_pipelines_manager->init(m_device, "graphics_pipelines.xml"s);

    return true;
}

void VulkanRenderer::destroy() {
    vkDeviceWaitIdle(m_device->getDevice());

    m_descriptors_manager->destroy();

    size_t sz = m_swapchain->getMaxFrames();
    m_swapchain->destroy();
    for(size_t i = 0u; i < sz; ++i) {
        m_per_frame[i]->destroy(*this);
    }
    m_command_manager->destroy();
    m_fence_manager->destroy();
    m_semaphore_manager->destroy();
    m_resources_manager->destroy();
    m_shaders_manager->destroy();
    m_pipelines_manager->destroy();
    m_render_passes_manager->destroy();
}

const std::shared_ptr<VulkanSwapChain>& VulkanRenderer::getSwapchain() const {
    return m_swapchain;
}

std::shared_ptr<VulkanDevice>& VulkanRenderer::GetDevice() {
    return m_device;
}

std::shared_ptr<VulkanDescriptorsManager>& VulkanRenderer::getDescriptorsManager() {
    return m_descriptors_manager;
}

std::shared_ptr<VulkanShadersManager>& VulkanRenderer::getShadersManager() {
    return m_shaders_manager;
}

std::shared_ptr<VulkanPipelinesManager>& VulkanRenderer::getPipelinesManager() {
    return m_pipelines_manager;
}
	
std::shared_ptr<VulkanFenceManager>& VulkanRenderer::getFenceManager() {
    return m_fence_manager;
}

std::shared_ptr<VulkanSemaphoresManager>& VulkanRenderer::getSemaphoreManager() {
    return m_semaphore_manager;
}

std::shared_ptr<VulkanCommandManager>& VulkanRenderer::getCommandManager() {
    return m_command_manager;
}

std::shared_ptr<VulkanRenderPassesManager>& VulkanRenderer::getRenderPassesManager() {
    return m_render_passes_manager;
}

std::shared_ptr<VulkanFormatManager>& VulkanRenderer::getFormatManager() {
    return m_format_manager;
}

std::shared_ptr<VulkanResourcesManager>& VulkanRenderer::getResourcesManager() {
    return m_resources_manager;
}

std::shared_ptr<VulkanImageBuffer>& VulkanRenderer::getOutColorImage(uint32_t image_index) {
    return m_per_frame[image_index]->out_color_image;
}

std::shared_ptr<VulkanImageBuffer>& VulkanRenderer::getOutDepthImage(uint32_t image_index) {
    return m_per_frame[image_index]->out_depth_image;
}

void VulkanRenderer::beginFrame(unsigned image_index) {
    m_per_frame[image_index]->begin(*this, image_index);

    vkWaitForFences(m_device->getDevice(), 1u, &(m_per_frame[image_index]->cmd_submit_finish_fence), VK_TRUE, UINT64_MAX);
    vkResetFences(m_device->getDevice(), 1u, &(m_per_frame[image_index]->cmd_submit_finish_fence));

    m_per_frame[image_index]->command_buffer->reset();
    m_per_frame[image_index]->cmd_submit_finish_signal_sem = m_per_frame[image_index]->command_buffer->getInProgressSemaphore();
    m_per_frame[image_index]->cmd_submit_finish_fence = m_per_frame[image_index]->command_buffer->getRenderFence();
}

void VulkanRenderer::recordCommandBuffer(CommandBatch& command_buffer, unsigned image_index) {
    VulkanCommandManager::beginCommandBuffer(command_buffer);

    const RenderGraph::RenderNodeList& topologically_sorted_nodes = m_per_frame[image_index]->render_graph->getTopologicallySortedNodes();
    for(const std::shared_ptr<RenderNode>& render_node_ptr : topologically_sorted_nodes) {
        render_node_ptr->render(command_buffer);
    }
    
    VulkanCommandManager::endCommandBuffer(command_buffer);
}

void VulkanRenderer::drawFrame(unsigned image_index) {
    recordCommandBuffer(*m_per_frame[image_index]->command_buffer, image_index);
    
    CommandBatch::BatchWaitInfo wait_info;
    wait_info.wait_for_semaphores.push_back(m_per_frame[m_prev_frame]->swapchain_available_sem);
    if(m_per_frame[image_index]->cmd_submit_wait_sem.size()) {
        wait_info.wait_for_semaphores.insert(wait_info.wait_for_semaphores.end(), m_per_frame[image_index]->cmd_submit_wait_sem.begin(), m_per_frame[image_index]->cmd_submit_wait_sem.end());
    }
    wait_info.wait_for_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    VkSubmitInfo submit_info = m_per_frame[image_index]->command_buffer->getSubmitInfo(&wait_info);

    m_command_manager->submitCommandBuffer(m_per_frame[image_index]->command_buffer, VulkanCommandManager::SELECT_ALL_BUFFERS, &submit_info);

    m_per_frame[image_index]->present_wait_sem.push_back(m_per_frame[image_index]->cmd_submit_finish_signal_sem);
    
    VkSwapchainKHR swapchains[] = {m_swapchain->getSwapchain()};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = static_cast<uint32_t>(m_per_frame[image_index]->present_wait_sem.size());
    present_info.pWaitSemaphores = m_per_frame[image_index]->present_wait_sem.data();
    present_info.swapchainCount = 1u;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    VkResult result = vkQueuePresentKHR(m_command_manager->getQueue(), &present_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_per_frame[image_index]->end(*this);
}

std::shared_ptr<PerFrame>& VulkanRenderer::getFrameData(uint32_t image_index) {
    return m_per_frame[image_index];
}

void VulkanRenderer::update_frame(const GameTimerDelta& delta, uint32_t image_index) {
    
}

void VulkanRenderer::addRenderNode(std::shared_ptr<RenderNode> render_node, unsigned image_index) {
    m_per_frame[image_index]->render_graph->add_pass(std::move(render_node));
}

std::pair<bool, uint32_t> VulkanRenderer::acquire_next_image() {
    m_prev_frame = m_frame;
    //vkWaitForFences(m_device->getDevice(), 1u, &(m_per_frame[m_frame]->swapchain_available_fen), VK_TRUE, UINT64_MAX);
    //vkResetFences(m_device->getDevice(), 1u, &(m_per_frame[m_frame]->swapchain_available_fen));
    VkResult result = vkAcquireNextImageKHR(
        m_device->getDevice(),
        m_swapchain->getSwapchain(),
        UINT64_MAX,
        m_per_frame[m_prev_frame]->swapchain_available_sem,
        //m_per_frame[m_prev_frame]->swapchain_available_fen,
        VK_NULL_HANDLE,
        &m_frame
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return {false, m_frame};
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    return {true, m_frame};
}