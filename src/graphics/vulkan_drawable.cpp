#include "vulkan_drawable.h"
#include "vulkan_device.h"
#include "vulkan_command_manager.h"

bool VulkanDrawable::init(std::shared_ptr<VulkanDevice> device, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) {
    m_device = std::move(device);

    m_indices_count = indices.size();
    createAndTransferVertexBuffer(vertices);
    createAndTransferIndexBuffer(indices);

    return true;
}

void VulkanDrawable::destroy() {
    vkDestroyBuffer(m_device->getDevice(), m_vertex_buffer.buf, nullptr);
    vkFreeMemory(m_device->getDevice(), m_vertex_buffer.mem, nullptr);
    
    vkDestroyBuffer(m_device->getDevice(), m_index_buffer.buf, nullptr);
    vkFreeMemory(m_device->getDevice(), m_index_buffer.mem, nullptr);
}

size_t VulkanDrawable::getIndicesCount() const {
    return m_indices_count;
}

VulkanBuffer VulkanDrawable::getVertexBuffer() const {
    return m_vertex_buffer;
}
    
VulkanBuffer VulkanDrawable::getIndexBuffer() const {
    return m_index_buffer;
}

void VulkanDrawable::createAndTransferVertexBuffer(const std::vector<Vertex>& vertices) {
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
    VulkanBuffer staging_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void* data;
    vkMapMemory(m_device->getDevice(), staging_buffer.mem, 0, buffer_size, 0u, &data);
    memcpy(data, vertices.data(), buffer_size);
    vkUnmapMemory(m_device->getDevice(), staging_buffer.mem);
    
    m_vertex_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_device->getCommandManager().copyBuffer(staging_buffer.buf, m_vertex_buffer.buf, buffer_size);

    vkDestroyBuffer(m_device->getDevice(), staging_buffer.buf, nullptr);
    vkFreeMemory(m_device->getDevice(), staging_buffer.mem, nullptr);

    m_vertex_input_bind_desc = Vertex::getBindingDescription();
    vertex_input_attr_descs = Vertex::getAttributeDescritpions();
}

void VulkanDrawable::createAndTransferIndexBuffer(const std::vector<uint16_t>& indices) {
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    
    VulkanBuffer staging_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    void* data;
    vkMapMemory(m_device->getDevice(), staging_buffer.mem, 0u, buffer_size, 0u, &data);
    memcpy(data, indices.data(), buffer_size);
    vkUnmapMemory(m_device->getDevice(), staging_buffer.mem);
    
    m_index_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_device->getCommandManager().copyBuffer(staging_buffer.buf, m_index_buffer.buf, buffer_size);
    
    vkDestroyBuffer(m_device->getDevice(), staging_buffer.buf, nullptr);
    vkFreeMemory(m_device->getDevice(), staging_buffer.mem, nullptr);
}