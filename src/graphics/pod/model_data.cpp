#include "model_data.h"

#include <mutex>
#include <utility>

#include "../api/vulkan_buffer.h"

ModelData::ModelData() : m_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {}

const std::shared_ptr<VulkanBuffer>& ModelData::GetVertexBuffer() const {
    return m_vertex_buffer;
}

void ModelData::SetPrimitiveTopology(VkPrimitiveTopology primitive_toplogy) {
    m_primitive_topology = primitive_toplogy;
}

VkPrimitiveTopology ModelData::GetPrimitiveTopology() const {
    return m_primitive_topology;
}

void ModelData::SetVertexBuffer(std::shared_ptr<VulkanBuffer> vertex_buffer) {
    m_vertex_buffer = std::move(vertex_buffer);
}

void ModelData::SetIndexBuffer(std::shared_ptr<VulkanBuffer> index_buffer) {
    m_index_buffer = std::move(index_buffer);
}

const std::shared_ptr<VulkanBuffer>& ModelData::GetIndexBuffer() const {
    return m_index_buffer;
}

size_t ModelData::GetIndexCount() const {
    size_t index_count = 0u;
    if (m_index_buffer) {
        index_count = m_index_buffer->getSize() / m_vertex_format.getIndexTypeBytesCount();;
    }

    return index_count;
}

size_t ModelData::GetVertexCount() const {
    size_t vertex_count = 0u;

    if (m_vertex_buffer) {
        vertex_count = m_vertex_buffer->getSize() /  m_vertex_format.getVertexSize();
    }

    return vertex_count;
}

void ModelData::SetMaterial(std::shared_ptr<Material> material) {
    m_material = std::move(material);
}

std::shared_ptr<Material> ModelData::GetMaterial() const {
    return m_material;
}

void ModelData::SetAABB(const BoundingBox& aabb) {
    m_AABB = aabb;
}

const BoundingBox& ModelData::GetAABB() const {
    return m_AABB;
}

const BoundingSphere& ModelData::GetSphere() const {
    return m_sphere;
}

const std::string& ModelData::GetName() const {
    return m_name;
}

void ModelData::SetName(std::string name) {
    m_name = std::move(name);
}

const VertexFormat& ModelData::GetVertexFormat() {
    return m_vertex_format;
}

void ModelData::SetVertexFormat(const VertexFormat& format) {
    m_vertex_format = format;
}