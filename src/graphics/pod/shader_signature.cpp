#include "shader_signature.h"

#include "../api/vulkan_device.h"

VkShaderCreateFlagBitsEXT getShaderCreateFlagBitsEXT(const std::string& flag_str) {
    using namespace std::literals;
    VkShaderCreateFlagBitsEXT res{};
         if(flag_str == "link_stage_bit_ext"s) { res = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT; }
    else if(flag_str == "allow_varying_subgroup_size"s) { res = VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT; }
    else if(flag_str == "require_full_subgroups"s) { res = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT; }
    else if(flag_str == "no_task_shader_bit_ext"s) { res = VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT; }
    else if(flag_str == "dispatch_base_bit_ext"s) { res = VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT; }
    else if(flag_str == "fragment_shading_rate_attachment_bit_ext"s) { res = VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT; }
    else if(flag_str == "fragment_density_map_attachment_bit_ext"s) { res = VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT; }
    else if(flag_str == "indirect_bindable_bit_ext"s) { res = VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT; }
    return res;
}

VkShaderStageFlagBits getShaderStageFlagBits(const std::string& stage_str) {
    using namespace std::literals;
    VkShaderStageFlagBits res{};
         if(stage_str == "vertex_bit"s) {res = VK_SHADER_STAGE_VERTEX_BIT;}
    else if(stage_str == "tessellation_control_bit"s) {res = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;}
    else if(stage_str == "tessellation_evaluation_bit"s) {res = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;}
    else if(stage_str == "geometry_bit"s) {res = VK_SHADER_STAGE_GEOMETRY_BIT;}
    else if(stage_str == "fragment_bit"s) {res = VK_SHADER_STAGE_FRAGMENT_BIT;}
    else if(stage_str == "compute_bit"s) {res = VK_SHADER_STAGE_COMPUTE_BIT;}
    else if(stage_str == "all_graphics"s) {res = VK_SHADER_STAGE_ALL_GRAPHICS;}
    else if(stage_str == "all"s) {res = VK_SHADER_STAGE_ALL;}
    else if(stage_str == "raygen_bit_khr"s) {res = VK_SHADER_STAGE_RAYGEN_BIT_KHR;}
    else if(stage_str == "any_hit_bit_khr"s) {res = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;}
    else if(stage_str == "closest_hit_bit_khr"s) {res = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;}
    else if(stage_str == "miss_bit_khr"s) {res = VK_SHADER_STAGE_MISS_BIT_KHR;}
    else if(stage_str == "intersection_bit_khr"s) {res = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;}
    else if(stage_str == "callable_bit_khr"s) {res = VK_SHADER_STAGE_CALLABLE_BIT_KHR;}
    else if(stage_str == "task_bit_ext"s) {res = VK_SHADER_STAGE_TASK_BIT_EXT;}
    else if(stage_str == "mesh_bit_ext"s) {res = VK_SHADER_STAGE_MESH_BIT_EXT;}
    else if(stage_str == "subpass_shading_bit_huawei"s) {res = VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI;}
    else if(stage_str == "cluster_culling_bit_huawei"s) {res = VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI;}
    else if(stage_str == "raygen_bit_nv"s) {res = VK_SHADER_STAGE_RAYGEN_BIT_NV;}
    else if(stage_str == "any_hit_bit_nv"s) {res = VK_SHADER_STAGE_ANY_HIT_BIT_NV;}
    else if(stage_str == "closest_hit_bit_nv"s) {res = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;}
    else if(stage_str == "miss_bit_nv"s) {res = VK_SHADER_STAGE_MISS_BIT_NV;}
    else if(stage_str == "intersection_bit_nv"s) {res = VK_SHADER_STAGE_INTERSECTION_BIT_NV;}
    else if(stage_str == "callable_bit_nv"s) {res = VK_SHADER_STAGE_CALLABLE_BIT_NV;}
    else if(stage_str == "task_bit_nv"s) {res = VK_SHADER_STAGE_TASK_BIT_NV;}
    else if(stage_str == "mesh_bit_nv"s) {res = VK_SHADER_STAGE_MESH_BIT_NV;}
    return res;
}

