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
#include "pod/present_render_node.h"
#include "pod/graphics_render_node.h"
#include "pod/render_graph.h"
#include "pod/framebuffer_config.h"
#include "pod/graphics_render_node_config.h"
#include "pod/image_buffer_config.h"
#include "pod/render_pass_config.h"
#include "pod/pipeline_config.h"

#include <algorithm>

bool PerFrame::init(std::shared_ptr<VulkanDevice> device, unsigned index) {
    frame_index = index;
    return true;
}

void PerFrame::destroy(VulkanRenderer& renderer) {
    renderer.getSemaphoreManager()->returnSemaphore(swapchain_available_sem);
    renderer.getFenceManager()->returnFence(swapchain_available_fen);
    for(VkSemaphore sem : cmd_submit_wait_sem) {
        renderer.getSemaphoreManager()->returnSemaphore(sem);
    }
    renderer.getResourcesManager()->delete_image(out_color_image);
    renderer.getResourcesManager()->delete_image(out_depth_image);
    command_buffer->destroy();
    present_render_node->destroy();
    render_graph->destroy();
}
	
void PerFrame::begin(VulkanRenderer& renderer, uint32_t image_index) {
    frame_index = image_index;
    for(VkSemaphore sem : cmd_submit_wait_sem) {
        renderer.getSemaphoreManager()->returnSemaphore(sem);
    }
    cmd_submit_wait_sem.clear();
    present_render_node->clearWaitSemaphores();
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
    m_resources_manager->init(window, "graphics_pipelines.xml"s);

    m_swapchain = std::make_shared<VulkanSwapChain>();
    m_swapchain->init(m_device, std::move(window), "graphics_pipelines.xml"s);

    m_command_manager = m_device->getCommandManager();

    m_frame = 0u;
    //m_prev_frame = 0u;

    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = m_swapchain->getSwapchainImages();
    int max_frames = m_swapchain->getMaxFrames();
    m_per_frame.reserve(max_frames);
    for(int i = 0; i < max_frames; ++i) {
        std::shared_ptr<PerFrame> per_frame = std::make_shared<PerFrame>();

        per_frame->out_color_image = m_resources_manager->create_image("render_target_color", "render_target_color_resource");
        per_frame->out_depth_image = m_resources_manager->create_image("render_target_depth", "render_target_depth_resource");

        per_frame->swapchain_available_sem = m_semaphore_manager->getSemaphore("swapchain_available_sem");
        per_frame->swapchain_available_fen = m_fence_manager->getFence();
        vkResetFences(m_device->getDevice(), 1u, &per_frame->swapchain_available_fen);

        per_frame->render_graph = std::make_shared<RenderGraph>();
        per_frame->render_graph->init(m_device);

        per_frame->init(m_device, i);
        per_frame->command_buffer = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::GRAPICS);
        per_frame->cmd_submit_finish_fence = per_frame->command_buffer->getRenderFence();
        
        per_frame->present_render_node = std::make_shared<PresentRenderNode>();
        per_frame->present_render_node->init(m_device, "presenter"s, per_frame->render_graph);
        per_frame->present_render_node->addReadDependency(swapchain_images.at(i), "swapchain_image"s);
        per_frame->render_graph->add_pass(per_frame->present_render_node);

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

    m_per_frame[image_index]->present_render_node->addWaitSemaphore(m_per_frame[image_index]->cmd_submit_finish_signal_sem);
}

