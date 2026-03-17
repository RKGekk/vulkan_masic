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

#include <algorithm>

bool PerFrame::init(std::shared_ptr<VulkanDevice> device, unsigned index) {
    return true;
}

void PerFrame::destroy() {

}
	
bool PerFrame::wait(uint64_t timeout) {
    return true;
}

void PerFrame::begin() {

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

    int max_frames = m_swapchain->getMaxFrames();
    m_per_frame.reserve(max_frames);
    for(int i = 0; i < max_frames; ++i) {
        std::shared_ptr<PerFrame> per_frame = std::make_shared<PerFrame>();

        per_frame->out_color_image = m_resources_manager->create_image("render_target_color", "render_target_color_resource");
        per_frame->out_depth_image = m_resources_manager->create_image("render_target_depth", "render_target_depth_resource");

        per_frame->init(m_device, i);
        per_frame->command_buffer = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::GRAPICS);

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

void VulkanRenderer::TransitionResourcesToProperState(const std::shared_ptr<RenderNode>& render_node_ptr, CommandBatch& command_buffer) {
    const std::shared_ptr<VulkanPipeline>& pipeline_ptr = render_node_ptr->getPipeline();
    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = pipeline_ptr->getRenderPass();

    for(const auto&[gloabal_name, att_slot] : render_node_ptr->getReadResourcesMap()){
        RenderResource::Type att_slot_res_type = att_slot.resource->getType();
        if(att_slot_res_type == RenderResource::Type::BUFFER) continue;

        size_t last_written_by_node_id = m_render_graph->getLastWrittenIdentity(render_node_ptr, gloabal_name);
        size_t last_read_by_node_id = m_render_graph->getLastReadIdentity(render_node_ptr, gloabal_name);

        if(last_read_by_node_id != RenderGraph::NO_ID && last_read_by_node_id > last_written_by_node_id) {
            continue;
        }

        if(last_written_by_node_id != RenderGraph::NO_ID && last_written_by_node_id > last_read_by_node_id) {
            const RenderGraph::RenderNodePtr& last_written_by_node = m_render_graph->getRenderNodeByID(last_read_by_node_id);
            
        }
    }

    for(const auto&[gloabal_name, att_slot] : render_node_ptr->getWrittenResourcesMap()){
        
    }
}

std::vector<std::shared_ptr<VulkanImageBuffer>>& VulkanRenderer::getOutColorImages() {
    return m_out_color_images;
}

std::vector<std::shared_ptr<VulkanImageBuffer>>& VulkanRenderer::getOutDepthImages() {
    return m_out_depth_images;
}

void VulkanRenderer::recordCommandBuffer(CommandBatch& command_buffer) {
    VulkanCommandManager::beginCommandBuffer(command_buffer);

    uint32_t current_frame = m_swapchain->getCurrentFrame();

    // return all RenderResources to initial state

    for(const std::shared_ptr<RenderNode> render_node_ptr : m_render_graph->getTopologicallySortedNodes()) {
        const std::shared_ptr<VulkanPipeline>& pipeline_ptr = render_node_ptr->getPipeline();
        const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = pipeline_ptr->getRenderPass();

        TransitionResourcesToProperState(render_node_ptr, command_buffer);

        VkRenderPassBeginInfo renderpass_info{};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_info.renderPass = render_pass_ptr->getRenderPass();
        //renderpass_info.framebuffer = render_node_ptr->getFramebuffer(m_swapchain->getCurrentSync());
        renderpass_info.framebuffer = render_node_ptr->getFramebuffer();
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

    m_command_manager->submitCommandBuffer(*m_per_frame[m_swapchain->getCurrentSync()]->command_buffer, VulkanCommandManager::SELECT_ALL_BUFFERS, &submit_info);
    
    VkSwapchainKHR swapchains[] = {m_swapchain->getSwapchain()};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1u;
    present_info.pWaitSemaphores = m_per_frame[m_swapchain->getCurrentSync()]->command_buffer->getInProgressSemaphorePtr();
    present_info.swapchainCount = 1u;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = m_swapchain->getCurrentFramePtr();
    present_info.pResults = nullptr;
    VkResult result = vkQueuePresentKHR(m_command_manager->getQueue(), &present_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void VulkanRenderer::update_frame(const GameTimerDelta& delta, uint32_t image_index) {
    
}

void VulkanRenderer::addRenderNode(std::shared_ptr<RenderNode> render_node) {
    m_render_graph->add_pass(std::move(render_node));
}