VertexAttributeFormat getInputAttributeFormat(const std::string& format_str) {
    using namespace std::literals;
    VertexAttributeFormat res{};

         if(format_str == "bool"s) {res = VertexAttributeFormat::BOOL;}
    else if(format_str == "int"s) {res = VertexAttributeFormat::INT;}
    else if(format_str == "uint"s) {res = VertexAttributeFormat::UINT;}
    else if(format_str == "float"s) {res = VertexAttributeFormat::FLOAT;}
    else if(format_str == "double"s) {res = VertexAttributeFormat::DOUBLE;}
    else if(format_str == "bvec2"s) {res = VertexAttributeFormat::BOOL_VEC2;}
    else if(format_str == "bvec3"s) {res = VertexAttributeFormat::BOOL_VEC3;}
    else if(format_str == "bvec4"s) {res = VertexAttributeFormat::BOOL_VEC4;}
    else if(format_str == "ivec2"s) {res = VertexAttributeFormat::INT_VEC2;}
    else if(format_str == "ivec3"s) {res = VertexAttributeFormat::INT_VEC3;}
    else if(format_str == "ivec4"s) {res = VertexAttributeFormat::INT_VEC4;}
    else if(format_str == "uvec2"s) {res = VertexAttributeFormat::UINT_VEC2;}
    else if(format_str == "uvec3"s) {res = VertexAttributeFormat::UINT_VEC3;}
    else if(format_str == "uvec4"s) {res = VertexAttributeFormat::UINT_VEC4;}
    else if(format_str == "vec2"s) {res = VertexAttributeFormat::INT_VEC2;}
    else if(format_str == "vec3"s) {res = VertexAttributeFormat::INT_VEC3;}
    else if(format_str == "vec4"s) {res = VertexAttributeFormat::INT_VEC4;}
    else if(format_str == "dvec2"s) {res = VertexAttributeFormat::DOUBLE_VEC2;}
    else if(format_str == "dvec3"s) {res = VertexAttributeFormat::DOUBLE_VEC3;}
    else if(format_str == "dvec4"s) {res = VertexAttributeFormat::DOUBLE_VEC4;}

    return res;
}

VertexAttributeSemantic getVertexAttributeSemantic(const std::string& semantic_str) {
    using namespace std::literals;
    VertexAttributeSemantic res{};

    if(semantic_str == "POSITION"s) {res = VertexAttributeSemantic::POSITION;}
    else if(semantic_str == "NORMAL"s) {res = VertexAttributeSemantic::NORMAL;}
    else if(semantic_str == "TANGENT"s) {res = VertexAttributeSemantic::TANGENT;}
    else if(semantic_str == "BINORMAL"s) {res = VertexAttributeSemantic::BINORMAL;}
    else if(semantic_str == "TEXCOORD"s) {res = VertexAttributeSemantic::TEXCOORD;}
    else if(semantic_str == "COLOR"s) {res = VertexAttributeSemantic::COLOR;}
    else if(semantic_str == "JOINTS"s) {res = VertexAttributeSemantic::JOINTS;}
    else if(semantic_str == "WEIGHTS"s) {res = VertexAttributeSemantic::WEIGHTS;}

    return res;
}

VkDescriptorType getDescriptorType(const std::string& desc_str) {
    using namespace std::literals;
    VkDescriptorType res{};

         if(desc_str == "sampler"s) {res = VK_DESCRIPTOR_TYPE_SAMPLER;}
    else if(desc_str == "combined_image_sampler"s) {res = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;}
    else if(desc_str == "sampled_image"s) {res = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;}
    else if(desc_str == "storage_image"s) {res = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;}
    else if(desc_str == "uniform_texel_buffer"s) {res = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;}
    else if(desc_str == "storage_texel_buffer"s) {res = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;}
    else if(desc_str == "uniform_buffer"s) {res = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;}
    else if(desc_str == "storage_buffer"s) {res = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;}
    else if(desc_str == "uniform_buffer_dynamic"s) {res = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;}
    else if(desc_str == "storage_buffer_dynamic"s) {res = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;}
    else if(desc_str == "input_attachment"s) {res = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;}
    else if(desc_str == "inline_uniform_block"s) {res = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;}
    else if(desc_str == "acceleration_structure_khr"s) {res = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;}
    else if(desc_str == "acceleration_structure_nv"s) {res = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;}
    else if(desc_str == "sample_weight_image_qcom"s) {res = VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM;}
    else if(desc_str == "block_match_image_qcom"s) {res = VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM;}
    else if(desc_str == "mutable_ext"s) {res = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;}
    else if(desc_str == "inline_uniform_block_ext"s) {res = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;}
    else if(desc_str == "mutable_valve"s) {res = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;}

    return res;
}

VkSamplerCreateFlagBits getSamplerCreateFlagBit(const std::string& flag_str) {
    using namespace std::literals;
    VkSamplerCreateFlagBits res{};

         if(flag_str == "subsampled_bit_ext"s) {res = VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;}
    else if(flag_str == "subsampled_coarse_reconstruction_bit_ext"s) {res = VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT;}
    else if(flag_str == "descriptor_buffer_capture_replay_bit_ext"s) {res = VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;}
    else if(flag_str == "non_seamless_cube_map_bit_ext"s) {res = VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT;}
    else if(flag_str == "image_processing_bit_qcom"s) {res = VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM;}

    return res;
}

