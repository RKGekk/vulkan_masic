#pragma warning(disable : 4996)

#include "string_tools.h"

std::wstring ConvertString(const std::string& string) {
    static std::locale loc("");
    auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
    return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(string);
}

std::string ConvertString(const std::wstring& wstring) {
    static std::locale loc("");
    auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
    return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(wstring);
}

std::wstring to_wstring(const std::string& s) {
    return ConvertString(s);
}

const std::wstring& to_wstring(const std::wstring& s) {
    return s;
}

std::wstring to_wstring(char c) {
    return to_wstring(std::string(1, c));
}

std::string w2s(const std::wstring& var) {
	static std::locale loc("");
	auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
	return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(var);
}

std::wstring s2w(const std::string& var) {
	static std::locale loc("");
	auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
	return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(var);
}

std::string fixedfloat(float value, int precision) {
	std::ostringstream strout;
	strout << std::fixed;
	strout << std::setprecision(precision);
	strout << value;
	std::string str = strout.str();
	size_t end = str.find_last_not_of('0') + 1;
	if (str[end - 1] == '.') {
		return str.erase(end + 1);
	}
	else {
		return str.erase(end);
	}
}

std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
	std::ios::fmtflags oldFlag = os.flags();
	os << "(" << fixedfloat(v.x, 6) << ", " << fixedfloat(v.y, 6) << ")";
	os.flags(oldFlag);
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
	std::ios::fmtflags oldFlag = os.flags();
	os << "(" << fixedfloat(v.x, 6) << ", " << fixedfloat(v.y, 6) << ", " << fixedfloat(v.z, 6) << ")";
	os.flags(oldFlag);
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
	std::ios::fmtflags oldFlag = os.flags();
	os << "(" << fixedfloat(v.x, 6) << ", " << fixedfloat(v.y, 6) << ", " << fixedfloat(v.z, 6) << ", " << fixedfloat(v.w, 6) << ")";
	os.flags(oldFlag);
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::mat4& m) {
	std::ios::fmtflags oldFlag = os.flags();
	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			os << std::setw(6) << fixedfloat(m[r][c], 6);
		}
		os << std::endl;
	}
	os.flags(oldFlag);
	return os;
}

void stoupper(std::string& s) {
	std::for_each(s.begin(), s.end(), [](char& c) { c = ::toupper(c); });
}

std::string stoupper(const std::string& s) {
	std::string res = s;
	std::for_each(res.begin(), res.end(), [](char& c) { c = ::toupper(c); });
	return res;
}

bool stobool(const std::string& s) {
	std::string us = stoupper(s);
	return (us == "YES" || us == "TRUE" || us == "1") ? true : false;
}

bool ntobool(const pugi::xml_node node_with_bool) {
	bool res = false;
	if (node_with_bool) {
		std::string s(node_with_bool.first_child().value());
		res = stobool(s);
	}
	return res;
}

bool ntobool(const pugi::xml_node node_with_bool, bool def) {
	bool res = def;
	if (node_with_bool) {
		std::string s(node_with_bool.first_child().value());
		res = stobool(s);
	}
	return res;
}

float ntofloat(const pugi::xml_node node_with_float) {
	float res = 0.0f;
	if (node_with_float) {
		std::string s(node_with_float.first_child().value());
		res = std::stof(s);
	}
	return res;
}

float ntofloat(const pugi::xml_node node_with_float, float def) {
	float res = def;
	if (node_with_float) {
		std::string s(node_with_float.first_child().value());
		res = std::stof(s);
	}
	return res;
}

int ntoint(const pugi::xml_node node_with_int) {
	int res = 0;
	if (node_with_int) {
		std::string s(node_with_int.first_child().value());
		res = std::stoi(s);
	}
	return res;
}

int ntoint(const pugi::xml_node node_with_int, int def) {
	int res = def;
	if (node_with_int) {
		std::string s(node_with_int.first_child().value());
		res = std::stoi(s);
	}
	return res;
}

