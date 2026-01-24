#include "shader_signature.h"

#include "../api/vulkan_device.h"\
#include "../../tools/string_tools.h"

std::vector<char> getDataFromXmlValue(pugi::xml_node value_data, VertexAttributeFormat value_format) {
    std::vector<char> data;

    switch (value_format) {
        case VertexAttributeFormat::FLOAT : {
            float value = value_data.text().as_float();
            data.resize(4);
            *(float*)(data.data()) = value;
        } break;
        case VertexAttributeFormat::FLOAT_VEC2 : {} break;
        case VertexAttributeFormat::FLOAT_VEC3 : {} break;
        case VertexAttributeFormat::FLOAT_VEC4 : {} break;
        case VertexAttributeFormat::INT : {
            int value = value_data.text().as_int();
            data.resize(4);
            *(int*)(data.data()) = value;
        } break;
        case VertexAttributeFormat::INT_VEC2 : {} break;
        case VertexAttributeFormat::INT_VEC3 : {} break;
        case VertexAttributeFormat::INT_VEC4 : {} break;
        case VertexAttributeFormat::UINT : {
            uint32_t value = value_data.text().as_uint();
            data.resize(4);
            *(uint32_t*)(data.data()) = value;
        } break;
        case VertexAttributeFormat::UINT_VEC2 : {} break;
        case VertexAttributeFormat::UINT_VEC3 : {} break;
        case VertexAttributeFormat::UINT_VEC4 : {} break;
        case VertexAttributeFormat::DOUBLE : {} break;
        case VertexAttributeFormat::DOUBLE_VEC2 : {} break;
        case VertexAttributeFormat::DOUBLE_VEC3 : {} break;
        case VertexAttributeFormat::DOUBLE_VEC4 : {} break;
        case VertexAttributeFormat::BOOL : {} break;
        case VertexAttributeFormat::BOOL_VEC2 : {} break;
        case VertexAttributeFormat::BOOL_VEC3 : {} break;
        case VertexAttributeFormat::BOOL_VEC4 : {} break;
        default: break;
    }

    return data;
}

