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
    InputFileStramGuard stream_guard(std::ifstream(file_name, std::ios::ate | std::ios::binary));
    std::ifstream& file = stream_guard.Get(); 
    if(!file.is_open()) {
        throw std::runtime_error("failed to open file: " + file_name + "\n");
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0u);
    file.read(buffer.data(), file_size);

    return buffer;
}