float attrtofloat(const pugi::xml_attribute attr_with_float) {
	float res = 0.0f;
	if (attr_with_float) {
		std::string s(attr_with_float.value());
		res = std::stof(s);
	}
	return res;
}

float attrtofloat(const pugi::xml_attribute attr_with_float, float def) {
	float res = def;
	if (attr_with_float) {
		std::string s(attr_with_float.value());
		res = std::stof(s);
	}
	return res;
}

glm::vec3 colorfromattr3f(const pugi::xml_node& node_with_color) {
	glm::vec3 color = { 0.0f, 0.0f, 0.0f };

	if (node_with_color) {
		std::string sr = node_with_color.attribute("r").value();
		std::string sg = node_with_color.attribute("g").value();
		std::string sb = node_with_color.attribute("b").value();

		color.x = std::stof(sr);
		color.y = std::stof(sg);
		color.z = std::stof(sb);
	}

	return color;
}

glm::vec3 colorfromattr3f(const pugi::xml_node& node_with_color, const glm::vec3& def) {
	glm::vec3 color = def;

	std::string sr = node_with_color.attribute("r").value();
	std::string sg = node_with_color.attribute("g").value();
	std::string sb = node_with_color.attribute("b").value();

	color.x = sr.empty() ? def.x : std::stof(sr);
	color.y = sg.empty() ? def.y : std::stof(sg);
	color.z = sb.empty() ? def.z : std::stof(sb);

	return color;
}

glm::vec4 colorfromattr4f(const pugi::xml_node& node_with_color) {
	glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

	if (node_with_color) {
		std::string sr = node_with_color.attribute("r").value();
		std::string sg = node_with_color.attribute("g").value();
		std::string sb = node_with_color.attribute("b").value();
		std::string sa = node_with_color.attribute("a").value();

		color.x = std::stof(sr);
		color.y = std::stof(sg);
		color.z = std::stof(sb);
		color.w = std::stof(sa);
	}

	return color;
}

glm::vec4 colorfromattr4f(const pugi::xml_node& node_with_color, const glm::vec4& def) {
	glm::vec4 color = def;

	std::string sr = node_with_color.attribute("r").value();
	std::string sg = node_with_color.attribute("g").value();
	std::string sb = node_with_color.attribute("b").value();
	std::string sa = node_with_color.attribute("a").value();

	color.x = sr.empty() ? def.x : std::stof(sr);
	color.y = sg.empty() ? def.y : std::stof(sg);
	color.z = sb.empty() ? def.z : std::stof(sb);
	color.w = sa.empty() ? def.w : std::stof(sa);

	return color;
}

glm::vec3 posfromattr3f(const pugi::xml_node& node_with_pos) {
	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };

	if (node_with_pos) {
		std::string sx = node_with_pos.attribute("x").value();
		std::string sy = node_with_pos.attribute("y").value();
		std::string sz = node_with_pos.attribute("z").value();

		pos.x = std::stof(sx);
		pos.y = std::stof(sy);
		pos.z = std::stof(sz);
	}

	return pos;
}

glm::vec3 posfromattr3f(const pugi::xml_node& node_with_pos, const glm::vec3& def) {
	glm::vec3 pos = def;

	if (node_with_pos) {
		std::string sx = node_with_pos.attribute("x").value();
		std::string sy = node_with_pos.attribute("y").value();
		std::string sz = node_with_pos.attribute("z").value();

		pos.x = std::stof(sx);
		pos.y = std::stof(sy);
		pos.z = std::stof(sz);
	}

	return pos;
}

glm::vec3 anglesfromattr3f(const pugi::xml_node& node_with_angles) {
	glm::vec3 angles = { 0.0f, 0.0f, 0.0f };

	if (node_with_angles) {
		std::string sx = node_with_angles.attribute("x").value();
		std::string sy = node_with_angles.attribute("y").value();
		std::string sz = node_with_angles.attribute("z").value();

		float yaw = std::stof(sx);
		float pitch = std::stof(sy);
		float roll = std::stof(sz);

		angles.x = glm::radians(yaw);
		angles.y = glm::radians(pitch);
		angles.z = glm::radians(roll);
	}

	return angles;
}

