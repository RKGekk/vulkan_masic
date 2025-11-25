#include "vertex_format.h"
#include "../tools/memory_utility.h"

#include <regex>

void VertexFormat::addVertexAttribute(const SemanticName& name, VertexAttributeFormat format) {
    size_t pos = m_names_pos.size();
    const auto[type, number] = getSemanticParams(name);

    m_names_pos.push_back(name);
    m_format_pos.push_back(format);
    m_name_pos_map[name] = pos;
    m_semantic_name_number[name] = number;
    m_semantic_attrib_type[name] = type;
}

size_t VertexFormat::getVertexAttribPos(const VertexFormat::SemanticName& name) const {
    return m_name_pos_map.at(name);
}

const VertexFormat::SemanticName& VertexFormat::getPosSemanticName(size_t pos) const {
    return m_names_pos.at(pos);
}

VertexFormat::VertexAttributeFormat VertexFormat::getAttribFormat (size_t pos) const {
    return m_format_pos.at(pos);
}

VertexFormat::VertexAttributeFormat VertexFormat::getAttribFormat (const VertexFormat::SemanticName& name) const {
    if(!m_name_pos_map.count(name)) return VertexFormat::VertexAttributeFormat::FLOAT;
    return m_format_pos.at(m_name_pos_map.at(name));
}

size_t getBytesForType(VertexFormat::VertexAttributeFormat format) {
    switch (format) {
        case VertexFormat::VertexAttributeFormat::FLOAT : return 4;
        case VertexFormat::VertexAttributeFormat::FLOAT_VEC2 : return 8;
        case VertexFormat::VertexAttributeFormat::FLOAT_VEC3 : return 12;
        case VertexFormat::VertexAttributeFormat::FLOAT_VEC4 : return 16;
        case VertexFormat::VertexAttributeFormat::INT : return 4;
        case VertexFormat::VertexAttributeFormat::INT_VEC2 : return 8;
        case VertexFormat::VertexAttributeFormat::INT_VEC3 : return 12;
        case VertexFormat::VertexAttributeFormat::INT_VEC4 : return 16;
        default : return 4;
    }
}

size_t VertexFormat::getStride(const VertexFormat::SemanticName& name) const {
    size_t stride = 0u;
    if(!m_name_pos_map.count(name)) return stride;

    size_t to = m_name_pos_map.at(name);
    for(size_t i = 0u; i < to; ++i) {
        VertexAttributeFormat curr_format = m_format_pos.at(i);
        size_t bytes_ct = getBytesForType(curr_format);
        stride += bytes_ct;
    }
    return stride;
}

size_t VertexFormat::getStride(size_t pos) const {
    size_t stride = 0u;

    for(size_t i = 0u; i < pos; ++i) {
        VertexAttributeFormat curr_format = m_format_pos.at(i);
        size_t bytes_ct = getBytesForType(curr_format);
        stride += bytes_ct;
    }
    return stride;
}

size_t VertexFormat::getVertexAttribCount() const {
    return m_names_pos.size();
}

size_t VertexFormat::getVertexSize() const {
    size_t stride = 0u;

    size_t sz = m_names_pos.size();
    for(size_t i = 0u; i < sz; ++i) {
        VertexAttributeFormat curr_format = m_format_pos.at(i);
        size_t bytes_ct = getBytesForType(curr_format);
        stride += bytes_ct;
    }
    return stride;
}

std::pair<VertexFormat::VertexAttributeType, VertexFormat::SemanticNumber> VertexFormat::getSemanticParams(const VertexFormat::SemanticName& name) {
    SemanticNumber number {};
    VertexAttributeType type {};
    std::regex rx_numbers("[0-9]+");
    std::regex_constants::match_flag_type fl = std::regex_constants::match_default;
    std::smatch number_match;
    bool is_num = std::regex_search(name, number_match, rx_numbers, fl);
    if(is_num) {
        std::string sem_num_str = number_match[0];
        number = std::stoi(sem_num_str);
    }
    
    if(name.find("POSITION") != std::string::npos) {
        type = VertexAttributeType::POSITION;
    }
    else if(name.find("NORMAL") != std::string::npos) {
        type = VertexAttributeType::NORMAL;
    }
    else if(name.find("TANGENT") != std::string::npos) {
        type = VertexAttributeType::TANGENT;
    }
    else if(name.find("BINORMAL") != std::string::npos) {
        type = VertexAttributeType::BINORMAL;
    }
    else if(name.find("TEXCOORD") != std::string::npos) {
        type = VertexAttributeType::TEXCOORD;
    }
    else if(name.find("COLOR") != std::string::npos) {
        type = VertexAttributeType::COLOR;
    }
    else if(name.find("JOINTS") != std::string::npos) {
        type = VertexAttributeType::JOINTS;
    }
    else if(name.find("WEIGHTS") != std::string::npos) {
        type = VertexAttributeType::WEIGHTS;
    }

    return {type, number};
}