#include "shader_signature.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_resources_manager.h"
#include "../../tools/string_tools.h"

bool ShaderSignature::init(std::shared_ptr<VulkanResourcesManager>& resources_manager, const pugi::xml_node& shader_data) {
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
                vf.setVertexAttribute({semantic, num}, attr_glsl_format, attr_format, location, name);
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

    pugi::xml_node push_constants_node = shader_data.child("PushConstantName");
	if (push_constants_node) {
        std::string push_const_name = push_constants_node.text().as_string();
        m_push_constants = resources_manager->create_push_constant(m_name + "_pc_"s, push_const_name);
    }

    pugi::xml_node specialization_constants_node = shader_data.child("SpecializationConstants");
	if (specialization_constants_node) {
        uint32_t offset = 0u;
        for (pugi::xml_node constant_node = specialization_constants_node.first_child(); constant_node; constant_node = constant_node.next_sibling()) {
            VkSpecializationMapEntry map_entry{};

            map_entry.constantID = constant_node.child("ConstantID").text().as_int(0);
            map_entry.offset = offset;

            VkFormat vk_format = getFormat(constant_node.child("InternalFormat").text().as_string());
            VertexAttributeGLSLFormat glsl_format = getInputAttributeGLSLFormat(constant_node.child("GLSLFormat").text().as_string());
            size_t bytes_for_type = VulkanDevice::getBytesCount(vk_format);
            map_entry.size = bytes_for_type;

            offset += bytes_for_type;
			m_specialization_constants.push_back(map_entry);

            std::vector<char> data;
            if(glsl_format == VertexAttributeGLSLFormat::BOOL) {
                VkBool32 value = ntobool(constant_node.child("Value"));
                data.resize(sizeof(VkBool32));
                *(VkBool32*)(data.data()) = value;
            }

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

std::shared_ptr<VulkanPushConstant>& ShaderSignature::getPushConstants() {
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