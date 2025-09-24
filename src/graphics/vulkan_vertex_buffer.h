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
    virtual VkIndexType getIndexType () const = 0;
};

class VertexBuffer : public IVertexBuffer {
public:

    template<typename VertexType, typename IndexType>
    bool init(std::shared_ptr<VulkanDevice> device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices, VkPipelineVertexInputStateCreateInfo vertex_info) {
        m_device = std::move(device);
        m_indices_count = indices.size();
        m_index_type = getIndexTypeByCount(sizeof(indices[0]));

        VkDeviceSize vertices_buffer_size = sizeof(vertices[0]) * vertices.size();
        createAndTransferVertexBuffer(vertices.data(), vertices_buffer_size);

        VkDeviceSize indices_buffer_size = sizeof(indices[0]) * indices.size();
        createAndTransferIndexBuffer(indices.data(), indices_buffer_size);
        m_vertex_info = vertex_info;

        return true;
    }

    bool init(std::shared_ptr<VulkanDevice> device, const void* vertices_data, size_t vertices_count, const void* indices_data, size_t indices_count, VkIndexType index_type, VkPipelineVertexInputStateCreateInfo vertex_info);
    void destroy() override;
    size_t getIndicesCount() const override;
    VulkanBuffer getVertexBuffer() const override;
    VulkanBuffer getIndexBuffer() const override;
    VkPipelineVertexInputStateCreateInfo getVertextInputInfo() const override;
    VkIndexType getIndexType() const override;

private:

    void createAndTransferVertexBuffer(const void* vertices_data, VkDeviceSize buffer_size);
    void createAndTransferIndexBuffer(const void* indices_data, VkDeviceSize buffer_size);

    static size_t getIndexBytesCount(VkIndexType index_type);
    static VkIndexType getIndexTypeByCount(size_t sz);

    std::shared_ptr<VulkanDevice> m_device;

    VulkanBuffer m_vertex_buffer;
    VulkanBuffer m_index_buffer;
    size_t m_indices_count = 0u;
    VkPipelineVertexInputStateCreateInfo m_vertex_info;
    VkIndexType m_index_type;
};