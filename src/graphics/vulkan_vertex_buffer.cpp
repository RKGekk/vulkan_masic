#include "vulkan_vertex_buffer.h"

bool VertexBuffer::init(std::shared_ptr<VulkanDevice> device, const void* vertices_data, size_t vertices_count, const void* indices_data, size_t indices_count, VkIndexType index_type, VkPipelineVertexInputStateCreateInfo vertex_info, VkMemoryPropertyFlags properties) {
    m_device = std::move(device);
    m_indices_count = indices_count;
    m_vertex_count = vertices_count;
    m_index_type = index_type;
    m_vertex_info = vertex_info;

    VkDeviceSize vertices_buffer_size = vertex_info.pVertexBindingDescriptions->stride * vertices_count;
    VkDeviceSize indices_buffer_size = getIndexBytesCount(index_type) * indices_count;

    bool result = false;
    if(vertices_data) {
        CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
        
        m_vertex_buffer = std::make_shared<VulkanBuffer>();
        bool result = m_vertex_buffer->init(m_device, command_buffer, vertices_data, vertices_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, properties);
        
        m_index_buffer = std::make_shared<VulkanBuffer>();
        result &= m_index_buffer->init(m_device, command_buffer, indices_data, indices_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, properties);
        
        m_device->getCommandManager().submitCommandBuffer(command_buffer);
        m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);
    }
    else {
        m_vertex_buffer = std::make_shared<VulkanBuffer>();
        bool result = m_vertex_buffer->init(m_device, vertices_data, vertices_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, properties);
        
        m_index_buffer = std::make_shared<VulkanBuffer>();
        result &= m_index_buffer->init(m_device, indices_data, indices_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, properties);
    }

    return result;
}

bool VertexBuffer::init(std::shared_ptr<VulkanDevice> device, CommandBatch& command_buffer, const void* vertices_data, size_t vertices_count, const void* indices_data, size_t indices_count, VkIndexType index_type, VkPipelineVertexInputStateCreateInfo vertex_info, VkMemoryPropertyFlags properties) {
    bool result = init(std::move(device), nullptr, vertices_count, nullptr, indices_count, index_type, vertex_info, properties);

    if(vertices_data) {
        VkDeviceSize vertices_buffer_size = vertex_info.pVertexBindingDescriptions->stride * vertices_count;
        VkDeviceSize indices_buffer_size = getIndexBytesCount(index_type) * indices_count;

        m_vertex_buffer->update(command_buffer, vertices_data, vertices_buffer_size);
        m_index_buffer->update(command_buffer, indices_data, indices_buffer_size);
    }

    return result;
}

void VertexBuffer::destroy() {
    m_vertex_buffer->destroy();
    m_index_buffer->destroy();
}

size_t VertexBuffer::getVertexCount() const {
    return m_vertex_count;
};

const std::shared_ptr<VulkanBuffer>& VertexBuffer::getVertexBuffer() const {
    return m_vertex_buffer;
}

size_t VertexBuffer::getIndicesCount() const {
    return m_indices_count;
};

const std::shared_ptr<VulkanBuffer>& VertexBuffer::getIndexBuffer() const {
    return m_index_buffer;
}

VkPipelineVertexInputStateCreateInfo VertexBuffer::getVertextInputInfo() const {
    return m_vertex_info;
}

VkIndexType VertexBuffer::getIndexType() const {
    return m_index_type;
}

size_t VertexBuffer::getIndexBytesCount(VkIndexType index_type) {
    switch (index_type) {
        case VK_INDEX_TYPE_UINT16 : return 2u;
        case VK_INDEX_TYPE_UINT32 : return 4u;
        case VK_INDEX_TYPE_NONE_KHR : return 4u;
        case VK_INDEX_TYPE_UINT8_KHR : return 1u;
        default: return 4u;
    }
}

VkIndexType VertexBuffer::getIndexTypeByCount(size_t sz) {
    switch (sz) {
        case 1 : return VK_INDEX_TYPE_UINT8_KHR;
        case 2 : return VK_INDEX_TYPE_UINT16;
        case 4 : return VK_INDEX_TYPE_UINT32;
        default: return VK_INDEX_TYPE_UINT32;
    }
}