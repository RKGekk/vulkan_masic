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

VkShaderCreateFlagBitsEXT getShaderCreateFlagEXT(const std::string& flag_str) {
    using namespace std::literals;
    VkShaderCreateFlagBitsEXT res{};
         if(flag_str == "link_stage_ext"s) { res = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT; }
    else if(flag_str == "allow_varying_subgroup_size"s) { res = VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT; }
    else if(flag_str == "require_full_subgroups"s) { res = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT; }
    else if(flag_str == "no_task_shader_ext"s) { res = VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT; }
    else if(flag_str == "dispatch_base_ext"s) { res = VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT; }
    else if(flag_str == "fragment_shading_rate_attachment_ext"s) { res = VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT; }
    else if(flag_str == "fragment_density_map_attachment_ext"s) { res = VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT; }
    else if(flag_str == "indirect_bindable_ext"s) { res = VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT; }
    return res;
}

VkPipelineShaderStageCreateFlagBits getPipelineShaderStageCreateFlag(const std::string& flag_str) {
    using namespace std::literals;

    VkPipelineShaderStageCreateFlagBits res{};
         if(flag_str == "allow_varying_subgroup_size"s) { res = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT; }
    else if(flag_str == "require_full_subgroups"s) { res = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT; }
    else if(flag_str == "allow_varying_subgroup_size_ext"s) { res = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT; }
    else if(flag_str == "require_full_subgroups_ext"s) { res = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT; }
    
    return res;
}