glm::vec3 anglesfromattr3f(const pugi::xml_node& node_with_angles, const glm::vec3& def) {
	glm::vec3 angles = def;

	if (node_with_angles) {
		std::string sx = node_with_angles.attribute("x").value();
		std::string sy = node_with_angles.attribute("y").value();
		std::string sz = node_with_angles.attribute("z").value();

		float yaw = std::stof(sx);
		float pitch = std::stof(sy);
		float roll = std::stof(sz);

		angles.x = glm::radians(yaw);
		angles.y = glm::radians(pitch);
		angles.z = glm::radians(roll);
	}

	return angles;
}

std::unordered_set<std::string> getNamesUnsupported(const std::unordered_set<std::string>& available_names, const std::unordered_set<std::string>& required_names) {
    std::unordered_set<std::string> result;
    std::vector<std::string> from(available_names.cbegin(), available_names.cend());
    std::sort(from.begin(), from.end());
    std::vector<std::string> to(required_names.cbegin(), required_names.cend());
    std::sort(to.begin(), to.end());
    std::set_difference(
        from.cbegin(),
        from.cend(),
        to.cbegin(),
        to.cend(),
        std::inserter(result, result.begin())
    );
    return result;
}

std::vector<char> readFile(const std::string& file_name) {
    FileStramGuard stream_guard(std::fstream(file_name, std::ios::in | std::ios::ate | std::ios::binary));
    std::fstream& file = stream_guard.Get();
    if(!file.is_open()) {
        throw std::runtime_error("failed to open file: " + file_name + "\n");
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0u);
    file.read(buffer.data(), file_size);

    return buffer;
}

void writeFile(const std::string& file_name, size_t file_size, const void* data) {
	FileStramGuard stream_guard(std::fstream(file_name, std::ios::out | std::ios::trunc | std::ios::binary));
    std::fstream& file = stream_guard.Get();
    if(!file.is_open()) {
        throw std::runtime_error("failed to open file: " + file_name + "\n");
    }

    file.seekg(0u);
    file.write((const char*)data, file_size);
}

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

