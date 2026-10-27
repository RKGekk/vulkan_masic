#include "present_render_node.h"

#include "../../application.h"
#include "../api/vulkan_swapchain.h"
#include "image_buffer_config.h"
#include "graphics_render_node.h"
#include "graphics_render_node_config.h"
#include "framebuffer_config.h"
#include "../api/vulkan_framebuffer.h"
#include "../api/vulkan_render_pass.h"
#include "render_pass_config.h"
#include "../api/vulkan_command_buffer.h"

bool PresentRenderNode::init(std::string render_node_name, unsigned frame_index, std::shared_ptr<VulkanDevice> device, const std::string& node_config_name, bool instance_config, std::weak_ptr<RenderGraph> render_graph) {
    m_render_node_name = std::move(render_node_name);
    m_frame_index = frame_index;
    m_device = std::move(device);
    m_render_graph = std::move(render_graph);
    m_present_count = 0u;
    
    m_swapchain = Application::GetRenderer().getSwapchain();

    m_present_info = {};
    m_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    m_present_info.swapchainCount = 1u;
    m_present_info.pSwapchains = m_swapchain->getSwapchainPtr();
    m_present_info.pResults = nullptr;

    setExecutionBypass(false);
    setExecutionOrder(0u);

    const std::shared_ptr<VulkanSemaphoresManager>& semaphore_manager = Application::GetRenderer().getSemaphoreManager();
    //std::shared_ptr<VulkanSemaphore> 
    //TransitionResourcesToProperState();

    const std::shared_ptr<VulkanCommandManager>& command_manager = Application::GetRenderer().getCommandManager();
    
    m_command_buffer = command_manager->allocCommandBufferPtr(PoolTypeEnum::TRANSFER, 1u, )
    m_dirty_secondary_command_buffer = true;

    return true;
}

void PresentRenderNode::destroy() {

}

void PresentRenderNode::render(std::shared_ptr<CommandBatch>& command_buffer) {
    TransitionResourcesToProperState(command_buffer);

    m_present_info.waitSemaphoreCount = static_cast<uint32_t>(m_present_wait_sem.size());
    m_present_info.pWaitSemaphores = m_present_wait_sem.data();
    //m_present_info.swapchainCount = 1u;
    m_present_info.pImageIndices = &m_frame_index;
    VkResult result = vkQueuePresentKHR(Application::GetRenderer().getCommandManager()->getQueue(), &m_present_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    ++m_present_count;
}

RenderNode::RenderNodeType PresentRenderNode::getRenderNodeType() const {
    return RenderNode::RenderNodeType::Present;
}

void PresentRenderNode::finishRenderNode() {

}

void PresentRenderNode::TransitionResourcesToProperState(std::shared_ptr<CommandBatch>& command_buffer) {
    std::shared_ptr<RenderGraph> render_graph = m_render_graph.lock();
    //size_t my_node_id = render_graph->getTopologicalIdentity(shared_from_this());

    for(const auto&[read_resource_gloabal_name, att_slot] : getReadResourcesMap()){
        RenderResource::Type att_slot_res_type = att_slot.resource->getType();
        if(att_slot_res_type != RenderResource::Type::IMAGE) continue;

        const std::shared_ptr<VulkanImageBuffer>& current_resource = getReadAttachedImageResource(att_slot.attached_as);
        const std::shared_ptr<ImageBufferConfig>& current_resource_cfg = current_resource->getImageConfig();
        size_t last_rw_by_node_id = render_graph->getLastRWIdentity(shared_from_this(), read_resource_gloabal_name);
        if(last_rw_by_node_id == RenderGraph::NO_ID && m_present_count == 0u && current_resource_cfg->getAfterInitLayout() != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            current_resource->changeLayout(command_buffer, current_resource_cfg->getAfterInitLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            continue;
        }
        if(last_rw_by_node_id == RenderGraph::NO_ID) continue;
        
        const std::shared_ptr<RenderNode>& last_rw_render_node = render_graph->getRenderNodeByID(last_rw_by_node_id);
        //bool is_written = last_rw_render_node->isWrittenGlobal(read_resource_gloabal_name);
        bool is_read = last_rw_render_node->isReadGlobal(read_resource_gloabal_name);
        const RenderNode::LocalName& bind_name = last_rw_render_node->getAttachedAsName(read_resource_gloabal_name);
        if(last_rw_render_node->getRenderNodeType() == RenderNode::RenderNodeType::Graphics) {
            std::shared_ptr<GraphicsRenderNode> last_rw_graphics_node = std::static_pointer_cast<GraphicsRenderNode>(last_rw_render_node);
            const std::shared_ptr<GraphicsRenderNodeConfig>& last_rw_graphics_node_cfg = last_rw_graphics_node->getGraphicsRenderNodeConfig();
            if(is_read) {
                VkImageLayout rw_image_layout = last_rw_graphics_node_cfg->getUpdateMetadata(bind_name)->read_image_layout;
                if(rw_image_layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
                    current_resource->changeLayout(command_buffer, rw_image_layout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
                }
            }
            else {
                const std::shared_ptr<VulkanFramebuffer>& last_rw_frame_buffer = last_rw_graphics_node->getFB();
                //const std::shared_ptr<FramebufferConfig>& last_rw_graphics_node_fb_cfg = last_rw_graphics_node_cfg->getFramebufferConfig();
                const std::shared_ptr<VulkanRenderPass>& last_rw_renderpass = last_rw_frame_buffer->getRenderpass();
                const std::shared_ptr<RenderPassConfig>& last_rw_renderpass_cfg = last_rw_renderpass->getRenderPassConfig();
                const VkAttachmentDescription& last_rw_renderpass_att_desc = last_rw_renderpass_cfg->getAttachmentDescription(bind_name);
                if(last_rw_renderpass_att_desc.finalLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
                    current_resource->changeLayout(command_buffer, last_rw_renderpass_att_desc.finalLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
                }
            }
        }
    }
}

void PresentRenderNode::addWaitSemaphore(VkSemaphore present_wait_for_semaphore) {
    m_present_wait_sem.push_back(present_wait_for_semaphore);
}

void PresentRenderNode::clearWaitSemaphores() {
    m_present_wait_sem.clear();
}