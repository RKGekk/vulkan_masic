#include "model_data.h"

#include <mutex>
#include <utility>

ModelData::ModelData() : m_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {}

const std::shared_ptr<VertexBuffer>& ModelData::GetVertexBuffer() const {
    return m_vertex_buffer;
}

void ModelData::SetPrimitiveTopology(VkPrimitiveTopology primitive_toplogy) {
    m_primitive_topology = primitive_toplogy;
}

VkPrimitiveTopology ModelData::GetPrimitiveTopology() const {
    return m_primitive_topology;
}

void ModelData::SetVertexBuffer(std::shared_ptr<VertexBuffer> vertex_buffer) {
    m_vertex_buffer = std::move(vertex_buffer);
}

size_t ModelData::GetIndexCount() const {
    size_t index_count = 0;
    if (m_vertex_buffer) {
        index_count = m_vertex_buffer->getIndicesCount();
    }

    return index_count;
}

size_t ModelData::GetVertexCount() const {
    size_t vertex_count = 0u;

    if (m_vertex_buffer) {
        vertex_count = m_vertex_buffer->getVertexCount();
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

VkFormat getVkFormat(VertexAttributeFormat attrib_format) {
    switch (attrib_format) {
        case VertexAttributeFormat::FLOAT : return VK_FORMAT_R32_SFLOAT;
        case VertexAttributeFormat::FLOAT_VEC2 : return VK_FORMAT_R32G32_SFLOAT;
        case VertexAttributeFormat::FLOAT_VEC3 : return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexAttributeFormat::FLOAT_VEC4 : return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexAttributeFormat::INT : return VK_FORMAT_R32_SINT;
        case VertexAttributeFormat::INT_VEC2 : return VK_FORMAT_R32G32_SINT;
        case VertexAttributeFormat::INT_VEC3 : return VK_FORMAT_R32G32B32_SINT;
        case VertexAttributeFormat::INT_VEC4 : return VK_FORMAT_R32G32B32A32_SINT;
        case VertexAttributeFormat::UINT : return VK_FORMAT_R32_UINT;
        case VertexAttributeFormat::UINT_VEC2 : return VK_FORMAT_R32G32_UINT;
        case VertexAttributeFormat::UINT_VEC3 : return VK_FORMAT_R32G32B32_UINT;
        case VertexAttributeFormat::UINT_VEC4 : return VK_FORMAT_R32G32B32A32_UINT;
        default : return VK_FORMAT_R32_SFLOAT;
    }
}

VkPipelineVertexInputStateCreateInfo ModelData::GetVertextInputInfo() const {
    static std::vector<VkVertexInputBindingDescription> binding_desc(1);
    static std::once_flag binding_exe_flag;
    std::call_once(binding_exe_flag, [this](){
        binding_desc[0].binding = 0u;
        binding_desc[0].stride = m_vertex_format.getVertexSize();
        binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    });

    static std::vector<VkVertexInputAttributeDescription> attribute_desc;
    static std::once_flag attribute_exe_flag;
    std::call_once(attribute_exe_flag, [this](){
        size_t sz = m_vertex_format.getVertexAttribCount();
        attribute_desc.resize(sz);
        for (size_t i = 0; i < sz; ++i) {
            attribute_desc[i].binding = 0u;
            attribute_desc[i].location = i;
            attribute_desc[i].format = getVkFormat(m_vertex_format.getAttribFormat(i));
            attribute_desc[i].offset = m_vertex_format.getOffset(i);
        }
    });

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_desc.size());
    vertex_input_info.pVertexBindingDescriptions = binding_desc.data();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();

    return vertex_input_info;
}

const VertexFormat& ModelData::GetVertexFormat() {
    return m_vertex_format;
}

void ModelData::SetVertexFormat(const VertexFormat& format) {
    m_vertex_format = format;
}