bool ShaderSignature::init(const pugi::xml_node& shader_data) {
    using namespace std::literals;

    m_name = shader_data.attribute("name").as_string();
	m_file_name = shader_data.child("FilePath").text().as_string();

    m_create_flags = {};
    pugi::xml_node flags_node = shader_data.child("Flags");
	if (flags_node) {
        for (pugi::xml_node flag_node = flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			m_create_flags |= getShaderCreateFlagBitsEXT(flag_node.text().as_string());
		}
    }

    m_pipeline_shader_stage_create_flags = {};
    pugi::xml_node pipeline_shader_stage_create_flags_node = shader_data.child("PipelineShaderStageCreateFlags");
	if (pipeline_shader_stage_create_flags_node) {
        for (pugi::xml_node flag_node = pipeline_shader_stage_create_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			m_pipeline_shader_stage_create_flags |= getPipelineShaderStageCreateFlagBits(flag_node.text().as_string());
		}
    }

    pugi::xml_node stages_node = shader_data.child("Stage");
	if (stages_node) {
        m_stage = getShaderStageFlagBits(stages_node.text().as_string());
    }

    pugi::xml_node entry_point_name_node = shader_data.child("EntryPointName");
	if (entry_point_name_node) {
        m_entry_point_name = entry_point_name_node.text().as_string();
    }
    else {
        m_entry_point_name = "main"s;
    }

    pugi::xml_node input_attributes_node = shader_data.child("InputAttributeDescription");
	if (input_attributes_node) {
        for (pugi::xml_node attribute_node = input_attributes_node.first_child(); attribute_node; attribute_node = attribute_node.next_sibling()) {
            int location = attribute_node.child("Location").text().as_int(0);
            VertexAttributeFormat attr_format = getInputAttributeFormat(attribute_node.child("Format").text().as_string());
            int num = attribute_node.child("Semantic").attribute("num").as_int(0);
            VertexAttributeSemantic semantic = getVertexAttributeSemantic(attribute_node.child("Semantic").text().as_string());
            std::string name = attribute_node.attribute("name").as_string();
            m_input_attributes.setVertexAttribute({semantic, num}, attr_format, location);
		}
    }

    pugi::xml_node descriptor_set_node = shader_data.child("DescriptorSet");
	if (descriptor_set_node) {
        for (pugi::xml_node set_node = descriptor_set_node.first_child(); set_node; set_node = set_node.next_sibling()) {
            m_desc_set_names.push_back(set_node.text().as_string());
        }
    }

    pugi::xml_node push_constants_node = shader_data.child("PushConstants");
	if (push_constants_node) {
        uint32_t offset = 0u;
        for (pugi::xml_node constant_node = push_constants_node.first_child(); constant_node; constant_node = constant_node.next_sibling()) {
            VertexAttributeFormat const_format = getInputAttributeFormat(constant_node.child("Format").text().as_string());
            size_t bytes_for_type = VertexFormat::getBytesForType(const_format);
            VkPushConstantRange constant_info{};
            constant_info.offset = offset;
            constant_info.size = bytes_for_type;
            offset += bytes_for_type;
			m_push_constants.push_back(constant_info);
		}
    }

    pugi::xml_node specialization_constants_node = shader_data.child("SpecializationConstants");
	if (specialization_constants_node) {
        uint32_t offset = 0u;
        for (pugi::xml_node constant_node = specialization_constants_node.first_child(); constant_node; constant_node = constant_node.next_sibling()) {
            VkSpecializationMapEntry map_entry{};

            map_entry.constantID = constant_node.child("ConstantID").text().as_int(0);
            map_entry.offset = offset;

            VertexAttributeFormat const_format = getInputAttributeFormat(constant_node.child("Type").text().as_string());
            size_t bytes_for_type = VertexFormat::getBytesForType(const_format);
            map_entry.size = bytes_for_type;

            offset += bytes_for_type;
            
			m_specialization_constants.push_back(map_entry);

            std::vector<char> data = getDataFromXmlValue(constant_node.child("Value"), const_format);
            m_specialization_constants_data.insert(m_specialization_constants_data.end(), data.begin(), data.end());
		}
        m_specialization_info = VkSpecializationInfo{};
        m_specialization_info.mapEntryCount = m_specialization_constants.size();
        m_specialization_info.pMapEntries = m_specialization_constants.data();
        m_specialization_info.dataSize = m_specialization_constants_data.size();
        m_specialization_info.pData = m_specialization_constants_data.data();
    }

    return true;
}

bool ShaderSignature::init(const std::string& rg_file_path) {

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node shaders_node = root_node.child("Shaders");
	if (shaders_node) {
		for (pugi::xml_node shader_node = shaders_node.first_child(); shader_node; shader_node = shader_node.next_sibling()) {
			return init(shader_node);
		}
	}

    return true;
}

const VertexFormat& ShaderSignature::getVertexFormat() const {
    return m_vertex_format;
}

const std::string& ShaderSignature::getName() const {
    return m_name;
}

const std::string& ShaderSignature::getFileName() const {
    return m_file_name;
}

const std::string& ShaderSignature::getEntryPointName() const {
    return m_entry_point_name;
}

VkFlags ShaderSignature::getShaderCreateFlags() const {
    return m_create_flags;
}

VkFlags ShaderSignature::getPipelineShaderStageCreateFlags() const {
    return m_pipeline_shader_stage_create_flags;
}

VkShaderStageFlagBits ShaderSignature::getStage() const {
    return m_stage;
}

const VertexFormat& ShaderSignature::getInputAttributes() const {
    return m_input_attributes;
}

const std::vector<std::string>& ShaderSignature::getDescSetNames() const {
    return m_desc_set_names;
}

const std::vector<VkPushConstantRange>& ShaderSignature::getPushConstantsRanges() const {
    return m_push_constants;
}

const std::vector<VkSpecializationMapEntry>& ShaderSignature::getSpecializationConstantsMap() const {
    return m_specialization_constants;
}

const std::vector<char>& ShaderSignature::getSpecializationConstantsData() const {
    return m_specialization_constants_data;
}

const VkSpecializationInfo& ShaderSignature::getSpecializationInfo() const {
    return m_specialization_info;
}