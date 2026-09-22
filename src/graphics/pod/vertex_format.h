#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum class VertexAttributeSemantic : int32_t {
	POSITION = 0,
	NORMAL = 1,
	TANGENT = 2,
	BINORMAL = 3,
	TEXCOORD = 4,
    COLOR = 20,
    JOINTS = 21,
    WEIGHTS = 22,
    OTHER = 31
};

struct SemanticName {
    VertexAttributeSemantic semantic;
    int num = 0;
    bool init(const std::string& semantic_name);

    bool operator==(const SemanticName& other) const;
};

namespace std {
    template<>
    struct hash<SemanticName> {
        size_t operator()(const SemanticName& key) const {
            //return 1;
            return static_cast<int32_t>(key.semantic) ^ key.num;
        }
    };
}

enum class VertexAttributeGLSLFormat : int32_t {
    FLOAT = 0,
    FLOAT_VEC2 = 1,
    FLOAT_VEC3 = 2,
    FLOAT_VEC4 = 3,
    INT = 4,
    INT_VEC2 = 5,
    INT_VEC3 = 6,
    INT_VEC4 = 7,
    UINT = 8,
    UINT_VEC2 = 9,
    UINT_VEC3 = 10,
    UINT_VEC4 = 11,
    DOUBLE = 12, // Double-precision attributes are only available in OpenGL 4.1
    DOUBLE_VEC2 = 13,
    DOUBLE_VEC3 = 14,
    DOUBLE_VEC4 = 15,
    BOOL = 16, // vertex attributes cannot be booleans. From the specification.
    BOOL_VEC2 = 17,
    BOOL_VEC3 = 18,
    BOOL_VEC4 = 19,
    FLOAT_MAT2 = 20,
    FLOAT_MAT3 = 21,
    FLOAT_MAT4 = 24
};

class VertexFormat {
public:
    using BindingNum = size_t;
    using Location = size_t;

    static size_t getBytesForType(VkFormat format);
    static size_t GetNumComponentsInGLSLType(VertexAttributeGLSLFormat glsl_format);

    void addVertexAttribute(SemanticName semantic_name, VertexAttributeGLSLFormat glsl_format, VkFormat internal_format, std::string name);
    void setVertexAttribute(SemanticName semantic_name, VertexAttributeGLSLFormat glsl_format, VkFormat internal_format, Location location, std::string name);

    bool checkVertexAttribExist(SemanticName semantic) const;
    size_t getVertexAttribPos(SemanticName semantic) const;
    SemanticName getPosSemantic(Location pos) const;

    VertexAttributeGLSLFormat getAttribGLSLFormat (Location pos) const;
    VkFormat getAttribInternalFormat (Location pos) const;
    VertexAttributeGLSLFormat getAttribGLSLFormat (SemanticName semantic) const;
    VkFormat getAttribInternalFormat (SemanticName semantic) const;

    const std::string& getAttribName(Location pos) const;

    size_t GetNumComponentsInGLSLType(SemanticName semantic) const;
    size_t GetNumComponentsInVkType(SemanticName semantic) const;
    size_t GetComponentSizeInVkType(SemanticName semantic) const;

    size_t getOffset(SemanticName semantic) const;
    size_t getOffset(Location pos) const;

    template<typename ElementType>
    size_t getOffset(SemanticName semantic) const {
        size_t offset = -1;
        if(!m_semantic_pos_map.count(semantic)) return offset;
        offset = getOffset(semantic);
        size_t target_type_size = sizeof(ElementType);
        
        return offset / target_type_size;
    };

    size_t getVertexAttribCount() const;
    size_t getVertexSize() const;
    const std::map<Location, SemanticName>& getPosSemanticMap() const;

    VkVertexInputRate getInputRate() const;
    void setInputRate(VkVertexInputRate rate);

    BindingNum getBindingNum() const;
    void setBindingNum(BindingNum num);

    const std::string& getVertexBufferBindingName() const;
    void setVertexBufferBindingName(std::string name);

    uint32_t getVertexBufferOffset() const;
    void setVertexBufferOffset(uint32_t offset);

    const std::string& getVertexBufferResourceType() const;
    void setVertexBufferResourceType(std::string res_type);

private:
    VkVertexInputRate m_input_rate = VK_VERTEX_INPUT_RATE_VERTEX;
    BindingNum m_binding_num;

    std::string m_vertex_buffer_binding_name;
    uint32_t m_vertex_buffer_offset;
    std::string m_vertex_buffer_resource_type;

    std::unordered_map<Location, std::string> m_name_pos_map;
    std::unordered_map<Location, VertexAttributeGLSLFormat> m_glsl_format_pos_map;
    std::unordered_map<Location, VkFormat> m_internal_format_pos_map;
    std::map<Location, SemanticName> m_pos_semantic_map;
    std::unordered_map<SemanticName, Location> m_semantic_pos_map;
};