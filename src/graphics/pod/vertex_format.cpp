#include "vertex_format.h"
#include "../../tools/memory_utility.h"
#include "../api/vulkan_device.h"

#include <regex>

bool SemanticName::init(std::string semantic_name) {
    using namespace std::literals;

    std::regex sem_reg(R"(^([A-Z]+)_(\d+)$)");
    std::smatch matches;
    if (!std::regex_search(semantic_name, matches, sem_reg)) return false;
    
    std::string name = matches[1];

    if(name == "POSITION"s) {
        semantic = VertexAttributeSemantic::POSITION;
    }
    else if(name == "NORMAL"s) {
        semantic = VertexAttributeSemantic::NORMAL;
    }
    else if(name == "TANGENT"s) {
        semantic = VertexAttributeSemantic::TANGENT;
    }
    else if(name == "BINORMAL"s) {
        semantic = VertexAttributeSemantic::BINORMAL;
    }
    else if(name == "TEXCOORD"s) {
        semantic = VertexAttributeSemantic::TEXCOORD;
    }
    else if(name == "COLOR"s) {
        semantic = VertexAttributeSemantic::COLOR;
    }
    else if(name == "JOINTS"s) {
        semantic = VertexAttributeSemantic::JOINTS;
    }
    else if(name == "WEIGHTS"s) {
        semantic = VertexAttributeSemantic::WEIGHTS;
    }
    else return false;

    if(matches.length() == 3) {
        num = std::stoi(matches[2]);
    }
    
    return true;
};

bool SemanticName::operator==(const SemanticName& other) const {
    return semantic == other.semantic && num == other.num;
}

size_t VertexFormat::getBytesForType(VkFormat format) {
    return VulkanDevice::getBytesCount(format);
}

size_t VertexFormat::GetNumComponentsInGLSLType(VertexAttributeGLSLFormat glsl_format) {
    switch (glsl_format) {
        case VertexAttributeGLSLFormat::FLOAT : return 1;
        case VertexAttributeGLSLFormat::FLOAT_VEC2 : return 2;
        case VertexAttributeGLSLFormat::FLOAT_VEC3 : return 3;
        case VertexAttributeGLSLFormat::FLOAT_VEC4 : return 4;
        case VertexAttributeGLSLFormat::INT : return 1;
        case VertexAttributeGLSLFormat::INT_VEC2 : return 2;
        case VertexAttributeGLSLFormat::INT_VEC3 : return 3;
        case VertexAttributeGLSLFormat::INT_VEC4 : return 4;
        case VertexAttributeGLSLFormat::UINT : return 1;
        case VertexAttributeGLSLFormat::UINT_VEC2 : return 2;
        case VertexAttributeGLSLFormat::UINT_VEC3 : return 3;
        case VertexAttributeGLSLFormat::UINT_VEC4 : return 4;
        case VertexAttributeGLSLFormat::DOUBLE : return 1;
        case VertexAttributeGLSLFormat::DOUBLE_VEC2 : return 2;
        case VertexAttributeGLSLFormat::DOUBLE_VEC3 : return 3;
        case VertexAttributeGLSLFormat::DOUBLE_VEC4 : return 4;
        case VertexAttributeGLSLFormat::BOOL : return 1;
        case VertexAttributeGLSLFormat::BOOL_VEC2 : return 2;
        case VertexAttributeGLSLFormat::BOOL_VEC3 : return 3;
        case VertexAttributeGLSLFormat::BOOL_VEC4 : return 4;
        default : return 1;
    }
}

void VertexFormat::addVertexAttribute(SemanticName semantic_name, VertexAttributeGLSLFormat glsl_format, VkFormat internal_format) {
    size_t pos = m_semantic_pos.size();

    m_semantic_pos.push_back(semantic_name);
    m_glsl_format_pos.push_back(glsl_format);
    m_internal_format_pos.push_back(internal_format);
    m_semantic_pos_map[semantic_name] = pos;
}

void VertexFormat::setVertexAttribute(SemanticName semantic_name, VertexAttributeGLSLFormat glsl_format, VkFormat internal_format, int location) {
    if(m_semantic_pos.size() <= location) {
        m_semantic_pos.resize(location + 1);
        m_glsl_format_pos.resize(location + 1);
        m_internal_format_pos.resize(location + 1);
    }

    m_semantic_pos[location] = semantic_name;
    m_glsl_format_pos[location] = glsl_format;
    m_internal_format_pos[location] = internal_format;
    m_semantic_pos_map[semantic_name] = location;
}

bool VertexFormat::checkVertexAttribExist(SemanticName semantic) const {
    return m_semantic_pos_map.count(semantic);
}

