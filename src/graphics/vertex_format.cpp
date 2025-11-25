#include "vertex_format.h"
#include "../tools/memory_utility.h"

#include <regex>

void VertexFormat::addVertexAttribute(const SemanticName& name, VertexAttributeFormat format) {
    size_t pos = m_names_pos.size();
    const auto[type, number] = getSemanticParams(name);
    
    m_names_pos.push_back(name);
    m_format_pos.push_back(format);
    m_name_pos_map[name] = pos;
    
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