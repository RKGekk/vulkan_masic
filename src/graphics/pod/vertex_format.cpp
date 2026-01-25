#include "vertex_format.h"
#include "../../tools/memory_utility.h"

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

void VertexFormat::addVertexAttribute(SemanticName semantic_name, VertexAttributeFormat format) {
    size_t pos = m_semantic_pos.size();

    m_semantic_pos.push_back(semantic_name);
    m_format_pos.push_back(format);
    m_semantic_pos_map[semantic_name] = pos;
}

void VertexFormat::setVertexAttribute(SemanticName semantic_name, VertexAttributeFormat format, int location) {
    if(m_semantic_pos.size() < location) {
        m_semantic_pos.resize(location);
        m_format_pos.resize(location);
    }

    m_semantic_pos[location] = semantic_name;
    m_format_pos[location] = format;
    m_semantic_pos_map[semantic_name] = location;
}

size_t VertexFormat::getBytesForType(VertexAttributeFormat format) {
    switch (format) {
        case VertexAttributeFormat::FLOAT : return 4;
        case VertexAttributeFormat::FLOAT_VEC2 : return 8;
        case VertexAttributeFormat::FLOAT_VEC3 : return 12;
        case VertexAttributeFormat::FLOAT_VEC4 : return 16;
        case VertexAttributeFormat::INT : return 4;
        case VertexAttributeFormat::INT_VEC2 : return 8;
        case VertexAttributeFormat::INT_VEC3 : return 12;
        case VertexAttributeFormat::INT_VEC4 : return 16;
        case VertexAttributeFormat::UINT : return 4;
        case VertexAttributeFormat::UINT_VEC2 : return 8;
        case VertexAttributeFormat::UINT_VEC3 : return 12;
        case VertexAttributeFormat::UINT_VEC4 : return 16;
        case VertexAttributeFormat::BOOL : return 4;
        case VertexAttributeFormat::BOOL_VEC2 : return 8;
        case VertexAttributeFormat::BOOL_VEC3 : return 12;
        case VertexAttributeFormat::BOOL_VEC4 : return 16;
        case VertexAttributeFormat::DOUBLE : return 8;
        case VertexAttributeFormat::DOUBLE_VEC2 : return 16;
        case VertexAttributeFormat::DOUBLE_VEC3 : return 24;
        case VertexAttributeFormat::DOUBLE_VEC4 : return 32;
        default : return 4;
    }
}

size_t VertexFormat::GetNumComponentsInType(VertexAttributeFormat format) {
    switch (format) {
        case VertexAttributeFormat::FLOAT: return 1u;
        case VertexAttributeFormat::FLOAT_VEC2: return 2u;
        case VertexAttributeFormat::FLOAT_VEC3: return 3u;
        case VertexAttributeFormat::FLOAT_VEC4: return 4u;
        case VertexAttributeFormat::INT: return 1u;
        case VertexAttributeFormat::INT_VEC2: return 2u;
        case VertexAttributeFormat::INT_VEC3: return 3u;
        case VertexAttributeFormat::INT_VEC4: return 4u;
        case VertexAttributeFormat::UINT: return 1u;
        case VertexAttributeFormat::UINT_VEC2: return 2u;
        case VertexAttributeFormat::UINT_VEC3: return 3u;
        case VertexAttributeFormat::UINT_VEC4: return 4u;
        default: return 1u;
    }
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

VertexAttributeFormat VertexFormat::getAttribFormat (size_t pos) const {
    return m_format_pos.at(pos);
}

VertexAttributeFormat VertexFormat::getAttribFormat (SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) return VertexAttributeFormat::FLOAT;
    return m_format_pos.at(m_semantic_pos_map.at(semantic));
}

size_t VertexFormat::GetNumComponentsInType(SemanticName semantic) const {
    if(!m_semantic_pos_map.count(semantic)) 1u;
    return GetNumComponentsInType(getAttribFormat(semantic));
}

size_t VertexFormat::getOffset(SemanticName semantic) const {
    size_t offset = 0u;
    if(!m_semantic_pos_map.count(semantic)) return offset;

    size_t to = m_semantic_pos_map.at(semantic);
    for(size_t i = 0u; i < to; ++i) {
        VertexAttributeFormat curr_format = m_format_pos.at(i);
        size_t bytes_ct = getBytesForType(curr_format);
        offset += bytes_ct;
    }
    return offset;
}

size_t VertexFormat::getOffset(size_t pos) const {
    size_t offset = 0u;

    for(size_t i = 0u; i < pos; ++i) {
        VertexAttributeFormat curr_format = m_format_pos.at(i);
        size_t bytes_ct = getBytesForType(curr_format);
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
        VertexAttributeFormat curr_format = m_format_pos.at(i);
        size_t bytes_ct = getBytesForType(curr_format);
        stride += bytes_ct;
    }
    return stride;
}

VkVertexInputRate VertexFormat::getInputRate() const {
    return m_input_rate;
}

void VertexFormat::setInputRate(VkVertexInputRate rate) {
    m_input_rate = rate;
}