VkShaderStageFlagBits getShaderStageFlag(const std::string& stage_str) {
    using namespace std::literals;
    VkShaderStageFlagBits res{};
         if(stage_str == "vertex"s) {res = VK_SHADER_STAGE_VERTEX_BIT;}
    else if(stage_str == "tessellation_control"s) {res = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;}
    else if(stage_str == "tessellation_evaluation"s) {res = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;}
    else if(stage_str == "geometry"s) {res = VK_SHADER_STAGE_GEOMETRY_BIT;}
    else if(stage_str == "fragment"s) {res = VK_SHADER_STAGE_FRAGMENT_BIT;}
    else if(stage_str == "compute"s) {res = VK_SHADER_STAGE_COMPUTE_BIT;}
    else if(stage_str == "all_graphics"s) {res = VK_SHADER_STAGE_ALL_GRAPHICS;}
    else if(stage_str == "all"s) {res = VK_SHADER_STAGE_ALL;}
    else if(stage_str == "raygen_khr"s) {res = VK_SHADER_STAGE_RAYGEN_BIT_KHR;}
    else if(stage_str == "any_hit_khr"s) {res = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;}
    else if(stage_str == "closest_hit_khr"s) {res = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;}
    else if(stage_str == "miss_khr"s) {res = VK_SHADER_STAGE_MISS_BIT_KHR;}
    else if(stage_str == "intersection_khr"s) {res = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;}
    else if(stage_str == "callable_khr"s) {res = VK_SHADER_STAGE_CALLABLE_BIT_KHR;}
    else if(stage_str == "task_ext"s) {res = VK_SHADER_STAGE_TASK_BIT_EXT;}
    else if(stage_str == "mesh_ext"s) {res = VK_SHADER_STAGE_MESH_BIT_EXT;}
    else if(stage_str == "subpass_shading_huawei"s) {res = VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI;}
    else if(stage_str == "cluster_culling_huawei"s) {res = VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI;}
    else if(stage_str == "raygen_nv"s) {res = VK_SHADER_STAGE_RAYGEN_BIT_NV;}
    else if(stage_str == "any_hit_nv"s) {res = VK_SHADER_STAGE_ANY_HIT_BIT_NV;}
    else if(stage_str == "closest_hit_nv"s) {res = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;}
    else if(stage_str == "miss_nv"s) {res = VK_SHADER_STAGE_MISS_BIT_NV;}
    else if(stage_str == "intersection_nv"s) {res = VK_SHADER_STAGE_INTERSECTION_BIT_NV;}
    else if(stage_str == "callable_nv"s) {res = VK_SHADER_STAGE_CALLABLE_BIT_NV;}
    else if(stage_str == "task_nv"s) {res = VK_SHADER_STAGE_TASK_BIT_NV;}
    else if(stage_str == "mesh_nv"s) {res = VK_SHADER_STAGE_MESH_BIT_NV;}
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

VkSamplerCreateFlagBits getSamplerCreateFlag(const std::string& flag_str) {
    using namespace std::literals;
    VkSamplerCreateFlagBits res{};

         if(flag_str == "subsampled_ext"s) {res = VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;}
    else if(flag_str == "subsampled_coarse_reconstruction_ext"s) {res = VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT;}
    else if(flag_str == "descriptor_buffer_capture_replay_ext"s) {res = VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;}
    else if(flag_str == "non_seamless_cube_map_ext"s) {res = VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT;}
    else if(flag_str == "image_processing_qcom"s) {res = VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM;}

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

VkCullModeFlagBits getCullModeFlag(const std::string& mode_str) {
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

VkSampleCountFlagBits getSampleCountFlag(const std::string& samples_str) {
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

VkPipelineDepthStencilStateCreateFlagBits getPipelineDepthStencilStateCreateFlag(const std::string& flag_str) {
    using namespace std::literals;
    VkPipelineDepthStencilStateCreateFlagBits res{};

         if(flag_str == "rasterization_order_attachment_depth_access_ext"s) {res = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT;}
    else if(flag_str == "rasterization_order_attachment_stencil_access_ext"s) {res = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT;}
    else if(flag_str == "rasterization_order_attachment_depth_access_arm"s) {res = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;}
    else if(flag_str == "rasterization_order_attachment_stencil_access_arm"s) {res = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM;}
    
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

VkPipelineColorBlendStateCreateFlagBits getPipelineColorBlendStateCreateFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkPipelineColorBlendStateCreateFlagBits res{};

         if(flag_str == "rasterization_order_attachment_access_bit_ext"s) {res = VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT;}
    else if(flag_str == "rasterization_order_attachment_access_bit_arm"s) {res = VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM;}
    
    return res;
}

VkLogicOp getLogicOp(const std::string& op_str) {
	using namespace std::literals;
    VkLogicOp res{};

         if(op_str == "clear"s) {res = VK_LOGIC_OP_CLEAR;}
	else if(op_str == "and"s) {res = VK_LOGIC_OP_AND;}
	else if(op_str == "and_reverse"s) {res = VK_LOGIC_OP_AND_REVERSE;}
	else if(op_str == "copy"s) {res = VK_LOGIC_OP_COPY;}
	else if(op_str == "and_inverted"s) {res = VK_LOGIC_OP_AND_INVERTED;}
	else if(op_str == "no_op"s) {res = VK_LOGIC_OP_NO_OP;}
	else if(op_str == "xor"s) {res = VK_LOGIC_OP_XOR;}
	else if(op_str == "or"s) {res = VK_LOGIC_OP_OR;}
	else if(op_str == "nor"s) {res = VK_LOGIC_OP_NOR;}
	else if(op_str == "equivalent"s) {res = VK_LOGIC_OP_EQUIVALENT;}
	else if(op_str == "invert"s) {res = VK_LOGIC_OP_INVERT;}
	else if(op_str == "or_reverse"s) {res = VK_LOGIC_OP_OR_REVERSE;}
	else if(op_str == "copy_inverted"s) {res = VK_LOGIC_OP_COPY_INVERTED;}
	else if(op_str == "or_inverted"s) {res = VK_LOGIC_OP_OR_INVERTED;}
	else if(op_str == "nand"s) {res = VK_LOGIC_OP_NAND;}
	else if(op_str == "set"s) {res = VK_LOGIC_OP_SET;}
    
    return res;
}

VkBlendFactor getBlendFactor(const std::string& fac_str) {
	using namespace std::literals;
    VkBlendFactor res{};

         if(fac_str == "zero"s) {res = VK_BLEND_FACTOR_ZERO;}
	else if(fac_str == "one"s) {res = VK_BLEND_FACTOR_ONE;}
	else if(fac_str == "src_color"s) {res = VK_BLEND_FACTOR_SRC_COLOR;}
	else if(fac_str == "one_minus_src_color"s) {res = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;}
	else if(fac_str == "dst_color"s) {res = VK_BLEND_FACTOR_DST_COLOR;}
	else if(fac_str == "one_minus_dst_color"s) {res = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;}
	else if(fac_str == "src_alpha"s) {res = VK_BLEND_FACTOR_SRC_ALPHA;}
	else if(fac_str == "one_minus_src_alpha"s) {res = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;}
	else if(fac_str == "dst_alpha"s) {res = VK_BLEND_FACTOR_DST_ALPHA;}
	else if(fac_str == "one_minus_dst_alpha"s) {res = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;}
	else if(fac_str == "constant_color"s) {res = VK_BLEND_FACTOR_CONSTANT_COLOR;}
	else if(fac_str == "one_minus_constant_color"s) {res = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;}
	else if(fac_str == "constant_alpha"s) {res = VK_BLEND_FACTOR_CONSTANT_ALPHA;}
	else if(fac_str == "one_minus_constant_alpha"s) {res = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;}
	else if(fac_str == "src_alpha_saturate"s) {res = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;}
	else if(fac_str == "src1_color"s) {res = VK_BLEND_FACTOR_SRC1_COLOR;}
	else if(fac_str == "one_minus_src1_color"s) {res = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;}
	else if(fac_str == "src1_alpha"s) {res = VK_BLEND_FACTOR_SRC1_ALPHA;}
	else if(fac_str == "one_minus_src1_alpha"s) {res = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;}
    
    return res;
}

VkBlendOp getBlendOp(const std::string& op_str) {
	using namespace std::literals;
    VkBlendOp res{};

         if(op_str == "add"s) {res = VK_BLEND_OP_ADD;}
	else if(op_str == "subtract"s) {res = VK_BLEND_OP_SUBTRACT;}
	else if(op_str == "reverse_subtract"s) {res = VK_BLEND_OP_REVERSE_SUBTRACT;}
	else if(op_str == "min"s) {res = VK_BLEND_OP_MIN;}
	else if(op_str == "max"s) {res = VK_BLEND_OP_MAX;}
	else if(op_str == "zero_ext"s) {res = VK_BLEND_OP_ZERO_EXT;}
	else if(op_str == "src_ext"s) {res = VK_BLEND_OP_SRC_EXT;}
	else if(op_str == "dst_ext"s) {res = VK_BLEND_OP_DST_EXT;}
	else if(op_str == "src_over_ext"s) {res = VK_BLEND_OP_SRC_OVER_EXT;}
	else if(op_str == "dst_over_ext"s) {res = VK_BLEND_OP_DST_OVER_EXT;}
	else if(op_str == "src_in_ext"s) {res = VK_BLEND_OP_SRC_IN_EXT;}
	else if(op_str == "dst_in_ext"s) {res = VK_BLEND_OP_DST_IN_EXT;}
	else if(op_str == "src_out_ext"s) {res = VK_BLEND_OP_SRC_OUT_EXT;}
	else if(op_str == "dst_out_ext"s) {res = VK_BLEND_OP_DST_OUT_EXT;}
	else if(op_str == "src_atop_ext"s) {res = VK_BLEND_OP_SRC_ATOP_EXT;}
	else if(op_str == "dst_atop_ext"s) {res = VK_BLEND_OP_DST_ATOP_EXT;}
	else if(op_str == "xor_ext"s) {res = VK_BLEND_OP_XOR_EXT;}
	else if(op_str == "multiply_ext"s) {res = VK_BLEND_OP_MULTIPLY_EXT;}
	else if(op_str == "screen_ext"s) {res = VK_BLEND_OP_SCREEN_EXT;}
	else if(op_str == "overlay_ext"s) {res = VK_BLEND_OP_OVERLAY_EXT;}
	else if(op_str == "darken_ext"s) {res = VK_BLEND_OP_DARKEN_EXT;}
	else if(op_str == "lighten_ext"s) {res = VK_BLEND_OP_LIGHTEN_EXT;}
	else if(op_str == "colordodge_ext"s) {res = VK_BLEND_OP_COLORDODGE_EXT;}
	else if(op_str == "colorburn_ext"s) {res = VK_BLEND_OP_COLORBURN_EXT;}
	else if(op_str == "hardlight_ext"s) {res = VK_BLEND_OP_HARDLIGHT_EXT;}
	else if(op_str == "softlight_ext"s) {res = VK_BLEND_OP_SOFTLIGHT_EXT;}
	else if(op_str == "difference_ext"s) {res = VK_BLEND_OP_DIFFERENCE_EXT;}
	else if(op_str == "exclusion_ext"s) {res = VK_BLEND_OP_EXCLUSION_EXT;}
	else if(op_str == "invert_ext"s) {res = VK_BLEND_OP_INVERT_EXT;}
	else if(op_str == "invert_rgb_ext"s) {res = VK_BLEND_OP_INVERT_RGB_EXT;}
	else if(op_str == "lineardodge_ext"s) {res = VK_BLEND_OP_LINEARDODGE_EXT;}
	else if(op_str == "linearburn_ext"s) {res = VK_BLEND_OP_LINEARBURN_EXT;}
	else if(op_str == "vividlight_ext"s) {res = VK_BLEND_OP_VIVIDLIGHT_EXT;}
	else if(op_str == "linearlight_ext"s) {res = VK_BLEND_OP_LINEARLIGHT_EXT;}
	else if(op_str == "pinlight_ext"s) {res = VK_BLEND_OP_PINLIGHT_EXT;}
	else if(op_str == "hardmix_ext"s) {res = VK_BLEND_OP_HARDMIX_EXT;}
	else if(op_str == "hsl_hue_ext"s) {res = VK_BLEND_OP_HSL_HUE_EXT;}
	else if(op_str == "hsl_saturation_ext"s) {res = VK_BLEND_OP_HSL_SATURATION_EXT;}
	else if(op_str == "hsl_color_ext"s) {res = VK_BLEND_OP_HSL_COLOR_EXT;}
	else if(op_str == "hsl_luminosity_ext"s) {res = VK_BLEND_OP_HSL_LUMINOSITY_EXT;}
	else if(op_str == "plus_ext"s) {res = VK_BLEND_OP_PLUS_EXT;}
	else if(op_str == "plus_clamped_ext"s) {res = VK_BLEND_OP_PLUS_CLAMPED_EXT;}
	else if(op_str == "plus_clamped_alpha_ext"s) {res = VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT;}
	else if(op_str == "plus_darker_ext"s) {res = VK_BLEND_OP_PLUS_DARKER_EXT;}
	else if(op_str == "minus_ext"s) {res = VK_BLEND_OP_MINUS_EXT;}
	else if(op_str == "minus_clamped_ext"s) {res = VK_BLEND_OP_MINUS_CLAMPED_EXT;}
	else if(op_str == "contrast_ext"s) {res = VK_BLEND_OP_CONTRAST_EXT;}
	else if(op_str == "invert_ovg_ext"s) {res = VK_BLEND_OP_INVERT_OVG_EXT;}
	else if(op_str == "red_ext"s) {res = VK_BLEND_OP_RED_EXT;}
	else if(op_str == "green_ext"s) {res = VK_BLEND_OP_GREEN_EXT;}
	else if(op_str == "blue_ext"s) {res = VK_BLEND_OP_BLUE_EXT;}
    
    return res;
}

VkColorComponentFlagBits getColorComponentFlag(const std::string& mask_str) {
	using namespace std::literals;
    VkColorComponentFlagBits res{};

         if(mask_str == "r_bit"s) {res = VK_COLOR_COMPONENT_R_BIT;}
	else if(mask_str == "g_bit"s) {res = VK_COLOR_COMPONENT_G_BIT;}
	else if(mask_str == "b_bit"s) {res = VK_COLOR_COMPONENT_B_BIT;}
	else if(mask_str == "a_bit"s) {res = VK_COLOR_COMPONENT_A_BIT;}
    
    return res;
}

VkDynamicState getDynamicState(const std::string& dynamic_str) {
	using namespace std::literals;
    VkDynamicState res{};

         if(dynamic_str == "viewport"s) {res = VK_DYNAMIC_STATE_VIEWPORT;}
	else if(dynamic_str == "scissor"s) {res = VK_DYNAMIC_STATE_SCISSOR;}
	else if(dynamic_str == "line_width"s) {res = VK_DYNAMIC_STATE_LINE_WIDTH;}
	else if(dynamic_str == "depth_bias"s) {res = VK_DYNAMIC_STATE_DEPTH_BIAS;}
	else if(dynamic_str == "blend_constants"s) {res = VK_DYNAMIC_STATE_BLEND_CONSTANTS;}
	else if(dynamic_str == "depth_bounds"s) {res = VK_DYNAMIC_STATE_DEPTH_BOUNDS;}
	else if(dynamic_str == "stencil_compare_mask"s) {res = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;}
	else if(dynamic_str == "stencil_write_mask"s) {res = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;}
	else if(dynamic_str == "stencil_reference"s) {res = VK_DYNAMIC_STATE_STENCIL_REFERENCE;}
	else if(dynamic_str == "cull_mode"s) {res = VK_DYNAMIC_STATE_CULL_MODE;}
	else if(dynamic_str == "front_face"s) {res = VK_DYNAMIC_STATE_FRONT_FACE;}
	else if(dynamic_str == "primitive_topology"s) {res = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;}
	else if(dynamic_str == "viewport_with_count"s) {res = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT;}
	else if(dynamic_str == "scissor_with_count"s) {res = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT;}
	else if(dynamic_str == "vertex_input_binding_stride"s) {res = VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE;}
	else if(dynamic_str == "depth_test_enable"s) {res = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE;}
	else if(dynamic_str == "depth_write_enable"s) {res = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;}
	else if(dynamic_str == "depth_compare_op"s) {res = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP;}
	else if(dynamic_str == "depth_bounds_test_enable"s) {res = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE;}
	else if(dynamic_str == "stencil_test_enable"s) {res = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE;}
	else if(dynamic_str == "stencil_op"s) {res = VK_DYNAMIC_STATE_STENCIL_OP;}
	else if(dynamic_str == "rasterizer_discard_enable"s) {res = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE;}
	else if(dynamic_str == "depth_bias_enable"s) {res = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;}
	else if(dynamic_str == "primitive_restart_enable"s) {res = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;}
	else if(dynamic_str == "viewport_w_scaling_nv"s) {res = VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV;}
	else if(dynamic_str == "discard_rectangle_ext"s) {res = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT;}
	else if(dynamic_str == "discard_rectangle_enable_ext"s) {res = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT;}
	else if(dynamic_str == "discard_rectangle_mode_ext"s) {res = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT;}
	else if(dynamic_str == "sample_locations_ext"s) {res = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;}
	else if(dynamic_str == "ray_tracing_pipeline_stack_size_khr"s) {res = VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR;}
	else if(dynamic_str == "viewport_shading_rate_palette_nv"s) {res = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;}
	else if(dynamic_str == "viewport_coarse_sample_order_nv"s) {res = VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV;}
	else if(dynamic_str == "exclusive_scissor_enable_nv"s) {res = VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV;}
	else if(dynamic_str == "exclusive_scissor_nv"s) {res = VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV;}
	else if(dynamic_str == "fragment_shading_rate_khr"s) {res = VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR;}
	else if(dynamic_str == "vertex_input_ext"s) {res = VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;}
	else if(dynamic_str == "patch_control_points_ext"s) {res = VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT;}
	else if(dynamic_str == "logic_op_ext"s) {res = VK_DYNAMIC_STATE_LOGIC_OP_EXT;}
	else if(dynamic_str == "color_write_enable_ext"s) {res = VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT;}
	else if(dynamic_str == "depth_clamp_enable_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT;}
	else if(dynamic_str == "polygon_mode_ext"s) {res = VK_DYNAMIC_STATE_POLYGON_MODE_EXT;}
	else if(dynamic_str == "rasterization_samples_ext"s) {res = VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT;}
	else if(dynamic_str == "sample_mask_ext"s) {res = VK_DYNAMIC_STATE_SAMPLE_MASK_EXT;}
	else if(dynamic_str == "alpha_to_coverage_enable_ext"s) {res = VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT;}
	else if(dynamic_str == "alpha_to_one_enable_ext"s) {res = VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT;}
	else if(dynamic_str == "logic_op_enable_ext"s) {res = VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT;}
	else if(dynamic_str == "color_blend_enable_ext"s) {res = VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT;}
	else if(dynamic_str == "color_blend_equation_ext"s) {res = VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;}
	else if(dynamic_str == "color_write_mask_ext"s) {res = VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT;}
	else if(dynamic_str == "tessellation_domain_origin_ext"s) {res = VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT;}
	else if(dynamic_str == "rasterization_stream_ext"s) {res = VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT;}
	else if(dynamic_str == "conservative_rasterization_mode_ext"s) {res = VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT;}
	else if(dynamic_str == "extra_primitive_overestimation_size_ext"s) {res = VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT;}
	else if(dynamic_str == "depth_clip_enable_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT;}
	else if(dynamic_str == "sample_locations_enable_ext"s) {res = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT;}
	else if(dynamic_str == "color_blend_advanced_ext"s) {res = VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT;}
	else if(dynamic_str == "provoking_vertex_mode_ext"s) {res = VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT;}
	else if(dynamic_str == "line_rasterization_mode_ext"s) {res = VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT;}
	else if(dynamic_str == "line_stipple_enable_ext"s) {res = VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT;}
	else if(dynamic_str == "depth_clip_negative_one_to_one_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT;}
	else if(dynamic_str == "viewport_w_scaling_enable_nv"s) {res = VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV;}
	else if(dynamic_str == "viewport_swizzle_nv"s) {res = VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV;}
	else if(dynamic_str == "coverage_to_color_enable_nv"s) {res = VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV;}
	else if(dynamic_str == "coverage_to_color_location_nv"s) {res = VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV;}
	else if(dynamic_str == "coverage_modulation_mode_nv"s) {res = VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV;}
	else if(dynamic_str == "coverage_modulation_table_enable_nv"s) {res = VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV;}
	else if(dynamic_str == "coverage_modulation_table_nv"s) {res = VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV;}
	else if(dynamic_str == "shading_rate_image_enable_nv"s) {res = VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV;}
	else if(dynamic_str == "representative_fragment_test_enable_nv"s) {res = VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV;}
	else if(dynamic_str == "coverage_reduction_mode_nv"s) {res = VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV;}
	else if(dynamic_str == "attachment_feedback_loop_enable_ext"s) {res = VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT;}
	else if(dynamic_str == "line_stipple_khr"s) {res = VK_DYNAMIC_STATE_LINE_STIPPLE_KHR;}
	else if(dynamic_str == "depth_clamp_range_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT;}
	else if(dynamic_str == "line_stipple_ext"s) {res = VK_DYNAMIC_STATE_LINE_STIPPLE_EXT;}
	else if(dynamic_str == "cull_mode_ext"s) {res = VK_DYNAMIC_STATE_CULL_MODE_EXT;}
	else if(dynamic_str == "front_face_ext"s) {res = VK_DYNAMIC_STATE_FRONT_FACE_EXT;}
	else if(dynamic_str == "primitive_topology_ext"s) {res = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT;}
	else if(dynamic_str == "viewport_with_count_ext"s) {res = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT;}
	else if(dynamic_str == "scissor_with_count_ext"s) {res = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT;}
	else if(dynamic_str == "vertex_input_binding_stride_ext"s) {res = VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT;}
	else if(dynamic_str == "depth_test_enable_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT;}
	else if(dynamic_str == "depth_write_enable_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT;}
	else if(dynamic_str == "depth_compare_op_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT;}
	else if(dynamic_str == "depth_bounds_test_enable_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT;}
	else if(dynamic_str == "stencil_test_enable_ext"s) {res = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT;}
	else if(dynamic_str == "stencil_op_ext"s) {res = VK_DYNAMIC_STATE_STENCIL_OP_EXT;}
	else if(dynamic_str == "rasterizer_discard_enable_ext"s) {res = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT;}
	else if(dynamic_str == "depth_bias_enable_ext"s) {res = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT;}
	else if(dynamic_str == "primitive_restart_enable_ext"s) {res = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT;}
    
    return res;
}

VkDescriptorSetLayoutCreateFlagBits getDescriptorSetLayoutCreateFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkDescriptorSetLayoutCreateFlagBits res{};

         if(flag_str == "update_after_bind_pool"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;}
	else if(flag_str == "push_descriptor_khr"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;}
	else if(flag_str == "descriptor_buffer_ext"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;}
	else if(flag_str == "embedded_immutable_samplers_ext"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;}
	else if(flag_str == "indirect_bindable_nv"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_INDIRECT_BINDABLE_BIT_NV;}
	else if(flag_str == "host_only_pool_ext"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT;}
	else if(flag_str == "per_stage_nv"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV;}
	else if(flag_str == "update_after_bind_pool_ext"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;}
	else if(flag_str == "host_only_pool_valve"s) {res = VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE;}
    
    return res;
}

VkPipelineCreateFlagBits getPipelineCreateFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkPipelineCreateFlagBits res{};

         if(flag_str == "disable_optimization"s) {res = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;}
	else if(flag_str == "allow_derivatives"s) {res = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;}
	else if(flag_str == "derivative"s) {res = VK_PIPELINE_CREATE_DERIVATIVE_BIT;}
	else if(flag_str == "view_index_from_device_index"s) {res = VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT;}
	else if(flag_str == "dispatch_base"s) {res = VK_PIPELINE_CREATE_DISPATCH_BASE_BIT;}
	else if(flag_str == "fail_on_pipeline_compile_required"s) {res = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;}
	else if(flag_str == "early_return_on_failure"s) {res = VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT;}
	else if(flag_str == "rendering_fragment_shading_rate_attachment_khr"s) {res = VK_PIPELINE_CREATE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;}
	else if(flag_str == "rendering_fragment_density_map_attachment_ext"s) {res = VK_PIPELINE_CREATE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT;}
	else if(flag_str == "ray_tracing_no_null_any_hit_shaders_khr"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;}
	else if(flag_str == "ray_tracing_no_null_closest_hit_shaders_khr"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;}
	else if(flag_str == "ray_tracing_no_null_miss_shaders_khr"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;}
	else if(flag_str == "ray_tracing_no_null_intersection_shaders_khr"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;}
	else if(flag_str == "ray_tracing_skip_triangles_khr"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;}
	else if(flag_str == "ray_tracing_skip_aabbs_khr"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;}
	else if(flag_str == "ray_tracing_shader_group_handle_capture_replay_khr"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;}
	else if(flag_str == "defer_compile_nv"s) {res = VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV;}
	else if(flag_str == "capture_statistics_khr"s) {res = VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;}
	else if(flag_str == "capture_internal_representations_khr"s) {res = VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;}
	else if(flag_str == "indirect_bindable_nv"s) {res = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;}
	else if(flag_str == "library_khr"s) {res = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;}
	else if(flag_str == "descriptor_buffer_ext"s) {res = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;}
	else if(flag_str == "retain_link_time_optimization_info_ext"s) {res = VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;}
	else if(flag_str == "link_time_optimization_ext"s) {res = VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT;}
	else if(flag_str == "ray_tracing_allow_motion_nv"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV;}
	else if(flag_str == "color_attachment_feedback_loop_ext"s) {res = VK_PIPELINE_CREATE_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;}
	else if(flag_str == "depth_stencil_attachment_feedback_loop_ext"s) {res = VK_PIPELINE_CREATE_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;}
	else if(flag_str == "ray_tracing_opacity_micromap_ext"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT;}
	//else if(flag_str == "ray_tracing_displacement_micromap_nv"s) {res = VK_PIPELINE_CREATE_RAY_TRACING_DISPLACEMENT_MICROMAP_BIT_NV;}
	else if(flag_str == "no_protected_access_ext"s) {res = VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT;}
	else if(flag_str == "protected_access_only_ext"s) {res = VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;}
	else if(flag_str == "dispatch_base"s) {res = VK_PIPELINE_CREATE_DISPATCH_BASE;}
	//else if(flag_str == "vk_pipeline_rasterization_state_create_fragment_shading_rate_attachment_khrisadeprecatedalias"s) {res = VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHRisadeprecatedalias;}
	else if(flag_str == "vk_pipeline_rasterization_state_create_fragment_shading_rate_attachment_khr"s) {res = VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;}
	//else if(flag_str == "vk_pipeline_rasterization_state_create_fragment_density_map_attachment_extisadeprecatedalias"s) {res = VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXTisadeprecatedalias;}
	else if(flag_str == "vk_pipeline_rasterization_state_create_fragment_density_map_attachment_ext"s) {res = VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT;}
	else if(flag_str == "view_index_from_device_index_khr"s) {res = VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT_KHR;}
	else if(flag_str == "dispatch_base_khr"s) {res = VK_PIPELINE_CREATE_DISPATCH_BASE_KHR;}
	else if(flag_str == "fail_on_pipeline_compile_required_ext"s) {res = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT;}
	else if(flag_str == "early_return_on_failure_ext"s) {res = VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT;}
    
    return res;
}

VkVertexInputRate getVertexInputRate(const std::string& rate_str) {
	using namespace std::literals;
    VkVertexInputRate res{};

         if(rate_str == "vertex"s) {res = VK_VERTEX_INPUT_RATE_VERTEX;}
	else if(rate_str == "instance"s) {res = VK_VERTEX_INPUT_RATE_INSTANCE;}
    
    return res;
}

VkFormat getAttributeFormat(VertexAttributeFormat attrib_format) {
    switch (attrib_format) {
        case VertexAttributeFormat::FLOAT : return VK_FORMAT_R32_SFLOAT;
        case VertexAttributeFormat::FLOAT_VEC2 : return VK_FORMAT_R32G32_SFLOAT;
        case VertexAttributeFormat::FLOAT_VEC3 : return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexAttributeFormat::FLOAT_VEC4 : return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexAttributeFormat::INT : return VK_FORMAT_R32_SINT;
        case VertexAttributeFormat::INT_VEC2 : return VK_FORMAT_R32G32_SINT;
        case VertexAttributeFormat::INT_VEC3 : return VK_FORMAT_R32G32B32_SINT;
        case VertexAttributeFormat::INT_VEC4 : return VK_FORMAT_R32G32B32A32_SINT;
        case VertexAttributeFormat::UINT : return VK_FORMAT_R32_UINT;
        case VertexAttributeFormat::UINT_VEC2 : return VK_FORMAT_R32G32_UINT;
        case VertexAttributeFormat::UINT_VEC3 : return VK_FORMAT_R32G32B32_UINT;
        case VertexAttributeFormat::UINT_VEC4 : return VK_FORMAT_R32G32B32A32_UINT;
        default : return VK_FORMAT_R32_SFLOAT;
    }
}

VkDescriptorPoolCreateFlagBits getDescriptorPoolCreateFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkDescriptorPoolCreateFlagBits res{};

         if(flag_str == "free_descriptor_set"s) {res = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;}
	else if(flag_str == "update_after_bind"s) {res = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;}
	else if(flag_str == "host_only_ext"s) {res = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT;}
	else if(flag_str == "allow_overallocation_sets_nv"s) {res = VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV;}
	else if(flag_str == "allow_overallocation_pools_nv"s) {res = VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_POOLS_BIT_NV;}
	else if(flag_str == "update_after_bind_ext"s) {res = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;}
	else if(flag_str == "host_only_valve"s) {res = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE;}
    
    return res;
}

VkPipelineBindPoint getPipelineBindPoint(const std::string& bp_str) {
	using namespace std::literals;
    VkPipelineBindPoint res{};

         if(bp_str == "graphics"s) {res = VK_PIPELINE_BIND_POINT_GRAPHICS;}
	else if(bp_str == "compute"s) {res = VK_PIPELINE_BIND_POINT_COMPUTE;}
	else if(bp_str == "ray_tracing_khr"s) {res = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;}
	else if(bp_str == "shading_huawei"s) {res = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI;}
	else if(bp_str == "ray_tracing_nv"s) {res = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV;}
    
    return res;
}

VkSubpassDescriptionFlagBits getSubpassDescriptionFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkSubpassDescriptionFlagBits res{};

         if(flag_str == "per_view_attributes_nvx"s) {res = VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX;}
	else if(flag_str == "per_view_position_x_only_nvx"s) {res = VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX;}
	else if(flag_str == "fragment_region_qcom"s) {res = VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM;}
	else if(flag_str == "shader_resolve_qcom"s) {res = VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM;}
	else if(flag_str == "rasterization_order_attachment_color_access_ext"s) {res = VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_EXT;}
	else if(flag_str == "rasterization_order_attachment_depth_access_ext"s) {res = VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT;}
	else if(flag_str == "rasterization_order_attachment_stencil_access_ext"s) {res = VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT;}
	else if(flag_str == "enable_legacy_dithering_ext"s) {res = VK_SUBPASS_DESCRIPTION_ENABLE_LEGACY_DITHERING_BIT_EXT;}
	else if(flag_str == "rasterization_order_attachment_color_access_arm"s) {res = VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_ARM;}
	else if(flag_str == "rasterization_order_attachment_depth_access_arm"s) {res = VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;}
	else if(flag_str == "rasterization_order_attachment_stencil_access_arm"s) {res = VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM;}
    
    return res;
}

VkRenderPassCreateFlagBits getRenderPassCreateFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkRenderPassCreateFlagBits res{};

    if(flag_str == "transform_bit_qcom"s) {res = VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;}
    
    return res;
}

VkAttachmentDescriptionFlagBits getAttachmentDescriptionFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkAttachmentDescriptionFlagBits res{};

    if(flag_str == "may_alias"s) {res = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;}
    
    return res;
}

VkFormat getFormat(const std::string& format_str) {
	using namespace std::literals;
    VkFormat res{};

         if(format_str == "undefined"s) {res = VK_FORMAT_UNDEFINED;}
	else if(format_str == "r4g4_unorm_pack8"s) {res = VK_FORMAT_R4G4_UNORM_PACK8;}
	else if(format_str == "r4g4b4a4_unorm_pack16"s) {res = VK_FORMAT_R4G4B4A4_UNORM_PACK16;}
	else if(format_str == "b4g4r4a4_unorm_pack16"s) {res = VK_FORMAT_B4G4R4A4_UNORM_PACK16;}
	else if(format_str == "r5g6b5_unorm_pack16"s) {res = VK_FORMAT_R5G6B5_UNORM_PACK16;}
	else if(format_str == "b5g6r5_unorm_pack16"s) {res = VK_FORMAT_B5G6R5_UNORM_PACK16;}
	else if(format_str == "r5g5b5a1_unorm_pack16"s) {res = VK_FORMAT_R5G5B5A1_UNORM_PACK16;}
	else if(format_str == "b5g5r5a1_unorm_pack16"s) {res = VK_FORMAT_B5G5R5A1_UNORM_PACK16;}
	else if(format_str == "a1r5g5b5_unorm_pack16"s) {res = VK_FORMAT_A1R5G5B5_UNORM_PACK16;}
	else if(format_str == "r8_unorm"s) {res = VK_FORMAT_R8_UNORM;}
	else if(format_str == "r8_snorm"s) {res = VK_FORMAT_R8_SNORM;}
	else if(format_str == "r8_uscaled"s) {res = VK_FORMAT_R8_USCALED;}
	else if(format_str == "r8_sscaled"s) {res = VK_FORMAT_R8_SSCALED;}
	else if(format_str == "r8_uint"s) {res = VK_FORMAT_R8_UINT;}
	else if(format_str == "r8_sint"s) {res = VK_FORMAT_R8_SINT;}
	else if(format_str == "r8_srgb"s) {res = VK_FORMAT_R8_SRGB;}
	else if(format_str == "r8g8_unorm"s) {res = VK_FORMAT_R8G8_UNORM;}
	else if(format_str == "r8g8_snorm"s) {res = VK_FORMAT_R8G8_SNORM;}
	else if(format_str == "r8g8_uscaled"s) {res = VK_FORMAT_R8G8_USCALED;}
	else if(format_str == "r8g8_sscaled"s) {res = VK_FORMAT_R8G8_SSCALED;}
	else if(format_str == "r8g8_uint"s) {res = VK_FORMAT_R8G8_UINT;}
	else if(format_str == "r8g8_sint"s) {res = VK_FORMAT_R8G8_SINT;}
	else if(format_str == "r8g8_srgb"s) {res = VK_FORMAT_R8G8_SRGB;}
	else if(format_str == "r8g8b8_unorm"s) {res = VK_FORMAT_R8G8B8_UNORM;}
	else if(format_str == "r8g8b8_snorm"s) {res = VK_FORMAT_R8G8B8_SNORM;}
	else if(format_str == "r8g8b8_uscaled"s) {res = VK_FORMAT_R8G8B8_USCALED;}
	else if(format_str == "r8g8b8_sscaled"s) {res = VK_FORMAT_R8G8B8_SSCALED;}
	else if(format_str == "r8g8b8_uint"s) {res = VK_FORMAT_R8G8B8_UINT;}
	else if(format_str == "r8g8b8_sint"s) {res = VK_FORMAT_R8G8B8_SINT;}
	else if(format_str == "r8g8b8_srgb"s) {res = VK_FORMAT_R8G8B8_SRGB;}
	else if(format_str == "b8g8r8_unorm"s) {res = VK_FORMAT_B8G8R8_UNORM;}
	else if(format_str == "b8g8r8_snorm"s) {res = VK_FORMAT_B8G8R8_SNORM;}
	else if(format_str == "b8g8r8_uscaled"s) {res = VK_FORMAT_B8G8R8_USCALED;}
	else if(format_str == "b8g8r8_sscaled"s) {res = VK_FORMAT_B8G8R8_SSCALED;}
	else if(format_str == "b8g8r8_uint"s) {res = VK_FORMAT_B8G8R8_UINT;}
	else if(format_str == "b8g8r8_sint"s) {res = VK_FORMAT_B8G8R8_SINT;}
	else if(format_str == "b8g8r8_srgb"s) {res = VK_FORMAT_B8G8R8_SRGB;}
	else if(format_str == "r8g8b8a8_unorm"s) {res = VK_FORMAT_R8G8B8A8_UNORM;}
	else if(format_str == "r8g8b8a8_snorm"s) {res = VK_FORMAT_R8G8B8A8_SNORM;}
	else if(format_str == "r8g8b8a8_uscaled"s) {res = VK_FORMAT_R8G8B8A8_USCALED;}
	else if(format_str == "r8g8b8a8_sscaled"s) {res = VK_FORMAT_R8G8B8A8_SSCALED;}
	else if(format_str == "r8g8b8a8_uint"s) {res = VK_FORMAT_R8G8B8A8_UINT;}
	else if(format_str == "r8g8b8a8_sint"s) {res = VK_FORMAT_R8G8B8A8_SINT;}
	else if(format_str == "r8g8b8a8_srgb"s) {res = VK_FORMAT_R8G8B8A8_SRGB;}
	else if(format_str == "b8g8r8a8_unorm"s) {res = VK_FORMAT_B8G8R8A8_UNORM;}
	else if(format_str == "b8g8r8a8_snorm"s) {res = VK_FORMAT_B8G8R8A8_SNORM;}
	else if(format_str == "b8g8r8a8_uscaled"s) {res = VK_FORMAT_B8G8R8A8_USCALED;}
	else if(format_str == "b8g8r8a8_sscaled"s) {res = VK_FORMAT_B8G8R8A8_SSCALED;}
	else if(format_str == "b8g8r8a8_uint"s) {res = VK_FORMAT_B8G8R8A8_UINT;}
	else if(format_str == "b8g8r8a8_sint"s) {res = VK_FORMAT_B8G8R8A8_SINT;}
	else if(format_str == "b8g8r8a8_srgb"s) {res = VK_FORMAT_B8G8R8A8_SRGB;}
	else if(format_str == "a8b8g8r8_unorm_pack32"s) {res = VK_FORMAT_A8B8G8R8_UNORM_PACK32;}
	else if(format_str == "a8b8g8r8_snorm_pack32"s) {res = VK_FORMAT_A8B8G8R8_SNORM_PACK32;}
	else if(format_str == "a8b8g8r8_uscaled_pack32"s) {res = VK_FORMAT_A8B8G8R8_USCALED_PACK32;}
	else if(format_str == "a8b8g8r8_sscaled_pack32"s) {res = VK_FORMAT_A8B8G8R8_SSCALED_PACK32;}
	else if(format_str == "a8b8g8r8_uint_pack32"s) {res = VK_FORMAT_A8B8G8R8_UINT_PACK32;}
	else if(format_str == "a8b8g8r8_sint_pack32"s) {res = VK_FORMAT_A8B8G8R8_SINT_PACK32;}
	else if(format_str == "a8b8g8r8_srgb_pack32"s) {res = VK_FORMAT_A8B8G8R8_SRGB_PACK32;}
	else if(format_str == "a2r10g10b10_unorm_pack32"s) {res = VK_FORMAT_A2R10G10B10_UNORM_PACK32;}
	else if(format_str == "a2r10g10b10_snorm_pack32"s) {res = VK_FORMAT_A2R10G10B10_SNORM_PACK32;}
	else if(format_str == "a2r10g10b10_uscaled_pack32"s) {res = VK_FORMAT_A2R10G10B10_USCALED_PACK32;}
	else if(format_str == "a2r10g10b10_sscaled_pack32"s) {res = VK_FORMAT_A2R10G10B10_SSCALED_PACK32;}
	else if(format_str == "a2r10g10b10_uint_pack32"s) {res = VK_FORMAT_A2R10G10B10_UINT_PACK32;}
	else if(format_str == "a2r10g10b10_sint_pack32"s) {res = VK_FORMAT_A2R10G10B10_SINT_PACK32;}
	else if(format_str == "a2b10g10r10_unorm_pack32"s) {res = VK_FORMAT_A2B10G10R10_UNORM_PACK32;}
	else if(format_str == "a2b10g10r10_snorm_pack32"s) {res = VK_FORMAT_A2B10G10R10_SNORM_PACK32;}
	else if(format_str == "a2b10g10r10_uscaled_pack32"s) {res = VK_FORMAT_A2B10G10R10_USCALED_PACK32;}
	else if(format_str == "a2b10g10r10_sscaled_pack32"s) {res = VK_FORMAT_A2B10G10R10_SSCALED_PACK32;}
	else if(format_str == "a2b10g10r10_uint_pack32"s) {res = VK_FORMAT_A2B10G10R10_UINT_PACK32;}
	else if(format_str == "a2b10g10r10_sint_pack32"s) {res = VK_FORMAT_A2B10G10R10_SINT_PACK32;}
	else if(format_str == "r16_unorm"s) {res = VK_FORMAT_R16_UNORM;}
	else if(format_str == "r16_snorm"s) {res = VK_FORMAT_R16_SNORM;}
	else if(format_str == "r16_uscaled"s) {res = VK_FORMAT_R16_USCALED;}
	else if(format_str == "r16_sscaled"s) {res = VK_FORMAT_R16_SSCALED;}
	else if(format_str == "r16_uint"s) {res = VK_FORMAT_R16_UINT;}
	else if(format_str == "r16_sint"s) {res = VK_FORMAT_R16_SINT;}
	else if(format_str == "r16_sfloat"s) {res = VK_FORMAT_R16_SFLOAT;}
	else if(format_str == "r16g16_unorm"s) {res = VK_FORMAT_R16G16_UNORM;}
	else if(format_str == "r16g16_snorm"s) {res = VK_FORMAT_R16G16_SNORM;}
	else if(format_str == "r16g16_uscaled"s) {res = VK_FORMAT_R16G16_USCALED;}
	else if(format_str == "r16g16_sscaled"s) {res = VK_FORMAT_R16G16_SSCALED;}
	else if(format_str == "r16g16_uint"s) {res = VK_FORMAT_R16G16_UINT;}
	else if(format_str == "r16g16_sint"s) {res = VK_FORMAT_R16G16_SINT;}
	else if(format_str == "r16g16_sfloat"s) {res = VK_FORMAT_R16G16_SFLOAT;}
	else if(format_str == "r16g16b16_unorm"s) {res = VK_FORMAT_R16G16B16_UNORM;}
	else if(format_str == "r16g16b16_snorm"s) {res = VK_FORMAT_R16G16B16_SNORM;}
	else if(format_str == "r16g16b16_uscaled"s) {res = VK_FORMAT_R16G16B16_USCALED;}
	else if(format_str == "r16g16b16_sscaled"s) {res = VK_FORMAT_R16G16B16_SSCALED;}
	else if(format_str == "r16g16b16_uint"s) {res = VK_FORMAT_R16G16B16_UINT;}
	else if(format_str == "r16g16b16_sint"s) {res = VK_FORMAT_R16G16B16_SINT;}
	else if(format_str == "r16g16b16_sfloat"s) {res = VK_FORMAT_R16G16B16_SFLOAT;}
	else if(format_str == "r16g16b16a16_unorm"s) {res = VK_FORMAT_R16G16B16A16_UNORM;}
	else if(format_str == "r16g16b16a16_snorm"s) {res = VK_FORMAT_R16G16B16A16_SNORM;}
	else if(format_str == "r16g16b16a16_uscaled"s) {res = VK_FORMAT_R16G16B16A16_USCALED;}
	else if(format_str == "r16g16b16a16_sscaled"s) {res = VK_FORMAT_R16G16B16A16_SSCALED;}
	else if(format_str == "r16g16b16a16_uint"s) {res = VK_FORMAT_R16G16B16A16_UINT;}
	else if(format_str == "r16g16b16a16_sint"s) {res = VK_FORMAT_R16G16B16A16_SINT;}
	else if(format_str == "r16g16b16a16_sfloat"s) {res = VK_FORMAT_R16G16B16A16_SFLOAT;}
	else if(format_str == "r32_uint"s) {res = VK_FORMAT_R32_UINT;}
	else if(format_str == "r32_sint"s) {res = VK_FORMAT_R32_SINT;}
	else if(format_str == "r32_sfloat"s) {res = VK_FORMAT_R32_SFLOAT;}
	else if(format_str == "r32g32_uint"s) {res = VK_FORMAT_R32G32_UINT;}
	else if(format_str == "r32g32_sint"s) {res = VK_FORMAT_R32G32_SINT;}
	else if(format_str == "r32g32_sfloat"s) {res = VK_FORMAT_R32G32_SFLOAT;}
	else if(format_str == "r32g32b32_uint"s) {res = VK_FORMAT_R32G32B32_UINT;}
	else if(format_str == "r32g32b32_sint"s) {res = VK_FORMAT_R32G32B32_SINT;}
	else if(format_str == "r32g32b32_sfloat"s) {res = VK_FORMAT_R32G32B32_SFLOAT;}
	else if(format_str == "r32g32b32a32_uint"s) {res = VK_FORMAT_R32G32B32A32_UINT;}
	else if(format_str == "r32g32b32a32_sint"s) {res = VK_FORMAT_R32G32B32A32_SINT;}
	else if(format_str == "r32g32b32a32_sfloat"s) {res = VK_FORMAT_R32G32B32A32_SFLOAT;}
	else if(format_str == "r64_uint"s) {res = VK_FORMAT_R64_UINT;}
	else if(format_str == "r64_sint"s) {res = VK_FORMAT_R64_SINT;}
	else if(format_str == "r64_sfloat"s) {res = VK_FORMAT_R64_SFLOAT;}
	else if(format_str == "r64g64_uint"s) {res = VK_FORMAT_R64G64_UINT;}
	else if(format_str == "r64g64_sint"s) {res = VK_FORMAT_R64G64_SINT;}
	else if(format_str == "r64g64_sfloat"s) {res = VK_FORMAT_R64G64_SFLOAT;}
	else if(format_str == "r64g64b64_uint"s) {res = VK_FORMAT_R64G64B64_UINT;}
	else if(format_str == "r64g64b64_sint"s) {res = VK_FORMAT_R64G64B64_SINT;}
	else if(format_str == "r64g64b64_sfloat"s) {res = VK_FORMAT_R64G64B64_SFLOAT;}
	else if(format_str == "r64g64b64a64_uint"s) {res = VK_FORMAT_R64G64B64A64_UINT;}
	else if(format_str == "r64g64b64a64_sint"s) {res = VK_FORMAT_R64G64B64A64_SINT;}
	else if(format_str == "r64g64b64a64_sfloat"s) {res = VK_FORMAT_R64G64B64A64_SFLOAT;}
	else if(format_str == "b10g11r11_ufloat_pack32"s) {res = VK_FORMAT_B10G11R11_UFLOAT_PACK32;}
	else if(format_str == "e5b9g9r9_ufloat_pack32"s) {res = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;}
	else if(format_str == "d16_unorm"s) {res = VK_FORMAT_D16_UNORM;}
	else if(format_str == "x8_d24_unorm_pack32"s) {res = VK_FORMAT_X8_D24_UNORM_PACK32;}
	else if(format_str == "d32_sfloat"s) {res = VK_FORMAT_D32_SFLOAT;}
	else if(format_str == "s8_uint"s) {res = VK_FORMAT_S8_UINT;}
	else if(format_str == "d16_unorm_s8_uint"s) {res = VK_FORMAT_D16_UNORM_S8_UINT;}
	else if(format_str == "d24_unorm_s8_uint"s) {res = VK_FORMAT_D24_UNORM_S8_UINT;}
	else if(format_str == "d32_sfloat_s8_uint"s) {res = VK_FORMAT_D32_SFLOAT_S8_UINT;}
	else if(format_str == "bc1_rgb_unorm_block"s) {res = VK_FORMAT_BC1_RGB_UNORM_BLOCK;}
	else if(format_str == "bc1_rgb_srgb_block"s) {res = VK_FORMAT_BC1_RGB_SRGB_BLOCK;}
	else if(format_str == "bc1_rgba_unorm_block"s) {res = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;}
	else if(format_str == "bc1_rgba_srgb_block"s) {res = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;}
	else if(format_str == "bc2_unorm_block"s) {res = VK_FORMAT_BC2_UNORM_BLOCK;}
	else if(format_str == "bc2_srgb_block"s) {res = VK_FORMAT_BC2_SRGB_BLOCK;}
	else if(format_str == "bc3_unorm_block"s) {res = VK_FORMAT_BC3_UNORM_BLOCK;}
	else if(format_str == "bc3_srgb_block"s) {res = VK_FORMAT_BC3_SRGB_BLOCK;}
	else if(format_str == "bc4_unorm_block"s) {res = VK_FORMAT_BC4_UNORM_BLOCK;}
	else if(format_str == "bc4_snorm_block"s) {res = VK_FORMAT_BC4_SNORM_BLOCK;}
	else if(format_str == "bc5_unorm_block"s) {res = VK_FORMAT_BC5_UNORM_BLOCK;}
	else if(format_str == "bc5_snorm_block"s) {res = VK_FORMAT_BC5_SNORM_BLOCK;}
	else if(format_str == "bc6h_ufloat_block"s) {res = VK_FORMAT_BC6H_UFLOAT_BLOCK;}
	else if(format_str == "bc6h_sfloat_block"s) {res = VK_FORMAT_BC6H_SFLOAT_BLOCK;}
	else if(format_str == "bc7_unorm_block"s) {res = VK_FORMAT_BC7_UNORM_BLOCK;}
	else if(format_str == "bc7_srgb_block"s) {res = VK_FORMAT_BC7_SRGB_BLOCK;}
	else if(format_str == "etc2_r8g8b8_unorm_block"s) {res = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;}
	else if(format_str == "etc2_r8g8b8_srgb_block"s) {res = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;}
	else if(format_str == "etc2_r8g8b8a1_unorm_block"s) {res = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;}
	else if(format_str == "etc2_r8g8b8a1_srgb_block"s) {res = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;}
	else if(format_str == "etc2_r8g8b8a8_unorm_block"s) {res = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;}
	else if(format_str == "etc2_r8g8b8a8_srgb_block"s) {res = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;}
	else if(format_str == "eac_r11_unorm_block"s) {res = VK_FORMAT_EAC_R11_UNORM_BLOCK;}
	else if(format_str == "eac_r11_snorm_block"s) {res = VK_FORMAT_EAC_R11_SNORM_BLOCK;}
	else if(format_str == "eac_r11g11_unorm_block"s) {res = VK_FORMAT_EAC_R11G11_UNORM_BLOCK;}
	else if(format_str == "eac_r11g11_snorm_block"s) {res = VK_FORMAT_EAC_R11G11_SNORM_BLOCK;}
	else if(format_str == "astc_4x4_unorm_block"s) {res = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;}
	else if(format_str == "astc_4x4_srgb_block"s) {res = VK_FORMAT_ASTC_4x4_SRGB_BLOCK;}
	else if(format_str == "astc_5x4_unorm_block"s) {res = VK_FORMAT_ASTC_5x4_UNORM_BLOCK;}
	else if(format_str == "astc_5x4_srgb_block"s) {res = VK_FORMAT_ASTC_5x4_SRGB_BLOCK;}
	else if(format_str == "astc_5x5_unorm_block"s) {res = VK_FORMAT_ASTC_5x5_UNORM_BLOCK;}
	else if(format_str == "astc_5x5_srgb_block"s) {res = VK_FORMAT_ASTC_5x5_SRGB_BLOCK;}
	else if(format_str == "astc_6x5_unorm_block"s) {res = VK_FORMAT_ASTC_6x5_UNORM_BLOCK;}
	else if(format_str == "astc_6x5_srgb_block"s) {res = VK_FORMAT_ASTC_6x5_SRGB_BLOCK;}
	else if(format_str == "astc_6x6_unorm_block"s) {res = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;}
	else if(format_str == "astc_6x6_srgb_block"s) {res = VK_FORMAT_ASTC_6x6_SRGB_BLOCK;}
	else if(format_str == "astc_8x5_unorm_block"s) {res = VK_FORMAT_ASTC_8x5_UNORM_BLOCK;}
	else if(format_str == "astc_8x5_srgb_block"s) {res = VK_FORMAT_ASTC_8x5_SRGB_BLOCK;}
	else if(format_str == "astc_8x6_unorm_block"s) {res = VK_FORMAT_ASTC_8x6_UNORM_BLOCK;}
	else if(format_str == "astc_8x6_srgb_block"s) {res = VK_FORMAT_ASTC_8x6_SRGB_BLOCK;}
	else if(format_str == "astc_8x8_unorm_block"s) {res = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;}
	else if(format_str == "astc_8x8_srgb_block"s) {res = VK_FORMAT_ASTC_8x8_SRGB_BLOCK;}
	else if(format_str == "astc_10x5_unorm_block"s) {res = VK_FORMAT_ASTC_10x5_UNORM_BLOCK;}
	else if(format_str == "astc_10x5_srgb_block"s) {res = VK_FORMAT_ASTC_10x5_SRGB_BLOCK;}
	else if(format_str == "astc_10x6_unorm_block"s) {res = VK_FORMAT_ASTC_10x6_UNORM_BLOCK;}
	else if(format_str == "astc_10x6_srgb_block"s) {res = VK_FORMAT_ASTC_10x6_SRGB_BLOCK;}
	else if(format_str == "astc_10x8_unorm_block"s) {res = VK_FORMAT_ASTC_10x8_UNORM_BLOCK;}
	else if(format_str == "astc_10x8_srgb_block"s) {res = VK_FORMAT_ASTC_10x8_SRGB_BLOCK;}
	else if(format_str == "astc_10x10_unorm_block"s) {res = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;}
	else if(format_str == "astc_10x10_srgb_block"s) {res = VK_FORMAT_ASTC_10x10_SRGB_BLOCK;}
	else if(format_str == "astc_12x10_unorm_block"s) {res = VK_FORMAT_ASTC_12x10_UNORM_BLOCK;}
	else if(format_str == "astc_12x10_srgb_block"s) {res = VK_FORMAT_ASTC_12x10_SRGB_BLOCK;}
	else if(format_str == "astc_12x12_unorm_block"s) {res = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;}
	else if(format_str == "astc_12x12_srgb_block"s) {res = VK_FORMAT_ASTC_12x12_SRGB_BLOCK;}
	else if(format_str == "g8b8g8r8_422_unorm"s) {res = VK_FORMAT_G8B8G8R8_422_UNORM;}
	else if(format_str == "b8g8r8g8_422_unorm"s) {res = VK_FORMAT_B8G8R8G8_422_UNORM;}
	else if(format_str == "g8_b8_r8_3plane_420_unorm"s) {res = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;}
	else if(format_str == "g8_b8r8_2plane_420_unorm"s) {res = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;}
	else if(format_str == "g8_b8_r8_3plane_422_unorm"s) {res = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;}
	else if(format_str == "g8_b8r8_2plane_422_unorm"s) {res = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;}
	else if(format_str == "g8_b8_r8_3plane_444_unorm"s) {res = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;}
	else if(format_str == "r10x6_unorm_pack16"s) {res = VK_FORMAT_R10X6_UNORM_PACK16;}
	else if(format_str == "r10x6g10x6_unorm_2pack16"s) {res = VK_FORMAT_R10X6G10X6_UNORM_2PACK16;}
	else if(format_str == "r10x6g10x6b10x6a10x6_unorm_4pack16"s) {res = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16;}
	else if(format_str == "g10x6b10x6g10x6r10x6_422_unorm_4pack16"s) {res = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16;}
	else if(format_str == "b10x6g10x6r10x6g10x6_422_unorm_4pack16"s) {res = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16;}
	else if(format_str == "g10x6_b10x6_r10x6_3plane_420_unorm_3pack16"s) {res = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16;}
	else if(format_str == "g10x6_b10x6r10x6_2plane_420_unorm_3pack16"s) {res = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;}
	else if(format_str == "g10x6_b10x6_r10x6_3plane_422_unorm_3pack16"s) {res = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16;}
	else if(format_str == "g10x6_b10x6r10x6_2plane_422_unorm_3pack16"s) {res = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16;}
	else if(format_str == "g10x6_b10x6_r10x6_3plane_444_unorm_3pack16"s) {res = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16;}
	else if(format_str == "r12x4_unorm_pack16"s) {res = VK_FORMAT_R12X4_UNORM_PACK16;}
	else if(format_str == "r12x4g12x4_unorm_2pack16"s) {res = VK_FORMAT_R12X4G12X4_UNORM_2PACK16;}
	else if(format_str == "r12x4g12x4b12x4a12x4_unorm_4pack16"s) {res = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16;}
	else if(format_str == "g12x4b12x4g12x4r12x4_422_unorm_4pack16"s) {res = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16;}
	else if(format_str == "b12x4g12x4r12x4g12x4_422_unorm_4pack16"s) {res = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16;}
	else if(format_str == "g12x4_b12x4_r12x4_3plane_420_unorm_3pack16"s) {res = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16;}
	else if(format_str == "g12x4_b12x4r12x4_2plane_420_unorm_3pack16"s) {res = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16;}
	else if(format_str == "g12x4_b12x4_r12x4_3plane_422_unorm_3pack16"s) {res = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16;}
	else if(format_str == "g12x4_b12x4r12x4_2plane_422_unorm_3pack16"s) {res = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16;}
	else if(format_str == "g12x4_b12x4_r12x4_3plane_444_unorm_3pack16"s) {res = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16;}
	else if(format_str == "g16b16g16r16_422_unorm"s) {res = VK_FORMAT_G16B16G16R16_422_UNORM;}
	else if(format_str == "b16g16r16g16_422_unorm"s) {res = VK_FORMAT_B16G16R16G16_422_UNORM;}
	else if(format_str == "g16_b16_r16_3plane_420_unorm"s) {res = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;}
	else if(format_str == "g16_b16r16_2plane_420_unorm"s) {res = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM;}
	else if(format_str == "g16_b16_r16_3plane_422_unorm"s) {res = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM;}
	else if(format_str == "g16_b16r16_2plane_422_unorm"s) {res = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM;}
	else if(format_str == "g16_b16_r16_3plane_444_unorm"s) {res = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM;}
	else if(format_str == "g8_b8r8_2plane_444_unorm"s) {res = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM;}
	else if(format_str == "g10x6_b10x6r10x6_2plane_444_unorm_3pack16"s) {res = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16;}
	else if(format_str == "g12x4_b12x4r12x4_2plane_444_unorm_3pack16"s) {res = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16;}
	else if(format_str == "g16_b16r16_2plane_444_unorm"s) {res = VK_FORMAT_G16_B16R16_2PLANE_444_UNORM;}
	else if(format_str == "a4r4g4b4_unorm_pack16"s) {res = VK_FORMAT_A4R4G4B4_UNORM_PACK16;}
	else if(format_str == "a4b4g4r4_unorm_pack16"s) {res = VK_FORMAT_A4B4G4R4_UNORM_PACK16;}
	else if(format_str == "astc_4x4_sfloat_block"s) {res = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK;}
	else if(format_str == "astc_5x4_sfloat_block"s) {res = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK;}
	else if(format_str == "astc_5x5_sfloat_block"s) {res = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK;}
	else if(format_str == "astc_6x5_sfloat_block"s) {res = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK;}
	else if(format_str == "astc_6x6_sfloat_block"s) {res = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK;}
	else if(format_str == "astc_8x5_sfloat_block"s) {res = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK;}
	else if(format_str == "astc_8x6_sfloat_block"s) {res = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK;}
	else if(format_str == "astc_8x8_sfloat_block"s) {res = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK;}
	else if(format_str == "astc_10x5_sfloat_block"s) {res = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK;}
	else if(format_str == "astc_10x6_sfloat_block"s) {res = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK;}
	else if(format_str == "astc_10x8_sfloat_block"s) {res = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK;}
	else if(format_str == "astc_10x10_sfloat_block"s) {res = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK;}
	else if(format_str == "astc_12x10_sfloat_block"s) {res = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK;}
	else if(format_str == "astc_12x12_sfloat_block"s) {res = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK;}
	else if(format_str == "pvrtc1_2bpp_unorm_block_img"s) {res = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;}
	else if(format_str == "pvrtc1_4bpp_unorm_block_img"s) {res = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;}
	else if(format_str == "pvrtc2_2bpp_unorm_block_img"s) {res = VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;}
	else if(format_str == "pvrtc2_4bpp_unorm_block_img"s) {res = VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;}
	else if(format_str == "pvrtc1_2bpp_srgb_block_img"s) {res = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;}
	else if(format_str == "pvrtc1_4bpp_srgb_block_img"s) {res = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;}
	else if(format_str == "pvrtc2_2bpp_srgb_block_img"s) {res = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;}
	else if(format_str == "pvrtc2_4bpp_srgb_block_img"s) {res = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;}
	else if(format_str == "r16g16_sfixed5_nv"s) {res = VK_FORMAT_R16G16_SFIXED5_NV;}
	else if(format_str == "a1b5g5r5_unorm_pack16_khr"s) {res = VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR;}
	else if(format_str == "a8_unorm_khr"s) {res = VK_FORMAT_A8_UNORM_KHR;}
	else if(format_str == "astc_4x4_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_5x4_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_5x5_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_6x5_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_6x6_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_8x5_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_8x6_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_8x8_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_10x5_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_10x6_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_10x8_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_10x10_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_12x10_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT;}
	else if(format_str == "astc_12x12_sfloat_block_ext"s) {res = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT;}
	else if(format_str == "g8b8g8r8_422_unorm_khr"s) {res = VK_FORMAT_G8B8G8R8_422_UNORM_KHR;}
	else if(format_str == "b8g8r8g8_422_unorm_khr"s) {res = VK_FORMAT_B8G8R8G8_422_UNORM_KHR;}
	else if(format_str == "g8_b8_r8_3plane_420_unorm_khr"s) {res = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;}
	else if(format_str == "g8_b8r8_2plane_420_unorm_khr"s) {res = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;}
	else if(format_str == "g8_b8_r8_3plane_422_unorm_khr"s) {res = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR;}
	else if(format_str == "g8_b8r8_2plane_422_unorm_khr"s) {res = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR;}
	else if(format_str == "g8_b8_r8_3plane_444_unorm_khr"s) {res = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR;}
	else if(format_str == "r10x6_unorm_pack16_khr"s) {res = VK_FORMAT_R10X6_UNORM_PACK16_KHR;}
	else if(format_str == "r10x6g10x6_unorm_2pack16_khr"s) {res = VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR;}
	else if(format_str == "r10x6g10x6b10x6a10x6_unorm_4pack16_khr"s) {res = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR;}
	else if(format_str == "g10x6b10x6g10x6r10x6_422_unorm_4pack16_khr"s) {res = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR;}
	else if(format_str == "b10x6g10x6r10x6g10x6_422_unorm_4pack16_khr"s) {res = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR;}
	else if(format_str == "g10x6_b10x6_r10x6_3plane_420_unorm_3pack16_khr"s) {res = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR;}
	else if(format_str == "g10x6_b10x6r10x6_2plane_420_unorm_3pack16_khr"s) {res = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR;}
	else if(format_str == "g10x6_b10x6_r10x6_3plane_422_unorm_3pack16_khr"s) {res = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR;}
	else if(format_str == "g10x6_b10x6r10x6_2plane_422_unorm_3pack16_khr"s) {res = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR;}
	else if(format_str == "g10x6_b10x6_r10x6_3plane_444_unorm_3pack16_khr"s) {res = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR;}
	else if(format_str == "r12x4_unorm_pack16_khr"s) {res = VK_FORMAT_R12X4_UNORM_PACK16_KHR;}
	else if(format_str == "r12x4g12x4_unorm_2pack16_khr"s) {res = VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR;}
	else if(format_str == "r12x4g12x4b12x4a12x4_unorm_4pack16_khr"s) {res = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR;}
	else if(format_str == "g12x4b12x4g12x4r12x4_422_unorm_4pack16_khr"s) {res = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR;}
	else if(format_str == "b12x4g12x4r12x4g12x4_422_unorm_4pack16_khr"s) {res = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR;}
	else if(format_str == "g12x4_b12x4_r12x4_3plane_420_unorm_3pack16_khr"s) {res = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR;}
	else if(format_str == "g12x4_b12x4r12x4_2plane_420_unorm_3pack16_khr"s) {res = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR;}
	else if(format_str == "g12x4_b12x4_r12x4_3plane_422_unorm_3pack16_khr"s) {res = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR;}
	else if(format_str == "g12x4_b12x4r12x4_2plane_422_unorm_3pack16_khr"s) {res = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR;}
	else if(format_str == "g12x4_b12x4_r12x4_3plane_444_unorm_3pack16_khr"s) {res = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR;}
	else if(format_str == "g16b16g16r16_422_unorm_khr"s) {res = VK_FORMAT_G16B16G16R16_422_UNORM_KHR;}
	else if(format_str == "b16g16r16g16_422_unorm_khr"s) {res = VK_FORMAT_B16G16R16G16_422_UNORM_KHR;}
	else if(format_str == "g16_b16_r16_3plane_420_unorm_khr"s) {res = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR;}
	else if(format_str == "g16_b16r16_2plane_420_unorm_khr"s) {res = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR;}
	else if(format_str == "g16_b16_r16_3plane_422_unorm_khr"s) {res = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR;}
	else if(format_str == "g16_b16r16_2plane_422_unorm_khr"s) {res = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR;}
	else if(format_str == "g16_b16_r16_3plane_444_unorm_khr"s) {res = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR;}
	else if(format_str == "g8_b8r8_2plane_444_unorm_ext"s) {res = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT;}
	else if(format_str == "g10x6_b10x6r10x6_2plane_444_unorm_3pack16_ext"s) {res = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT;}
	else if(format_str == "g12x4_b12x4r12x4_2plane_444_unorm_3pack16_ext"s) {res = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT;}
	else if(format_str == "g16_b16r16_2plane_444_unorm_ext"s) {res = VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT;}
	else if(format_str == "a4r4g4b4_unorm_pack16_ext"s) {res = VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT;}
	else if(format_str == "a4b4g4r4_unorm_pack16_ext"s) {res = VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT;}
	else if(format_str == "r16g16_s10_5_nv"s) {res = VK_FORMAT_R16G16_S10_5_NV;}
    
    return res;
}

VkAttachmentLoadOp getAttachmentLoadOp(const std::string& load_op_str) {
	using namespace std::literals;
    VkAttachmentLoadOp res{};

         if(load_op_str == "load"s) {res = VK_ATTACHMENT_LOAD_OP_LOAD;}
	else if(load_op_str == "clear"s) {res = VK_ATTACHMENT_LOAD_OP_CLEAR;}
	else if(load_op_str == "dont_care"s) {res = VK_ATTACHMENT_LOAD_OP_DONT_CARE;}
	else if(load_op_str == "none_khr"s) {res = VK_ATTACHMENT_LOAD_OP_NONE_KHR;}
	else if(load_op_str == "none_ext"s) {res = VK_ATTACHMENT_LOAD_OP_NONE_EXT;}
    
    return res;
}

VkAttachmentStoreOp getAttachmentStoreOp(const std::string& store_op_str) {
	using namespace std::literals;
    VkAttachmentStoreOp res{};

         if(store_op_str == "store"s) {res = VK_ATTACHMENT_STORE_OP_STORE;}
	else if(store_op_str == "dont_care"s) {res = VK_ATTACHMENT_STORE_OP_DONT_CARE;}
	else if(store_op_str == "none"s) {res = VK_ATTACHMENT_STORE_OP_NONE;}
	else if(store_op_str == "none_khr"s) {res = VK_ATTACHMENT_STORE_OP_NONE_KHR;}
	else if(store_op_str == "none_qcom"s) {res = VK_ATTACHMENT_STORE_OP_NONE_QCOM;}
	else if(store_op_str == "none_ext"s) {res = VK_ATTACHMENT_STORE_OP_NONE_EXT;}
    
    return res;
}

VkImageLayout getImageLayout(const std::string& layout_str) {
	using namespace std::literals;
    VkImageLayout res{};

         if(layout_str == "undefined"s) {res = VK_IMAGE_LAYOUT_UNDEFINED;}
	else if(layout_str == "general"s) {res = VK_IMAGE_LAYOUT_GENERAL;}
	else if(layout_str == "color_attachment_optimal"s) {res = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;}
	else if(layout_str == "depth_stencil_attachment_optimal"s) {res = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;}
	else if(layout_str == "depth_stencil_read_only_optimal"s) {res = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;}
	else if(layout_str == "shader_read_only_optimal"s) {res = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;}
	else if(layout_str == "transfer_src_optimal"s) {res = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;}
	else if(layout_str == "transfer_dst_optimal"s) {res = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;}
	else if(layout_str == "preinitialized"s) {res = VK_IMAGE_LAYOUT_PREINITIALIZED;}
	else if(layout_str == "depth_read_only_stencil_attachment_optimal"s) {res = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;}
	else if(layout_str == "depth_attachment_stencil_read_only_optimal"s) {res = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;}
	else if(layout_str == "depth_attachment_optimal"s) {res = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;}
	else if(layout_str == "depth_read_only_optimal"s) {res = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;}
	else if(layout_str == "stencil_attachment_optimal"s) {res = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;}
	else if(layout_str == "stencil_read_only_optimal"s) {res = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;}
	else if(layout_str == "read_only_optimal"s) {res = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;}
	else if(layout_str == "attachment_optimal"s) {res = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;}
	else if(layout_str == "present_src_khr"s) {res = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;}
	else if(layout_str == "video_decode_dst_khr"s) {res = VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR;}
	else if(layout_str == "video_decode_src_khr"s) {res = VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR;}
	else if(layout_str == "video_decode_dpb_khr"s) {res = VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR;}
	else if(layout_str == "shared_present_khr"s) {res = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;}
	else if(layout_str == "fragment_density_map_optimal_ext"s) {res = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;}
	else if(layout_str == "fragment_shading_rate_attachment_optimal_khr"s) {res = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;}
	else if(layout_str == "rendering_local_read_khr"s) {res = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;}
	else if(layout_str == "video_encode_dst_khr"s) {res = VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR;}
	else if(layout_str == "video_encode_src_khr"s) {res = VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR;}
	else if(layout_str == "video_encode_dpb_khr"s) {res = VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR;}
	else if(layout_str == "attachment_feedback_loop_optimal_ext"s) {res = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;}
	else if(layout_str == "depth_read_only_stencil_attachment_optimal_khr"s) {res = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR;}
	else if(layout_str == "depth_attachment_stencil_read_only_optimal_khr"s) {res = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;}
	else if(layout_str == "shading_rate_optimal_nv"s) {res = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;}
	else if(layout_str == "depth_attachment_optimal_khr"s) {res = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;}
	else if(layout_str == "depth_read_only_optimal_khr"s) {res = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;}
	else if(layout_str == "stencil_attachment_optimal_khr"s) {res = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;}
	else if(layout_str == "stencil_read_only_optimal_khr"s) {res = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;}
	else if(layout_str == "read_only_optimal_khr"s) {res = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;}
	else if(layout_str == "attachment_optimal_khr"s) {res = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;}
    
    return res;
}

VkPipelineStageFlagBits getPipelineStageFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkPipelineStageFlagBits res{};

         if(flag_str == "top_of_pipe"s) {res = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;}
	else if(flag_str == "draw_indirect"s) {res = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;}
	else if(flag_str == "vertex_input"s) {res = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;}
	else if(flag_str == "vertex_shader"s) {res = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;}
	else if(flag_str == "tessellation_control_shader"s) {res = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;}
	else if(flag_str == "tessellation_evaluation_shader"s) {res = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;}
	else if(flag_str == "geometry_shader"s) {res = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;}
	else if(flag_str == "fragment_shader"s) {res = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;}
	else if(flag_str == "early_fragment_tests"s) {res = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;}
	else if(flag_str == "late_fragment_tests"s) {res = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;}
	else if(flag_str == "color_attachment_output"s) {res = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;}
	else if(flag_str == "compute_shader"s) {res = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;}
	else if(flag_str == "transfer"s) {res = VK_PIPELINE_STAGE_TRANSFER_BIT;}
	else if(flag_str == "bottom_of_pipe"s) {res = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;}
	else if(flag_str == "host"s) {res = VK_PIPELINE_STAGE_HOST_BIT;}
	else if(flag_str == "all_graphics"s) {res = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;}
	else if(flag_str == "all_commands"s) {res = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;}
	else if(flag_str == "none"s) {res = VK_PIPELINE_STAGE_NONE;}
	else if(flag_str == "transform_feedback_ext"s) {res = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;}
	else if(flag_str == "conditional_rendering_ext"s) {res = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;}
	else if(flag_str == "acceleration_structure_build_khr"s) {res = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;}
	else if(flag_str == "ray_tracing_shader_khr"s) {res = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;}
	else if(flag_str == "fragment_density_process_ext"s) {res = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;}
	else if(flag_str == "fragment_shading_rate_attachment_khr"s) {res = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;}
	else if(flag_str == "command_preprocess_nv"s) {res = VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV;}
	else if(flag_str == "task_shader_ext"s) {res = VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;}
	else if(flag_str == "mesh_shader_ext"s) {res = VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;}
	else if(flag_str == "shading_rate_image_nv"s) {res = VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV;}
	else if(flag_str == "ray_tracing_shader_nv"s) {res = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;}
	else if(flag_str == "acceleration_structure_build_nv"s) {res = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV;}
	else if(flag_str == "task_shader_nv"s) {res = VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;}
	else if(flag_str == "mesh_shader_nv"s) {res = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV;}
	else if(flag_str == "none_khr"s) {res = VK_PIPELINE_STAGE_NONE_KHR;}
	else if(flag_str == "command_preprocess_ext"s) {res = VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_EXT;}
    
    return res;
}

VkAccessFlagBits getVkAccessFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkAccessFlagBits res{};

         if(flag_str == "indirect_command_read_bit"s) {res = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;}
	else if(flag_str == "index_read_bit"s) {res = VK_ACCESS_INDEX_READ_BIT;}
	else if(flag_str == "vertex_attribute_read_bit"s) {res = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;}
	else if(flag_str == "uniform_read_bit"s) {res = VK_ACCESS_UNIFORM_READ_BIT;}
	else if(flag_str == "input_attachment_read_bit"s) {res = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;}
	else if(flag_str == "shader_read_bit"s) {res = VK_ACCESS_SHADER_READ_BIT;}
	else if(flag_str == "shader_write_bit"s) {res = VK_ACCESS_SHADER_WRITE_BIT;}
	else if(flag_str == "color_attachment_read_bit"s) {res = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;}
	else if(flag_str == "color_attachment_write_bit"s) {res = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;}
	else if(flag_str == "depth_stencil_attachment_read_bit"s) {res = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;}
	else if(flag_str == "depth_stencil_attachment_write_bit"s) {res = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;}
	else if(flag_str == "transfer_read_bit"s) {res = VK_ACCESS_TRANSFER_READ_BIT;}
	else if(flag_str == "transfer_write_bit"s) {res = VK_ACCESS_TRANSFER_WRITE_BIT;}
	else if(flag_str == "host_read_bit"s) {res = VK_ACCESS_HOST_READ_BIT;}
	else if(flag_str == "host_write_bit"s) {res = VK_ACCESS_HOST_WRITE_BIT;}
	else if(flag_str == "memory_read_bit"s) {res = VK_ACCESS_MEMORY_READ_BIT;}
	else if(flag_str == "memory_write_bit"s) {res = VK_ACCESS_MEMORY_WRITE_BIT;}
	else if(flag_str == "none"s) {res = VK_ACCESS_NONE;}
	else if(flag_str == "transform_feedback_write_bit_ext"s) {res = VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;}
	else if(flag_str == "transform_feedback_counter_read_bit_ext"s) {res = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT;}
	else if(flag_str == "transform_feedback_counter_write_bit_ext"s) {res = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;}
	else if(flag_str == "conditional_rendering_read_bit_ext"s) {res = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;}
	else if(flag_str == "color_attachment_read_noncoherent_bit_ext"s) {res = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;}
	else if(flag_str == "acceleration_structure_read_bit_khr"s) {res = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;}
	else if(flag_str == "acceleration_structure_write_bit_khr"s) {res = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;}
	else if(flag_str == "fragment_density_map_read_bit_ext"s) {res = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;}
	else if(flag_str == "fragment_shading_rate_attachment_read_bit_khr"s) {res = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;}
	else if(flag_str == "command_preprocess_read_bit_nv"s) {res = VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV;}
	else if(flag_str == "command_preprocess_write_bit_nv"s) {res = VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV;}
	else if(flag_str == "shading_rate_image_read_bit_nv"s) {res = VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;}
	else if(flag_str == "acceleration_structure_read_bit_nv"s) {res = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;}
	else if(flag_str == "acceleration_structure_write_bit_nv"s) {res = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;}
	else if(flag_str == "none_khr"s) {res = VK_ACCESS_NONE_KHR;}
	else if(flag_str == "command_preprocess_read_bit_ext"s) {res = VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_EXT;}
	else if(flag_str == "command_preprocess_write_bit_ext"s) {res = VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_EXT;}
    
    return res;
}

VkDependencyFlagBits getDependencyFlag(const std::string& flag_str) {
	using namespace std::literals;
    VkDependencyFlagBits res{};

         if(flag_str == "by_region"s) {res = VK_DEPENDENCY_BY_REGION_BIT;}
	else if(flag_str == "device_group"s) {res = VK_DEPENDENCY_DEVICE_GROUP_BIT;}
	else if(flag_str == "view_local"s) {res = VK_DEPENDENCY_VIEW_LOCAL_BIT;}
	else if(flag_str == "feedback_loop_ext"s) {res = VK_DEPENDENCY_FEEDBACK_LOOP_BIT_EXT;}
	else if(flag_str == "view_local_khr"s) {res = VK_DEPENDENCY_VIEW_LOCAL_BIT_KHR;}
	else if(flag_str == "device_group_khr"s) {res = VK_DEPENDENCY_DEVICE_GROUP_BIT_KHR;}
    
    return res;
}

VkSamplerCreateInfo getSamplerCreateInfo(const pugi::xml_node& sampler_node) {
	VkSamplerCreateInfo sampler_info{};
            
    pugi::xml_node create_flags_node = sampler_node.child("CreateFlags");
    for (pugi::xml_node create_flag = create_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
    	sampler_info.flags |= getSamplerCreateFlag(create_flag.text().as_string());
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

	return sampler_info;
}