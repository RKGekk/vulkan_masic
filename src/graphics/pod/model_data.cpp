#include "model_data.h"

#include <mutex>
#include <utility>

#include "../api/vulkan_buffer.h"

ModelData::ModelData() : m_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {}

const std::shared_ptr<VulkanBuffer>& ModelData::GetVertexBuffer(VertexFormat::BindingNum binding) const {
    return m_vertex_buffers.at(binding);
}

void ModelData::SetPrimitiveTopology(VkPrimitiveTopology primitive_toplogy) {
    m_primitive_topology = primitive_toplogy;
}

VkPrimitiveTopology ModelData::GetPrimitiveTopology() const {
    return m_primitive_topology;
}

void ModelData::SetVertexBuffer(std::shared_ptr<VulkanBuffer> vertex_buffer, VertexFormat::BindingNum binding) {
    m_vertex_buffers[binding] = std::move(vertex_buffer);
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
        index_count = m_index_buffer->getNotAlignedSize() / m_shader_signature.getIndexTypeBytesCount();;
    }

    return index_count;
}

size_t ModelData::GetInstanceCount() const {
    size_t instance_count = 1u;
    if (!m_vertex_buffers.empty()) {
        for(const auto& [binding_num, vulkan_buffer] : m_vertex_buffers) {
            const VertexFormat& vf = m_shader_signature.getInputAttributes(binding_num);
            if(vf.getInputRate() == VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE) {
                instance_count = vulkan_buffer->getNotAlignedSize() / vf.getVertexSize();
                break;
            }
        }
    }

    return instance_count;
}

size_t ModelData::GetVertexCount() const {
    size_t vertex_count = 0u;

    if (!m_vertex_buffers.empty()) {
        for(const auto& [binding_num, vulkan_buffer] : m_vertex_buffers) {
            const VertexFormat& vf = m_shader_signature.getInputAttributes(binding_num);
            if(vf.getInputRate() == VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX) {
                vertex_count = vulkan_buffer->getNotAlignedSize() / vf.getVertexSize();
                break;
            }
        }
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

void ModelData::SetSphere(const BoundingSphere& sphere) {
    m_sphere = sphere;
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

const std::shared_ptr<ShaderSignature>& ModelData::GetShaderSignature() const {
    return m_shader_signature;
}

void ModelData::SetShaderSignature(std::shared_ptr<ShaderSignature> format) {
    m_shader_signature = std::move(format);
}