#include "shader_signature.h"

#include "../api/vulkan_device.h"
#include "../../tools/string_tools.h"

std::vector<char> getDataFromXmlValue(pugi::xml_node value_data, VkFormat value_format) {
    std::vector<char> data;

    size_t bytes_count = VulkanDevice::getBytesCount(value_format);
    size_t num_components = VulkanDevice::getNumComponents(value_format);
    VulkanDevice::VulkanFormatComponentType component_type = VulkanDevice::getComponentType(value_format);

    if(bytes_count == 1) {
        char value = value_data.text().as_uint();
        data.resize(4);
        *(char*)(data.data()) = value;
    }
    else if(bytes_count == 2) {
        uint16_t value = value_data.text().as_uint();
        data.resize(4);
        *(uint16_t*)(data.data()) = value;
    }
    else if(bytes_count == 3) {
        
    }
    else if(bytes_count == 4) {
        uint32_t value = value_data.text().as_float();
        data.resize(4);
        *(uint32_t*)(data.data()) = value;
    }
    else if(bytes_count == 6) {
        
    }
    else if(bytes_count == 8) {
        
    }
    else if(bytes_count == 12) {
        
    }
    else if(bytes_count == 16) {
        
    }

    return data;
}

bool ShaderSignature::init(const pugi::xml_node& shader_data) {
    using namespace std::literals;

    m_name = shader_data.attribute("name").as_string();
	m_file_name = shader_data.child("FilePath").attribute("file_name").as_string();

    m_create_flags = {};
    pugi::xml_node flags_node = shader_data.child("Flags");
	if (flags_node) {
        for (pugi::xml_node flag_node = flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			m_create_flags |= getShaderCreateFlagEXT(flag_node.text().as_string());
		}
    }

    m_pipeline_shader_stage_create_flags = {};
    pugi::xml_node pipeline_shader_stage_create_flags_node = shader_data.child("PipelineShaderStageCreateFlags");
	if (pipeline_shader_stage_create_flags_node) {
        for (pugi::xml_node flag_node = pipeline_shader_stage_create_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			m_pipeline_shader_stage_create_flags |= getPipelineShaderStageCreateFlag(flag_node.text().as_string());
		}
    }

    pugi::xml_node stages_node = shader_data.child("Stage");
	if (stages_node) {
        m_stage = getShaderStageFlag(stages_node.text().as_string());
    }

    pugi::xml_node entry_point_name_node = shader_data.child("EntryPointName");
	if (entry_point_name_node) {
        m_entry_point_name = entry_point_name_node.text().as_string();
    }
    else {
        m_entry_point_name = "main"s;
    }

    pugi::xml_node input_attributes_desc_node = shader_data.child("InputAttributeDescription");
	if (input_attributes_desc_node) {
        for (pugi::xml_node binding_node = input_attributes_desc_node.first_child(); binding_node; binding_node = binding_node.next_sibling()) {

            VertexFormat vf;
            vf.setInputRate(getVertexInputRate(binding_node.attribute("input_rate").as_string()));
            vf.setBindingNum(binding_node.attribute("num").as_int());
            vf.setVertexBufferBindingName(binding_node.attribute("vertex_buffer_bind_name").as_string());
            vf.setIndexBufferBindingName(binding_node.attribute("index_buffer_bind_name").as_string());
            vf.setVertexBufferResourceType(binding_node.attribute("vertex_buffer_resource_type").as_string());
            vf.setIndexBufferResourceType(binding_node.attribute("index_buffer_resource_type").as_string());
            vf.setIndexType(getIndexType(binding_node.attribute("index_type").as_string()));

            for (pugi::xml_node attribute_node = binding_node.first_child(); attribute_node; attribute_node = attribute_node.next_sibling()) {
                int location = attribute_node.child("Location").text().as_int(0);
                VertexAttributeGLSLFormat attr_glsl_format = getInputAttributeGLSLFormat(attribute_node.child("GLSLFormat").text().as_string());
                VkFormat attr_format = getFormat(attribute_node.child("InternalFormat").text().as_string());
                int num = attribute_node.child("Semantic").attribute("num").as_int(0);
                VertexAttributeSemantic semantic = getVertexAttributeSemantic(attribute_node.child("Semantic").text().as_string());
                std::string name = attribute_node.attribute("name").as_string();
                vf.setVertexAttribute({semantic, num}, attr_glsl_format, attr_format, location);
            }
            m_input_attributes.push_back(std::move(vf));
		}
    }

    pugi::xml_node descriptor_set_node = shader_data.child("DescriptorSet");
	if (descriptor_set_node) {
        for (pugi::xml_node set_node = descriptor_set_node.first_child(); set_node; set_node = set_node.next_sibling()) {
            uint32_t slot = set_node.attribute("slot").as_uint();
            m_desc_set_names[slot] = set_node.text().as_string();
        }
    }

    pugi::xml_node push_constants_node = shader_data.child("PushConstants");
	if (push_constants_node) {
        uint32_t offset = 0u;
        for (pugi::xml_node constant_node = push_constants_node.first_child(); constant_node; constant_node = constant_node.next_sibling()) {
            VkFormat vk_format = getFormat(constant_node.child("InternalFormat").text().as_string());
            size_t bytes_for_type = VulkanDevice::getBytesCount(vk_format);
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

            VkFormat vk_format = getFormat(constant_node.child("InternalFormat").text().as_string());
            size_t bytes_for_type = VulkanDevice::getBytesCount(vk_format);
            map_entry.size = bytes_for_type;

            offset += bytes_for_type;
            
			m_specialization_constants.push_back(map_entry);

            std::vector<char> data = getDataFromXmlValue(constant_node.child("Value"), vk_format);
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

const VertexFormat& ShaderSignature::getVertexFormat() const {
    return m_input_attributes.front();
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

const VertexFormat& ShaderSignature::getInputAttributes(size_t binding) const {
    return m_input_attributes.at(binding);
}

const std::vector<VertexFormat>& ShaderSignature::getInputAttributes() const {
    return m_input_attributes;
}

size_t ShaderSignature::getNumInputAttributeBindings() const {
    return m_input_attributes.size();
}

const std::unordered_map<ShaderSignature::SlotNumber, std::string>& ShaderSignature::getDescSetNames() const {
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