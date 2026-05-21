#include "push_constant_config.h"

#include "../../tools/string_tools.h"
#include "../api/vulkan_device.h"

bool PushConstantConfig::init(std::string name, const pugi::xml_node& const_data) {
    m_name = std::move(name);
    m_push_constant_range = {};

    pugi::xml_node stages_node = const_data.child("Stages");
	if (stages_node) {
        for (pugi::xml_node flag_node = stages_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
            m_push_constant_range.stageFlags |= getShaderStageFlag(flag_node.text().as_string());
        }
    }

    pugi::xml_node push_constants_node = const_data.child("Constants");
	if (!push_constants_node) return false;
    
    uint32_t offset = 0u;
    m_raw_size = 0u;
    uint32_t m_largest_member_alignment = 4u;
    for (pugi::xml_node constant_node = push_constants_node.first_child(); constant_node; constant_node = constant_node.next_sibling()) {
        ShaderConstant shader_constant{};
        shader_constant.vk_format = getFormat(constant_node.child("InternalFormat").text().as_string());
        shader_constant.glsl_format = getInputAttributeGLSLFormat(constant_node.child("GLSLFormat").text().as_string());
        shader_constant.size = VulkanDevice::getBytesCount(shader_constant.vk_format);
        shader_constant.offset = offset;
        shader_constant.allignment = getGLSLAlignment(shader_constant.glsl_format);
        shader_constant.name = constant_node.attribute("field_name").as_string();
        if(shader_constant.allignment > m_largest_member_alignment) m_largest_member_alignment = shader_constant.allignment;

        ShaderConstantMetadataId constant_id = m_push_constants_names_map.size();
        m_push_constants_metadata.push_back(shader_constant);
        m_push_constants_names_map[shader_constant.name] = constant_id;

        //offset += bytes_for_type;
        offset += shader_constant.allignment;
        m_raw_size += shader_constant.size;
	}
    m_total_size = m_push_constants_metadata.back().offset + m_push_constants_metadata.back().size;
    if(m_total_size % m_largest_member_alignment) {
        m_total_size += m_total_size % m_largest_member_alignment;
    }

    m_push_constant_range.offset = 0u;
    m_push_constant_range.size = m_total_size;

    return true;
}

uint32_t PushConstantConfig::getGLSLAlignment(VertexAttributeGLSLFormat glsl_format) {
    switch (glsl_format) {
        case VertexAttributeGLSLFormat::FLOAT : return 4u;
        case VertexAttributeGLSLFormat::INT : return 4u;
        case VertexAttributeGLSLFormat::UINT : return 4u;
        case VertexAttributeGLSLFormat::BOOL : return 4u;

        case VertexAttributeGLSLFormat::FLOAT_VEC2 : return 8u;
        case VertexAttributeGLSLFormat::INT_VEC2 : return 8u;
        case VertexAttributeGLSLFormat::UINT_VEC2 : return 8u;
        case VertexAttributeGLSLFormat::BOOL_VEC2 : return 8u;

        case VertexAttributeGLSLFormat::FLOAT_VEC3 : return 16u;
        case VertexAttributeGLSLFormat::INT_VEC3 : return 16u;
        case VertexAttributeGLSLFormat::UINT_VEC3 : return 16u;
        case VertexAttributeGLSLFormat::BOOL_VEC3 : return 16u;

        case VertexAttributeGLSLFormat::FLOAT_VEC4 : return 16u;
        case VertexAttributeGLSLFormat::INT_VEC4 : return 16u;
        case VertexAttributeGLSLFormat::UINT_VEC4 : return 16u;
        case VertexAttributeGLSLFormat::BOOL_VEC4 : return 16u;

        default : return 4u;
    }
}

std::shared_ptr<PushConstantConfig> PushConstantConfig::makeInstance(std::string name) const {
    using namespace std::literals;

    std::shared_ptr<PushConstantConfig> instance_ptr = std::make_shared<PushConstantConfig>();
    *instance_ptr = *this;
    instance_ptr->m_name = std::move(name);

    return instance_ptr;
}

const VkPushConstantRange& PushConstantConfig::getPushConstantRange() const {
    return m_push_constant_range;
}

void PushConstantConfig::setPushConstantRangeOffset(uint32_t offset) {
    m_push_constant_range.offset = offset;
}

const std::vector<PushConstantConfig::ShaderConstant>& PushConstantConfig::getAllPushConstantsMetadata() const {
    return m_push_constants_metadata;
}

const PushConstantConfig::ShaderConstant& PushConstantConfig::getPushConstantsMetadata(const PushConstantConfig::ShaderConstantName& name) const {
    return m_push_constants_metadata.at(m_push_constants_names_map.at(name));
}

const std::unordered_map<PushConstantConfig::ShaderConstantName, PushConstantConfig::ShaderConstantMetadataId>& PushConstantConfig::getPushConstantsNamesMap() const {
    return m_push_constants_names_map;
}

bool PushConstantConfig::hasPushConstantsName(const PushConstantConfig::ShaderConstantName& name) const {
    return m_push_constants_names_map.contains(name);
}

uint32_t PushConstantConfig::getLargestMemberAlignment() const {
    return m_largest_member_alignment;
}

uint32_t PushConstantConfig::getTotalSize() const {
    return m_total_size;
}

uint32_t PushConstantConfig::getRawSize() const {
    return m_raw_size;
}