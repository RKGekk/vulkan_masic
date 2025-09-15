#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_device.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

class IVertexBuffer {
public:
    virtual void destroy() = 0;

    virtual size_t getIndicesCount() const = 0;
    virtual VulkanBuffer getVertexBuffer() const = 0;
    virtual VulkanBuffer getIndexBuffer() const = 0;
    virtual VkPipelineVertexInputStateCreateInfo getVertextInputInfo() const = 0;
};

template<typename VertexType, typename IndexType>
class VertexBuffer : public IVertexBuffer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices, VkPipelineVertexInputStateCreateInfo vertex_info) {
        m_device = std::move(device);
        m_indices_count = indices.size();
        createAndTransferVertexBuffer(vertices);
        createAndTransferIndexBuffer(indices);
        m_vertex_info = vertex_info;

        return true;
    }

    void destroy() override {
        vkDestroyBuffer(m_device->getDevice(), m_vertex_buffer.buf, nullptr);
        vkFreeMemory(m_device->getDevice(), m_vertex_buffer.mem, nullptr);
    
        vkDestroyBuffer(m_device->getDevice(), m_index_buffer.buf, nullptr);
        vkFreeMemory(m_device->getDevice(), m_index_buffer.mem, nullptr);
    }

    size_t getIndicesCount() const override {
        return m_indices_count;
    }

    VulkanBuffer getVertexBuffer() const override {
        return m_vertex_buffer;
    }

    VulkanBuffer getIndexBuffer() const override {
        return m_index_buffer;
    }

    VkPipelineVertexInputStateCreateInfo getVertextInputInfo() const override {
        return m_vertex_info;
    }

private:
    void createAndTransferVertexBuffer(const std::vector<VertexType>& vertices) {
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
        VulkanBuffer staging_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        void* data;
        vkMapMemory(m_device->getDevice(), staging_buffer.mem, 0, buffer_size, 0u, &data);
        memcpy(data, vertices.data(), buffer_size);
        vkUnmapMemory(m_device->getDevice(), staging_buffer.mem);
    
        m_vertex_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
        VulkanCommandManager::beginCommandBuffer(command_buffer);
        m_device->getCommandManager().copyBuffer(command_buffer.getCommandBufer(), staging_buffer.buf, m_vertex_buffer.buf, buffer_size);
        VulkanCommandManager::endCommandBuffer(command_buffer);
        m_device->getCommandManager().submitCommandBuffer(command_buffer);
        m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);

        vkDestroyBuffer(m_device->getDevice(), staging_buffer.buf, nullptr);
        vkFreeMemory(m_device->getDevice(), staging_buffer.mem, nullptr);
    }

    void createAndTransferIndexBuffer(const std::vector<IndexType>& indices) {
        VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    
        VulkanBuffer staging_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        void* data;
        vkMapMemory(m_device->getDevice(), staging_buffer.mem, 0u, buffer_size, 0u, &data);
        memcpy(data, indices.data(), buffer_size);
        vkUnmapMemory(m_device->getDevice(), staging_buffer.mem);
    
        m_index_buffer = m_device->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
        VulkanCommandManager::beginCommandBuffer(command_buffer);
        m_device->getCommandManager().copyBuffer(command_buffer.getCommandBufer() ,staging_buffer.buf, m_index_buffer.buf, buffer_size);
        VulkanCommandManager::endCommandBuffer(command_buffer);
        m_device->getCommandManager().submitCommandBuffer(command_buffer);
        m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);
    
        vkDestroyBuffer(m_device->getDevice(), staging_buffer.buf, nullptr);
        vkFreeMemory(m_device->getDevice(), staging_buffer.mem, nullptr);
    }

    std::shared_ptr<VulkanDevice> m_device;

    VulkanBuffer m_vertex_buffer;
    VulkanBuffer m_index_buffer;
    size_t m_indices_count = 0u;
    VkPipelineVertexInputStateCreateInfo m_vertex_info;
};