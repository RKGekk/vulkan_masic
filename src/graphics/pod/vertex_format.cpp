#include "vertex_format.h"
#include "../../tools/memory_utility.h"
#include "../api/vulkan_device.h"

#include <regex>

bool SemanticName::init(const std::string& semantic_name) {
    using namespace std::literals;

    std::regex sem_reg(R"(^([A-Z]+)_*(\d+)*$)");
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
    else if(name == "OTHER"s) {
        semantic = VertexAttributeSemantic::OTHER;
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

void VertexFormat::addVertexAttribute(SemanticName semantic_name, VertexAttributeGLSLFormat glsl_format, VkFormat internal_format, std::string name) {
    size_t pos = 0u;
    if(m_pos_semantic_map.size() > 0u) {
        pos = (*(m_pos_semantic_map.end())).first + 1u;
    }

    m_pos_semantic_map[pos] = semantic_name;
    m_glsl_format_pos_map[pos] = glsl_format;
    m_internal_format_pos_map[pos] = internal_format;
    m_semantic_pos_map[semantic_name] = pos;
    m_name_pos_map[pos] = std::move(name);
}

void VertexFormat::setVertexAttribute(SemanticName semantic_name, VertexAttributeGLSLFormat glsl_format, VkFormat internal_format, Location location, std::string name) {
    m_pos_semantic_map[location] = semantic_name;
    m_glsl_format_pos_map[location] = glsl_format;
    m_internal_format_pos_map[location] = internal_format;
    m_semantic_pos_map[semantic_name] = location;
    m_name_pos_map[location] = std::move(name);
}

bool VertexFormat::checkVertexAttribExist(SemanticName semantic) const {
    return m_semantic_pos_map.count(semantic);
}

size_t VertexFormat::getVertexAttribPos(SemanticName semantic) const {
    return m_semantic_pos_map.at(semantic);
}

SemanticName VertexFormat::getPosSemantic(Location pos) const {
    return m_pos_semantic_map.at(pos);
}

VertexAttributeGLSLFormat VertexFormat::getAttribGLSLFormat (Location pos) const {
    return m_glsl_format_pos_map.at(pos);
}

VkFormat VertexFormat::getAttribInternalFormat (Location pos) const {
    return m_internal_format_pos_map.at(pos);
}

const std::string& VertexFormat::getAttribName(Location pos) const {
    return m_name_pos_map.at(pos);
}

VertexAttributeGLSLFormat VertexFormat::getAttribGLSLFormat (SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) return VertexAttributeGLSLFormat::FLOAT;
    return m_glsl_format_pos_map.at(m_semantic_pos_map.at(semantic));
}

VkFormat VertexFormat::getAttribInternalFormat (SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) return VK_FORMAT_R8G8B8A8_UNORM;
    return m_internal_format_pos_map.at(m_semantic_pos_map.at(semantic));
}

size_t VertexFormat::GetNumComponentsInGLSLType(SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) 1u;
    return GetNumComponentsInGLSLType(getAttribGLSLFormat(semantic));
}

size_t VertexFormat::GetNumComponentsInVkType(SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) 1u;
    return VulkanDevice::getNumComponents(getAttribInternalFormat(semantic));
}

size_t VertexFormat::GetComponentSizeInVkType(SemanticName semantic) const {
    return VulkanDevice::getBytesCount(getAttribInternalFormat(semantic)) / GetNumComponentsInVkType(semantic);
}

size_t VertexFormat::getOffset(SemanticName semantic) const {
    size_t offset = 0u;
    if(!m_semantic_pos_map.count(semantic)) return offset;

    for(const auto&[location, current_semantic] : m_pos_semantic_map) {
        if(current_semantic == semantic) return offset;
        VkFormat curr_format = m_internal_format_pos_map.at(location);
        size_t bytes_ct = VulkanDevice::getBytesCount(curr_format);
        offset += bytes_ct;
    }
    return offset;
}

size_t VertexFormat::getOffset(Location pos) const {
    size_t offset = 0u;

    for(const auto&[location, semantic] : m_pos_semantic_map) {
        if(pos == location) return offset;
        VkFormat curr_format = m_internal_format_pos_map.at(location);
        size_t bytes_ct = VulkanDevice::getBytesCount(curr_format);
        offset += bytes_ct;
    }
    return offset;
}

size_t VertexFormat::getVertexAttribCount() const {
    return m_semantic_pos_map.size();
}

size_t VertexFormat::getVertexSize() const {
    size_t stride = 0u;

    for(const auto&[location, semantic] : m_pos_semantic_map) {
        VkFormat curr_format = m_internal_format_pos_map.at(location);
        size_t bytes_ct = VulkanDevice::getBytesCount(curr_format);
        stride += bytes_ct;
    }
    return stride;
}

const std::map<VertexFormat::Location, SemanticName>& VertexFormat::getPosSemanticMap() const {
    return m_pos_semantic_map;
}

VkVertexInputRate VertexFormat::getInputRate() const {
    return m_input_rate;
}

void VertexFormat::setInputRate(VkVertexInputRate rate) {
    m_input_rate = rate;
}

VertexFormat::BindingNum VertexFormat::getBindingNum() const {
    return m_binding_num;
}

void VertexFormat::setBindingNum(BindingNum num) {
    m_binding_num = num;
}

const std::string& VertexFormat::getVertexBufferBindingName() const {
    return m_vertex_buffer_binding_name;
}

void VertexFormat::setVertexBufferBindingName(std::string name) {
    m_vertex_buffer_binding_name = std::move(name);
}

uint32_t VertexFormat::getVertexBufferOffset() const {
    return m_vertex_buffer_offset;
}

void VertexFormat::setVertexBufferOffset(uint32_t offset) {
    m_vertex_buffer_offset = offset;
}

const std::string& VertexFormat::getVertexBufferResourceType() const {
    return m_vertex_buffer_resource_type;
}

void VertexFormat::setVertexBufferResourceType(std::string res_type) {
    m_vertex_buffer_resource_type = std::move(res_type);
}