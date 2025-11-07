#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_device.h"
#include "vulkan_buffer.h"
#include "render_resource.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

class VertexBuffer : public RenderResource {
public:

    template<typename VertexType, typename IndexType>
    bool init(std::shared_ptr<VulkanDevice> device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices, VkPipelineVertexInputStateCreateInfo vertex_info) {
        CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
        bool result = init(device, command_buffer, vertices, indices, vertex_info);
        m_device->getCommandManager().submitCommandBuffer(command_buffer);
        m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);

        return result;
    }

    template<typename VertexType, typename IndexType>
    bool init(std::shared_ptr<VulkanDevice> device, CommandBatch& command_buffer, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices, VkPipelineVertexInputStateCreateInfo vertex_info) {
        m_device = std::move(device);
        m_indices_count = indices.size();
        m_vertex_count = vertices.size();
        m_index_type = getIndexTypeByCount(sizeof(IndexType));

        VkDeviceSize vertices_buffer_size = sizeof(VertexType) * vertices.size();
        m_vertex_buffer = std::make_shared<VulkanBuffer>();
        bool result = m_vertex_buffer->init(m_device, command_buffer, vertices.data(), vertices_buffer_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        VkDeviceSize indices_buffer_size = sizeof(IndexType) * indices.size();
        m_index_buffer = std::make_shared<VulkanBuffer>();
        result &= m_index_buffer->init(m_device, command_buffer, indices.data(), indices_buffer_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        
        m_vertex_info = vertex_info;

        return true;
    }

    bool init(std::shared_ptr<VulkanDevice> device, const void* vertices_data, size_t vertices_count, const void* indices_data, size_t indices_count, VkIndexType index_type, VkPipelineVertexInputStateCreateInfo vertex_info, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void destroy() override;

    size_t getVertexCount() const;
    const std::shared_ptr<VulkanBuffer>& getVertexBuffer() const;

    size_t getIndicesCount() const;
    const std::shared_ptr<VulkanBuffer>& getIndexBuffer() const;

    VkPipelineVertexInputStateCreateInfo getVertextInputInfo() const;
    VkIndexType getIndexType() const;

    template<typename VertexType, typename IndexType>
    void update(CommandBatch& command_buffer, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices) {
        VkDeviceSize new_vertices_buffer_size = m_vertex_info.pVertexBindingDescriptions->stride * vertices.size();
        VkDeviceSize new_indices_buffer_size = getIndexBytesCount(m_index_type) * indices.size();
        if(new_vertices_buffer_size > m_vertex_buffer->getSize() || new_indices_buffer_size > m_index_buffer->getSize()) {
            destroy();
            init(m_device, vertices, indices, m_vertex_info);
            return;
        }
        m_indices_count = indices.size();
        m_vertex_count = vertices.size();
        
        m_vertex_buffer->update(command_buffer, vertices.data(), new_vertices_buffer_size);
        m_index_buffer->update(command_buffer, indices.data(), new_indices_buffer_size);
    }

    template<typename VertexType, typename IndexType>
    void update(const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices) {
        CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
        update(command_buffer, vertices, indices);
        m_device->getCommandManager().submitCommandBuffer(command_buffer);
        m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);
    }

private:

    static size_t getIndexBytesCount(VkIndexType index_type);
    static VkIndexType getIndexTypeByCount(size_t sz);

    std::shared_ptr<VulkanDevice> m_device;

    std::shared_ptr<VulkanBuffer> m_vertex_buffer;
    size_t m_vertex_count = 0u;
    
    std::shared_ptr<VulkanBuffer> m_index_buffer;
    size_t m_indices_count = 0u;

    VkPipelineVertexInputStateCreateInfo m_vertex_info;
    VkIndexType m_index_type;
};