void VulkanRenderer::recordCommandBuffer(CommandBatch& command_buffer, unsigned image_index) {
    VulkanCommandManager::beginCommandBuffer(command_buffer);

    const std::vector<std::shared_ptr<DependencyLevel>>& dependency_levels = m_per_frame[image_index]->render_graph->getDependencyLevels();
    for (auto dependency_it = dependency_levels.begin(); dependency_it != dependency_levels.end(); ++dependency_it) {
        const std::shared_ptr<DependencyLevel>& dependency_lvl = *dependency_it;

        for(int blend_pass = 0; blend_pass <= 1; ++blend_pass) {

            for (auto const&[framebuffer_name, graphics_nodes] : dependency_lvl->getFramebufferNodeMap()) {
                for(const DependencyLevel::PipelineName& pipeline_name : dependency_lvl->getFramebufferToPipelineMap().at(framebuffer_name)) {

                    const std::shared_ptr<GraphicsRenderNode>& node_params = dependency_lvl->getPipelineNodeMap().at(pipeline_name)[0];
                    if(node_params->getPipeline()->getPipelineConfig()->haveBlendEnableAttachments() && blend_pass == 0){
                        break;
                    }

                    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = node_params->getPipeline()->getRenderPass();

                    VkRenderPassBeginInfo renderpass_info{};
                    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    renderpass_info.renderPass = render_pass_ptr->getRenderPass();
                    renderpass_info.framebuffer = node_params->getFramebuffer();
                    renderpass_info.renderArea.offset = node_params->getGraphicsRenderNodeConfig()->getFramebufferConfig()->getOffset2D();
                    renderpass_info.renderArea.extent = node_params->getGraphicsRenderNodeConfig()->getFramebufferConfig()->getExtent2D();
                    renderpass_info.clearValueCount = render_pass_ptr->getRenderPassConfig()->getClearValues().size();
                    renderpass_info.pClearValues = render_pass_ptr->getRenderPassConfig()->getClearValues().data();

                    vkCmdBeginRenderPass(command_buffer.getCommandBufer(), &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
                    vkCmdBindPipeline(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, node_params->getPipeline()->getPipeline());

                    for(const std::shared_ptr<GraphicsRenderNode>& graphics_node : dependency_lvl->getPipelineNodeMap().at(pipeline_name)) {
                        graphics_node->render(command_buffer, image_index);
                    }
                    vkCmdEndRenderPass(command_buffer.getCommandBufer());
                }
            }
        }
    }

    VulkanCommandManager::endCommandBuffer(command_buffer);
}

void VulkanRenderer::drawFrame(unsigned image_index) {
    uint32_t prev_frame = getPrevFrame();

    recordCommandBuffer(*m_per_frame[image_index]->command_buffer, image_index);
    
    CommandBatch::BatchWaitInfo wait_info;
    wait_info.wait_for_semaphores.push_back(m_per_frame[prev_frame]->swapchain_available_sem);
    if(m_per_frame[image_index]->cmd_submit_wait_sem.size()) {
        wait_info.wait_for_semaphores.insert(wait_info.wait_for_semaphores.end(), m_per_frame[image_index]->cmd_submit_wait_sem.begin(), m_per_frame[image_index]->cmd_submit_wait_sem.end());
    }
    wait_info.wait_for_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    VkSubmitInfo submit_info = m_per_frame[image_index]->command_buffer->getSubmitInfo(&wait_info);

    m_command_manager->submitCommandBuffer(m_per_frame[image_index]->command_buffer, VulkanCommandManager::SELECT_ALL_BUFFERS, &submit_info);

    m_per_frame[image_index]->present_render_node->render(*m_per_frame[image_index]->command_buffer, image_index);
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
    if(m_prev_frame.size() >= m_swapchain->getSwapchainSupportDetails().capabilities.minImageCount) {
        vkWaitForFences(m_device->getDevice(), 1u, &(m_per_frame[m_prev_frame.front()]->swapchain_available_fen), VK_TRUE, UINT64_MAX);
        vkResetFences(m_device->getDevice(), 1u, &(m_per_frame[m_prev_frame.front()]->swapchain_available_fen));
        m_prev_frame.erase(m_prev_frame.begin());
    }

    if(m_prev_frame.empty()) {
        m_prev_frame.push_back(m_frame);
        //m_prev_frame.push_back(m_swapchain->getMaxFrames() - 1u);
    }
    if(m_prev_frame.back() != m_frame) {
        m_prev_frame.push_back(m_frame);
    }

    //vkResetFences(m_device->getDevice(), 1u, &(m_per_frame[m_prev_frame.back()]->swapchain_available_fen));
    uint32_t prev_frame = getPrevFrame();
    VkResult result = vkAcquireNextImageKHR(
        m_device->getDevice(),
        m_swapchain->getSwapchain(),
        UINT64_MAX,
        m_per_frame[prev_frame]->swapchain_available_sem,
        m_per_frame[prev_frame]->swapchain_available_fen,
        //VK_NULL_HANDLE,
        &m_frame
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || result == VK_NOT_READY) {
        return {false, m_frame};
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    return {true, m_frame};
}

uint32_t VulkanRenderer::getPrevFrame() const {
    return m_prev_frame.back();
    //return m_prev_frame.empty() ? m_swapchain->getMaxFrames() - 1u : m_prev_frame.back();
    //return m_prev_frame.empty() ? 0u : m_prev_frame.back();
}