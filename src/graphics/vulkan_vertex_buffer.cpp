#include "vulkan_vertex_buffer.h"

bool VertexBuffer::init(std::shared_ptr<VulkanDevice> device, const void* vertices_data, size_t vertices_count, const void* indices_data, size_t indices_count, VkIndexType index_type, VkPipelineVertexInputStateCreateInfo vertex_info) {
    m_device = std::move(device);
    m_indices_count = indices_count;
    m_index_type = index_type;

    VkDeviceSize vertices_buffer_size = vertex_info.pVertexBindingDescriptions->stride * vertices_count;
    createAndTransferVertexBuffer(vertices_data, vertices_buffer_size);

    VkDeviceSize indices_buffer_size = getIndexBytesCount(index_type) * indices_count;
    createAndTransferIndexBuffer(indices_data, indices_buffer_size);
    m_vertex_info = vertex_info;

    return true;
}

void VertexBuffer::destroy() {
    vkDestroyBuffer(m_device->getDevice(), m_vertex_buffer.buf, nullptr);
    vkFreeMemory(m_device->getDevice(), m_vertex_buffer.mem, nullptr);

    vkDestroyBuffer(m_device->getDevice(), m_index_buffer.buf, nullptr);
    vkFreeMemory(m_device->getDevice(), m_index_buffer.mem, nullptr);
}

size_t VertexBuffer::getIndicesCount() const {
    return m_indices_count;
}

VulkanBuffer VertexBuffer::getVertexBuffer() const {
    return m_vertex_buffer;
}

VulkanBuffer VertexBuffer::getIndexBuffer() const {
    return m_index_buffer;
}

VkPipelineVertexInputStateCreateInfo VertexBuffer::getVertextInputInfo() const {
    return m_vertex_info;
}

VkIndexType VertexBuffer::getIndexType() const {
    return m_index_type;
}

void VertexBuffer::createAndTransferVertexBuffer(const void* vertices_data, VkDeviceSize buffer_size) {

    VulkanBuffer staging_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void* mount_point;
    vkMapMemory(m_device->getDevice(), staging_buffer.mem, 0, buffer_size, 0u, &mount_point);
    memcpy(mount_point, vertices_data, buffer_size);
    vkUnmapMemory(m_device->getDevice(), staging_buffer.mem);

    m_vertex_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
    m_device->getCommandManager().copyBuffer(command_buffer.getCommandBufer(), staging_buffer.buf, m_vertex_buffer.buf, buffer_size);
    m_device->getCommandManager().submitCommandBuffer(command_buffer);
    m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);

    vkDestroyBuffer(m_device->getDevice(), staging_buffer.buf, nullptr);
    vkFreeMemory(m_device->getDevice(), staging_buffer.mem, nullptr);
}

void VertexBuffer::createAndTransferIndexBuffer(const void* indices_data, VkDeviceSize buffer_size) {

    VulkanBuffer staging_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    void* mount_point;
    vkMapMemory(m_device->getDevice(), staging_buffer.mem, 0u, buffer_size, 0u, &mount_point);
    memcpy(mount_point, indices_data, buffer_size);
    vkUnmapMemory(m_device->getDevice(), staging_buffer.mem);

    m_index_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
    m_device->getCommandManager().copyBuffer(command_buffer.getCommandBufer() ,staging_buffer.buf, m_index_buffer.buf, buffer_size);
    m_device->getCommandManager().submitCommandBuffer(command_buffer);
    m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);

    vkDestroyBuffer(m_device->getDevice(), staging_buffer.buf, nullptr);
    vkFreeMemory(m_device->getDevice(), staging_buffer.mem, nullptr);
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