size_t VertexFormat::getVertexAttribPos(SemanticName semantic) const {
    return m_semantic_pos_map.at(semantic);
}

SemanticName VertexFormat::getPosSemantic(size_t pos) const {
    return m_semantic_pos.at(pos);
}

VertexAttributeGLSLFormat VertexFormat::getAttribGLSLFormat (size_t pos) const {
    return m_glsl_format_pos.at(pos);
}

VkFormat VertexFormat::getAttribInternalFormat (size_t pos) const {
    return m_internal_format_pos.at(pos);
}

VertexAttributeGLSLFormat VertexFormat::getAttribGLSLFormat (SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) return VertexAttributeGLSLFormat::FLOAT;
    return m_glsl_format_pos.at(m_semantic_pos_map.at(semantic));
}

VkFormat VertexFormat::getAttribInternalFormat (SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) return VK_FORMAT_R8G8B8A8_UNORM;
    return m_internal_format_pos.at(m_semantic_pos_map.at(semantic));
}

size_t VertexFormat::GetNumComponentsInGLSLType(SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) 1u;
    return GetNumComponentsInGLSLType(getAttribGLSLFormat(semantic));
}

size_t VertexFormat::GetNumComponentsInVkType(SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) 1u;
    return VulkanDevice::getNumComponents(getAttribInternalFormat(semantic));
}

size_t VertexFormat::getOffset(SemanticName semantic) const {
    size_t offset = 0u;
    if(!m_semantic_pos_map.count(semantic)) return offset;

    size_t to = m_semantic_pos_map.at(semantic);
    for(size_t i = 0u; i < to; ++i) {
        VkFormat curr_format = m_internal_format_pos.at(i);
        size_t bytes_ct = VulkanDevice::getBytesCount(curr_format);
        offset += bytes_ct;
    }
    return offset;
}

size_t VertexFormat::getOffset(size_t pos) const {
    size_t offset = 0u;

    for(size_t i = 0u; i < pos; ++i) {
        VkFormat curr_format = m_internal_format_pos.at(i);
        size_t bytes_ct = VulkanDevice::getBytesCount(curr_format);
        offset += bytes_ct;
    }
    return offset;
}

size_t VertexFormat::getVertexAttribCount() const {
    return m_semantic_pos.size();
}

size_t VertexFormat::getVertexSize() const {
    size_t stride = 0u;

    size_t sz = m_semantic_pos.size();
    for(size_t i = 0u; i < sz; ++i) {
        VkFormat curr_format = m_internal_format_pos.at(i);
        size_t bytes_ct = VulkanDevice::getBytesCount(curr_format);
        stride += bytes_ct;
    }
    return stride;
}

VkIndexType VertexFormat::getIndexType() const {
    return m_index_type;
}

uint32_t VertexFormat::getIndexTypeBytesCount() const {
    switch (m_index_type) {
        case VK_INDEX_TYPE_UINT16 : return 2u;
        case VK_INDEX_TYPE_UINT32 : return 4u;
        case VK_INDEX_TYPE_NONE_KHR : return 0u;
        case VK_INDEX_TYPE_UINT8_KHR : return 1u;
        default : return 0;
    }
}

void VertexFormat::setIndexType(VkIndexType idx_type) {
    m_index_type = idx_type;
}

VkVertexInputRate VertexFormat::getInputRate() const {
    return m_input_rate;
}

void VertexFormat::setInputRate(VkVertexInputRate rate) {
    m_input_rate = rate;
}

size_t VertexFormat::getBindingNum() const {
    return m_binding_num;
}

void VertexFormat::setBindingNum(size_t num) {
    m_binding_num = num;
}

const std::string& VertexFormat::getVertexBufferBindingName() const {
    return m_vertex_buffer_binding_name;
}

void VertexFormat::setVertexBufferBindingName(std::string name) {
    m_vertex_buffer_binding_name = std::move(name);
}

const std::string& VertexFormat::getIndexBufferBindingName() const {
    return m_index_buffer_binding_name;
}

void VertexFormat::setIndexBufferBindingName(std::string name) {
    m_index_buffer_binding_name = std::move(name);
}

const std::string& VertexFormat::getVertexBufferResourceType() const {
    return m_vertex_buffer_resource_type;
}

void VertexFormat::setVertexBufferResourceType(std::string res_type) {
    m_vertex_buffer_resource_type = std::move(res_type);
}
    
const std::string& VertexFormat::getIndexBufferResourceType() const {
    return m_index_buffer_resource_type;
}

void VertexFormat::setIndexBufferResourceType(std::string res_type) {
    m_index_buffer_resource_type = std::move(res_type);
}