VkPipelineShaderStageCreateFlagBits getPipelineShaderStageCreateFlagBits(const std::string& flag_str) {
    using namespace std::literals;

    VkPipelineShaderStageCreateFlagBits res{};
         if(flag_str == "allow_varying_subgroup_size"s) { res = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT; }
    else if(flag_str == "require_full_subgroups"s) { res = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT; }
    else if(flag_str == "allow_varying_subgroup_size_ext"s) { res = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT; }
    else if(flag_str == "require_full_subgroups_ext"s) { res = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT; }
    
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

VkPrimitiveTopology getPrimitiveTopology(const std::string& topology_str) {
    using namespace std::literals;
    VkPrimitiveTopology res{};

         if(topology_str == "point_list"s) {res = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;}
    else if(topology_str == "line_list"s) {res = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;}
    else if(topology_str == "line_strip"s) {res = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;}
    else if(topology_str == "triangle_list"s) {res = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;}
    else if(topology_str == "triangle_strip"s) {res = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;}
    else if(topology_str == "triangle_fan"s) {res = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;}
    else if(topology_str == "line_list_with_adjacency"s) {res = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;}
    else if(topology_str == "line_strip_with_adjacency"s) {res = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;}
    else if(topology_str == "triangle_list_with_adjacency"s) {res = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;}
    else if(topology_str == "triangle_strip_with_adjacency"s) {res = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;}
    else if(topology_str == "patch_list"s) {res = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;}

    return res;
}

VkPolygonMode getPolygonMode(const std::string& mode_str) {
    using namespace std::literals;
    VkPolygonMode res{};

         if(mode_str == "fill"s) {res = VK_POLYGON_MODE_FILL;}
    else if(mode_str == "line"s) {res = VK_POLYGON_MODE_LINE;}
    else if(mode_str == "point"s) {res = VK_POLYGON_MODE_POINT;}
    else if(mode_str == "rectangle_nv"s) {res = VK_POLYGON_MODE_FILL_RECTANGLE_NV;}
    
    return res;
}

VkCullModeFlagBits getCullModeFlagBit(const std::string& mode_str) {
    using namespace std::literals;
    VkCullModeFlagBits res{};

         if(mode_str == "none"s) {res = VK_CULL_MODE_NONE;}
    else if(mode_str == "front"s) {res = VK_CULL_MODE_FRONT_BIT;}
    else if(mode_str == "back"s) {res = VK_CULL_MODE_BACK_BIT;}
    else if(mode_str == "front_and_back"s) {res = VK_CULL_MODE_FRONT_AND_BACK;}
    
    return res;
}

VkFrontFace getFrontFace(const std::string& face_str) {
    using namespace std::literals;
    VkFrontFace res{};

         if(face_str == "counter_clockwise"s) {res = VK_FRONT_FACE_COUNTER_CLOCKWISE;}
    else if(face_str == "clockwise"s) {res = VK_FRONT_FACE_CLOCKWISE;}
    
    return res;
}

VkSampleCountFlagBits getSampleCountFlagBit(const std::string& samples_str) {
    using namespace std::literals;
    VkSampleCountFlagBits res{};

         if(samples_str == "1_bit"s) {res = VK_SAMPLE_COUNT_1_BIT;}
    else if(samples_str == "2_bit"s) {res = VK_SAMPLE_COUNT_2_BIT;}
    else if(samples_str == "4_bit"s) {res = VK_SAMPLE_COUNT_4_BIT;}
    else if(samples_str == "8_bit"s) {res = VK_SAMPLE_COUNT_8_BIT;}
    else if(samples_str == "16_bit"s) {res = VK_SAMPLE_COUNT_16_BIT;}
    else if(samples_str == "32_bit"s) {res = VK_SAMPLE_COUNT_32_BIT;}
    else if(samples_str == "64_bit"s) {res = VK_SAMPLE_COUNT_64_BIT;}
    
    return res;
}

VkPipelineDepthStencilStateCreateFlagBits getPipelineDepthStencilStateCreateFlagBit(const std::string& flag_str) {
    using namespace std::literals;
    VkPipelineDepthStencilStateCreateFlagBits res{};

         if(flag_str == "rasterization_order_attachment_depth_access_bit_ext"s) {res = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT;}
    else if(flag_str == "rasterization_order_attachment_stencil_access_bit_ext"s) {res = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT;}
    else if(flag_str == "rasterization_order_attachment_depth_access_bit_arm"s) {res = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;}
    else if(flag_str == "rasterization_order_attachment_stencil_access_bit_arm"s) {res = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM;}
    
    return res;
}

VkStencilOp getStencilOp(const std::string& op_str) {
	using namespace std::literals;
    VkStencilOp res{};

         if(op_str == "keep"s) {res = VK_STENCIL_OP_KEEP;}
    else if(op_str == "zero"s) {res = VK_STENCIL_OP_ZERO;}
    else if(op_str == "replace"s) {res = VK_STENCIL_OP_REPLACE;}
    else if(op_str == "increment_and_clamp"s) {res = VK_STENCIL_OP_INCREMENT_AND_CLAMP;}
	else if(op_str == "decrement_and_clamp"s) {res = VK_STENCIL_OP_DECREMENT_AND_CLAMP;}
    else if(op_str == "invert"s) {res = VK_STENCIL_OP_INVERT;}
    else if(op_str == "increment_and_wrap"s) {res = VK_STENCIL_OP_INCREMENT_AND_WRAP;}
	else if(op_str == "decrement_and_wrap"s) {res = VK_STENCIL_OP_DECREMENT_AND_WRAP;}
    
    return res;
}