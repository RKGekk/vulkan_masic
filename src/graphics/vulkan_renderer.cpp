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
#include "pod/render_node_config.h"
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

void VulkanRenderer::TransitionResourcesToProperState(const std::shared_ptr<RenderNode>& render_node_ptr, CommandBatch& command_buffer, unsigned image_index) {
    const std::shared_ptr<VulkanPipeline>& pipeline_ptr = render_node_ptr->getPipeline();
    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = pipeline_ptr->getRenderPass();

    for(const auto&[gloabal_name, att_slot] : render_node_ptr->getReadResourcesMap()){
        RenderResource::Type att_slot_res_type = att_slot.resource->getType();
        if(att_slot_res_type == RenderResource::Type::BUFFER) continue;

        size_t last_written_by_node_id = m_per_frame[image_index]->render_graph->getLastWrittenIdentity(render_node_ptr, gloabal_name);
        size_t last_read_by_node_id = m_per_frame[image_index]->render_graph->getLastReadIdentity(render_node_ptr, gloabal_name);

        if(last_read_by_node_id == RenderGraph::NO_ID && last_written_by_node_id == RenderGraph::NO_ID) {
            // std::shared_ptr<VulkanImageBuffer> attached_read_resource = render_node_ptr->getAttachedImageResource(att_slot.attached_as);
            // VkImageLayout current_image_layout = attached_read_resource->getImageConfig()->getAfterInitLayout();
            // VkImageLayout read_image_layout = render_node_ptr->getRenderNodeConfig()->getUpdateMetadata(att_slot.attached_as)->read_image_layout;
            // if(current_image_layout != read_image_layout) {
            //     attached_read_resource->changeLayout(current_image_layout, read_image_layout);
            // }
        }

        if(last_read_by_node_id != RenderGraph::NO_ID && last_read_by_node_id > last_written_by_node_id) {
            continue;
        }

        if(last_written_by_node_id != RenderGraph::NO_ID && last_written_by_node_id > last_read_by_node_id) {
            const RenderGraph::RenderNodePtr& last_written_by_node = m_per_frame[image_index]->render_graph->getRenderNodeByID(last_read_by_node_id);
            
        }
    }

    for(const auto&[gloabal_name, att_slot] : render_node_ptr->getWrittenResourcesMap()){
        RenderResource::Type att_slot_res_type = att_slot.resource->getType();
        if(att_slot_res_type == RenderResource::Type::BUFFER) continue;

        size_t last_written_by_node_id = m_per_frame[image_index]->render_graph->getLastWrittenIdentity(render_node_ptr, gloabal_name);
        size_t last_read_by_node_id = m_per_frame[image_index]->render_graph->getLastReadIdentity(render_node_ptr, gloabal_name);

        if(last_read_by_node_id == RenderGraph::NO_ID && last_written_by_node_id == RenderGraph::NO_ID) {
            std::shared_ptr<VulkanImageBuffer> attached_write_resource = render_node_ptr->getAttachedImageResource(att_slot.attached_as);
            VkImageLayout current_image_layout = attached_write_resource->getImageConfig()->getAfterInitLayout();
            
            const VkAttachmentDescription& attach_desc = render_node_ptr->getPipeline()->getRenderPass()->getRenderPassConfig()->getAttachmentDescription(att_slot.attached_as);
            VkImageLayout write_image_layout = attach_desc.initialLayout;
            if(current_image_layout != write_image_layout) {
                attached_write_resource->changeLayout(current_image_layout, write_image_layout);
                attached_write_resource->getImageConfig()->setAfterInitLayout(write_image_layout);
            }
        }

        if(last_read_by_node_id != RenderGraph::NO_ID && last_read_by_node_id > last_written_by_node_id) {
            continue;
        }

        if(last_written_by_node_id != RenderGraph::NO_ID && last_written_by_node_id > last_read_by_node_id) {
            const RenderGraph::RenderNodePtr& last_written_by_node = m_per_frame[image_index]->render_graph->getRenderNodeByID(last_read_by_node_id);
            
        }
    }
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

    static std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    // return all RenderResources to initial state

    const RenderGraph::RenderNodeList& topologically_sorted_nodes = m_per_frame[image_index]->render_graph->getTopologicallySortedNodes();
    for(const std::shared_ptr<RenderNode> render_node_ptr : topologically_sorted_nodes) {
        const std::shared_ptr<VulkanPipeline>& pipeline_ptr = render_node_ptr->getPipeline();
        const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = pipeline_ptr->getRenderPass();

        TransitionResourcesToProperState(render_node_ptr, command_buffer, image_index);

        VkRenderPassBeginInfo renderpass_info{};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_info.renderPass = render_pass_ptr->getRenderPass();
        renderpass_info.framebuffer = render_node_ptr->getFramebuffer();
        renderpass_info.renderArea.offset = {0, 0};
        renderpass_info.renderArea.extent = render_node_ptr->getViewportExtent();
        renderpass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        renderpass_info.pClearValues = clear_values.data();
        
        VkViewport view_port{};
        view_port.x = 0.0f;
        view_port.y = 0.0f;
        view_port.width = static_cast<float>(render_node_ptr->getViewportExtent().width);
        view_port.height = static_cast<float>(render_node_ptr->getViewportExtent().height);
        view_port.minDepth = 0.0f;
        view_port.maxDepth = 1.0f;
        
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = render_node_ptr->getViewportExtent();
        
        vkCmdBeginRenderPass(command_buffer.getCommandBufer(), &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(command_buffer.getCommandBufer(), 0u, 1u, &view_port);
        vkCmdSetScissor(command_buffer.getCommandBufer(), 0u, 1u, &scissor);
        
        for(const auto&[slot, desc] : render_node_ptr->getDescriptors()) {
            vkCmdBindDescriptorSets(
                command_buffer.getCommandBufer(), // commandBuffer
                VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
                render_node_ptr->getPipeline()->getPipelineLayout(), // pipeline layout
                slot, // first set
                1, // descriptor set count
                desc->getDescriptorSetPtr(), // descriptor sets pointer
                0, // dynamic offset count
                nullptr // dynamic offsets pointer
            );
        }

        vkCmdBindPipeline(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ptr->getPipeline());
        
        for (const VertexFormat& vf : pipeline_ptr->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature()->getInputAttributes()) {
            const std::string& vertex_buffer_name = vf.getVertexBufferBindingName();
            std::shared_ptr<VulkanBuffer> vertex_buffer = render_node_ptr->getReadAttachedBufferResource(vertex_buffer_name);
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(
                command_buffer.getCommandBufer(), // commandBuffer
                vf.getBindingNum(), // firstBinding
                1, // bindingCount
                vertex_buffer->getBufferPtr(), // buffers pointer
                offsets // offsets pointer
            );
        }
        const std::string& index_buffer_name = pipeline_ptr->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature()->getVertexFormat().getIndexBufferBindingName();
        std::shared_ptr<VulkanBuffer> index_buffer = render_node_ptr->getReadAttachedBufferResource(index_buffer_name);
        vkCmdBindIndexBuffer(
            command_buffer.getCommandBufer(), // commandBuffer
            index_buffer->getBuffer(), // buffer
            0u, // offset
            pipeline_ptr->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature()->getVertexFormat().getIndexType() // indexType
        );
        
        uint32_t index_count = index_buffer->getNotAlignedSize() / pipeline_ptr->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature()->getVertexFormat().getIndexTypeBytesCount();
        vkCmdDrawIndexed(
            command_buffer.getCommandBufer(), // commandBuffer
            index_count, // indexCount
            1u, // instanceCount
            0u, // firstIndex
            0u, // vertexOffset
            0u // firstInstance
        );
        
        vkCmdEndRenderPass(command_buffer.getCommandBufer());
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