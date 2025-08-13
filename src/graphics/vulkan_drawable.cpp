#include "vulkan_drawable.h"
#include "vulkan_device.h"
#include "vulkan_command_manager.h"

bool VulkanDrawable::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<VulkanPipeline> pipeline, std::shared_ptr<IVertexBuffer> vertex_buffer) {
    m_device = std::move(device);
    m_vertex_buffer = std::move(vertex_buffer);
    m_pipeline = std::move(pipeline);

    return true;
}

void VulkanDrawable::destroy() {
    m_vertex_buffer->destroy();
}

void VulkanDrawable::recordCommandBuffer(VkCommandBuffer command_buffer) {
    
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipeline());

    VkBuffer vertex_buffers[] = {m_vertex_buffer->getVertexBuffer().buf};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, m_vertex_buffer->getIndexBuffer().buf, 0u, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(m_vertex_buffer->getIndicesCount()), 1u, 0u, 0u, 0u);
}