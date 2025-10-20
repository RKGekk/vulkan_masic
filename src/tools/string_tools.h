#pragma once

#include <algorithm>
#include <codecvt>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <locale>
#include <unordered_set>
#include <vector>

#include <pugixml.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#define STR1(x) #x
#define STR(x) STR1(x)
#define WSTR1(x) L##x
#define WSTR(x) WSTR1(x)

#pragma warning( push )
#pragma warning( disable : 4996)

std::string w2s(const std::wstring& var);
std::wstring s2w(const std::string& var);

std::wstring ConvertString(const std::string& string);
std::string ConvertString(const std::wstring& wstring);

std::wstring to_wstring(const std::string& s);
const std::wstring& to_wstring(const std::wstring& s);
std::wstring to_wstring(char c);

#pragma warning( pop )

std::string fixedfloat(float value, int precision);
std::ostream& operator<<(std::ostream& os, const glm::vec4& v);
std::ostream& operator<<(std::ostream& os, const glm::vec3& v);
std::ostream& operator<<(std::ostream& os, const glm::vec2& v);
std::ostream& operator<<(std::ostream& os, const glm::mat4& m);

template<typename C, typename T>
auto insert_in_container(C& c, T&& t) -> decltype(c.push_back(std::forward<T>(t)), void()) {
    c.push_back(std::forward<T>(t));
}
template<typename C, typename T>
auto insert_in_container(C& c, T&& t) -> decltype(c.insert(std::forward<T>(t)), void()) {
    c.insert(std::forward<T>(t));
}
template<typename Container>
Container splitR(const std::string& input, const std::string& delims) {
    Container out;
    size_t delims_len = delims.size();
    auto begIdx = 0u;
    auto endIdx = input.find(delims, begIdx);
    if (endIdx == std::string::npos && input.size() != 0u) {
        insert_in_container(out, input);
    }
    else {
        size_t w = 0;
        while (endIdx != std::string::npos) {
            w = endIdx - begIdx;
            if (w != 0) insert_in_container(out, input.substr(begIdx, w));
            begIdx = endIdx + delims_len;
            endIdx = input.find(delims, begIdx);
        }
        w = input.length() - begIdx;
        if (w != 0) insert_in_container(out, input.substr(begIdx, w));
    }
    return out;
}

void stoupper(std::string& s);
std::string stoupper(const std::string& s);

bool stobool(const std::string& s);
bool ntobool(const pugi::xml_node node_with_bool);
bool ntobool(const pugi::xml_node node_with_bool, bool def);
float ntofloat(const pugi::xml_node node_with_float);
float ntofloat(const pugi::xml_node node_with_float, float def);
int ntoint(const pugi::xml_node node_with_int);
int ntoint(const pugi::xml_node node_with_int, int def);
float attrtofloat(const pugi::xml_attribute attr_with_float);
float attrtofloat(const pugi::xml_attribute attr_with_float, float def);

glm::vec3 colorfromattr3f(const pugi::xml_node& node_with_color);
glm::vec3 colorfromattr3f(const pugi::xml_node& node_with_color, const glm::vec3& def);
glm::vec4 colorfromattr4f(const pugi::xml_node& node_with_color);
glm::vec4 colorfromattr4f(const pugi::xml_node& node_with_color, const glm::vec4& def);
glm::vec3 posfromattr3f(const pugi::xml_node& node_with_pos);
glm::vec3 posfromattr3f(const pugi::xml_node& node_with_pos, const glm::vec3& def);
glm::vec3 anglesfromattr3f(const pugi::xml_node& node_with_angles);
glm::vec3 anglesfromattr3f(const pugi::xml_node& node_with_angles, const glm::vec3& def);

template<typename Container>
void printInfo(std::string_view header, Container container) {
    std::cout << header << ": " << std::endl;
    for(const auto& ext : container) {
        std::cout << "\t - " << ext << std::endl;
    }
}

template<typename Container1, typename Container2>
bool checkNamesSupported(const Container1& available_names, const Container2& required_names) {
    bool result = false;
    result = std::all_of(required_names.cbegin(), required_names.cend(), [&available_names](const auto& req_name){ return std::count(available_names.cbegin(), available_names.cend(), req_name); });
    return result;
}

std::unordered_set<std::string> getNamesUnsupported(const std::unordered_set<std::string>& available_names, const std::unordered_set<std::string>& required_names);

class InputFileStramGuard final {
public:
    InputFileStramGuard(std::ifstream&& stream) : m_stream(std::move(stream)) {}
    ~InputFileStramGuard() {
        m_stream.close();
    }
    InputFileStramGuard(const InputFileStramGuard&) = delete;
    InputFileStramGuard& operator=(const InputFileStramGuard&) = delete;
    
    std::ifstream& Get() {
        return m_stream;
    }
private:
    std::ifstream m_stream;
};

std::vector<char> readFile(const std::string& file_name);