VkFilter getSamplerFilter(const std::string& filter_str) {
    using namespace std::literals;
    VkFilter res{};

         if(filter_str == "nearest"s) {res = VK_FILTER_NEAREST;}
    else if(filter_str == "linear"s) {res = VK_FILTER_LINEAR;}
    else if(filter_str == "cubic_ext"s) {res = VK_FILTER_CUBIC_EXT;}
    else if(filter_str == "cubic_img"s) {res = VK_FILTER_CUBIC_IMG;}

    return res;
}

VkSamplerMipmapMode getSamplerMipmapMode(const std::string& mode_str) {
    using namespace std::literals;
    VkSamplerMipmapMode res{};

         if(mode_str == "nearest"s) {res = VK_SAMPLER_MIPMAP_MODE_NEAREST;}
    else if(mode_str == "linear"s) {res = VK_SAMPLER_MIPMAP_MODE_LINEAR;}

    return res;
}

VkSamplerAddressMode getSamplerAddressMode(const std::string& address_str) {
    using namespace std::literals;
    VkSamplerAddressMode res{};

         if(address_str == "repeat"s) {res = VK_SAMPLER_ADDRESS_MODE_REPEAT;}
    else if(address_str == "mirrored_repeat"s) {res = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;}
    else if(address_str == "clamp_to_edge"s) {res = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;}
    else if(address_str == "clamp_to_border"s) {res = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;}
    else if(address_str == "mirror_clamp_to_edge"s) {res = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;}
    else if(address_str == "mirror_clamp_to_edge_khr"s) {res = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE_KHR;}

    return res;
}

VkCompareOp getCompareOp(const std::string& op_str) {
    using namespace std::literals;
    VkCompareOp res{};

         if(op_str == "never"s) {res = VK_COMPARE_OP_NEVER;}
    else if(op_str == "less"s) {res = VK_COMPARE_OP_LESS;}
    else if(op_str == "equal"s) {res = VK_COMPARE_OP_EQUAL;}
    else if(op_str == "less_or_equal"s) {res = VK_COMPARE_OP_LESS_OR_EQUAL;}
    else if(op_str == "greater"s) {res = VK_COMPARE_OP_GREATER;}
    else if(op_str == "not_equal"s) {res = VK_COMPARE_OP_NOT_EQUAL;}
    else if(op_str == "greater_or_equal"s) {res = VK_COMPARE_OP_GREATER_OR_EQUAL;}
    else if(op_str == "always"s) {res = VK_COMPARE_OP_ALWAYS;}

    return res;
}

VkBorderColor getSamplerBorderColor(const std::string& color_str) {
    using namespace std::literals;
    VkBorderColor res{};

         if(color_str == "float_transparent_black"s) {res = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;}
    else if(color_str == "int_transparent_black"s) {res = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;}
    else if(color_str == "float_opaque_black"s) {res = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;}
    else if(color_str == "int_opaque_black"s) {res = VK_BORDER_COLOR_INT_OPAQUE_BLACK;}
    else if(color_str == "float_opaque_white"s) {res = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;}
    else if(color_str == "int_opaque_white"s) {res = VK_BORDER_COLOR_INT_OPAQUE_WHITE;}
    else if(color_str == "float_custom_ext"s) {res = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;}
    else if(color_str == "int_custom_ext"s) {res = VK_BORDER_COLOR_INT_CUSTOM_EXT;}

    return res;
}

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

bool ShaderSignature::add_shader(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data) {

    m_name = shader_data.attribute("name").as_string();
	m_file_name = shader_data.child("FilePath").text().as_string();

    m_create_flags = {};
    pugi::xml_node flags_node = shader_data.child("Flags");
	if (flags_node) {
        for (pugi::xml_node flag_node = flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			m_create_flags |= getShaderCreateFlagBitsEXT(flag_node.text().as_string());
		}
    }

    pugi::xml_node stages_node = shader_data.child("Stage");
	if (stages_node) {
        m_stage = getShaderStageFlagBits(stages_node.text().as_string());
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

                m_bindings.push_back(layout_binding);
            }
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
        VkSpecializationInfo specialization_info{};
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
    }
    

    return true;
}

bool ShaderSignature::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shaders_data) {
    if (shaders_data) {
		for (pugi::xml_node shader_node = shaders_data.first_child(); shaders_data; shader_node = shader_node.next_sibling()) {
			return add_shader(device, shader_node);
		}
	}
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
		return init(device, shaders_node);
	}

    return true;
}

const VertexFormat& ShaderSignature::getVertexFormat() const {
    return m_vertex_format;
}

void ShaderSignature::destroy() {
    for(std::shared_ptr<VulkanSampler>& sampler_ptr : m_immutable_samplers) {
        sampler_ptr->destroy();
    }
}