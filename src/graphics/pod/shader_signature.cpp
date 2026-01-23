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

bool ShaderSignature::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data) {
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

    pugi::xml_node descriptor_sets_node = shader_data.child("DescriptorSet");
	if (descriptor_sets_node) {
        for (pugi::xml_node set_node = descriptor_sets_node.first_child(); set_node; set_node = set_node.next_sibling()) {
            int slot = set_node.attribute("slot").as_int(0);
            std::vector<VkDescriptorSetLayoutBinding> bindings;
	        for (pugi::xml_node layout_binding_node = set_node.first_child(); layout_binding_node; layout_binding_node = layout_binding_node.next_sibling()) {
                VkDescriptorSetLayoutBinding layout_binding{};
                layout_binding.binding = layout_binding_node.child("Binding").text().as_int();
                layout_binding.descriptorType = getDescriptorType(layout_binding_node.child("DescriptorType").text().as_string());
                layout_binding.descriptorCount = layout_binding_node.child("DescriptorCount").text().as_int();
                layout_binding.stageFlags = m_stage;

                std::vector<VkSamplerCreateInfo> sampler_info_array;
                pugi::xml_node immutable_samplers_node = layout_binding_node.child("ImmutableSamplers");
                for (pugi::xml_node sampler_node = immutable_samplers_node.first_child(); sampler_node; sampler_node = sampler_node.next_sibling()) {
                    VkSamplerCreateInfo sampler_info{};

                    pugi::xml_node create_flags_node = sampler_node.child("CreateFlags");
	                if (!create_flags_node) continue;
                    for (pugi::xml_node create_flag = create_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	             	    sampler_info.flags |= getSamplerCreateFlagBit(create_flag.text().as_string());
	                }
                    sampler_info.magFilter = getSamplerFilter(sampler_node.child("MagFilter").text().as_string());
                    sampler_info.minFilter = getSamplerFilter(sampler_node.child("MinFilter").text().as_string());
                    sampler_info.mipmapMode = getSamplerMipmapMode(sampler_node.child("MipmapMode").text().as_string());
                    sampler_info.addressModeU = getSamplerAddressMode(sampler_node.child("AddressModeU").text().as_string());
                    sampler_info.addressModeV = getSamplerAddressMode(sampler_node.child("AddressModeV").text().as_string());
                    sampler_info.addressModeW = getSamplerAddressMode(sampler_node.child("AddressModeW").text().as_string());
                    sampler_info.mipLodBias = sampler_node.child("MipLodBias").text().as_float(0.0f);
                    sampler_info.anisotropyEnable = sampler_node.child("AnisotropyEnable").text().as_bool();
                    sampler_info.maxAnisotropy = sampler_node.child("MaxAnisotropy").text().as_float(0.0f);
                    sampler_info.compareEnable = sampler_node.child("CompareEnable").text().as_bool();
                    sampler_info.compareOp = getCompareOp(sampler_node.child("CompareOp").text().as_string());
                    sampler_info.minLod = sampler_node.child("MinLod").text().as_float(0.0f);
                    sampler_info.maxLod = sampler_node.child("MaxLod").text().as_float(0.0f);
                    sampler_info.borderColor = getSamplerBorderColor(sampler_node.child("BorderColor").text().as_string());
                    sampler_info.unnormalizedCoordinates = sampler_node.child("UnnormalizedCoordinates").text().as_bool();

                    std::shared_ptr<VulkanSampler> sampler = std::make_shared<VulkanSampler>();
                    sampler->init(device, sampler_info);
                    m_immutable_samplers_ptr.push_back(sampler->getSampler());
                    m_immutable_samplers.push_back(std::move(sampler));
                }

                layout_binding.pImmutableSamplers = m_immutable_samplers_ptr.data();

                bindings.push_back(layout_binding);
            }
            m_bindings.insert({slot, std::move(bindings)});
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

bool ShaderSignature::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path) {

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node shaders_node = root_node.child("Shaders");
	if (shaders_node) {
		for (pugi::xml_node shader_node = shaders_node.first_child(); shader_node; shader_node = shader_node.next_sibling()) {
			return init(device, shader_node);
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

const ShaderSignature::DescSetBindings& ShaderSignature::getBindings() const {
    return m_bindings;
}

const std::vector<std::shared_ptr<VulkanSampler>>& ShaderSignature::getImmutableSamplers() const {
    return m_immutable_samplers;
}

const std::vector<VkSampler>& ShaderSignature::getImmutableSamplersPtr() const {
    return m_immutable_samplers_ptr;
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

void ShaderSignature::destroy() {
    for(std::shared_ptr<VulkanSampler>& sampler_ptr : m_immutable_samplers) {
        sampler_ptr->destroy();
    }
}