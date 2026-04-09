#include "present_render_node.h"

#include "../../application.h"
#include "../api/vulkan_swapchain.h"

bool PresentRenderNode::init(std::shared_ptr<VulkanDevice> device, const std::string& node_config_name, std::weak_ptr<RenderGraph> render_graph) {
    m_device = std::move(device);
    m_render_graph = std::move(render_graph);
    
    m_swapchain = Application::GetRenderer().getSwapchain();

    m_present_info = {};
    m_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    m_present_info.swapchainCount = 1u;
    m_present_info.pSwapchains = m_swapchain->getSwapchainPtr();
    m_present_info.pResults = nullptr;

    return true;
}

void PresentRenderNode::destroy() {

}

void PresentRenderNode::render(CommandBatch& command_buffer, unsigned image_index) {
    m_present_info.waitSemaphoreCount = static_cast<uint32_t>(m_present_wait_sem.size());
    m_present_info.pWaitSemaphores = m_present_wait_sem.data();
    m_present_info.swapchainCount = 1u;
    m_present_info.pImageIndices = &image_index;
    VkResult result = vkQueuePresentKHR(Application::GetRenderer().getCommandManager()->getQueue(), &m_present_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void PresentRenderNode::finishRenderNode() {

}

void PresentRenderNode::TransitionResourcesToProperState(CommandBatch& command_buffer) {
    std::shared_ptr<RenderGraph> render_graph = m_render_graph.lock();

    for(const auto&[gloabal_name, att_slot] : getReadResourcesMap()){
        RenderResource::Type att_slot_res_type = att_slot.resource->getType();
        if(att_slot_res_type == RenderResource::Type::BUFFER) continue;

        size_t last_written_by_node_id = render_graph->getLastWrittenIdentity(shared_from_this(), gloabal_name);
        size_t last_read_by_node_id = render_graph->getLastReadIdentity(shared_from_this(), gloabal_name);

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
            const RenderGraph::RenderNodePtr& last_written_by_node = render_graph->getRenderNodeByID(last_read_by_node_id);
            
        }
    }
}

void PresentRenderNode::addWaitSemaphore(VkSemaphore present_wait_for_semaphore) {
    m_present_wait_sem.push_back(present_wait_for_semaphore);
}

void PresentRenderNode::clearWaitSemaphores() {
    m_present_wait_sem.clear();
}