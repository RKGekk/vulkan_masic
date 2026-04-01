#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
    WEIGHTS = 22
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
            return 1;
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
};

class VertexFormat {
public:
    static size_t getBytesForType(VkFormat format);
    static size_t GetNumComponentsInGLSLType(VertexAttributeGLSLFormat glsl_format);

    void addVertexAttribute(SemanticName semantic_name, VertexAttributeGLSLFormat glsl_format, VkFormat internal_format);
    void setVertexAttribute(SemanticName semantic_name, VertexAttributeGLSLFormat glsl_format, VkFormat internal_format, int location);

    bool checkVertexAttribExist(SemanticName semantic) const;
    size_t getVertexAttribPos(SemanticName semantic) const;
    SemanticName getPosSemantic(size_t pos) const;

    VertexAttributeGLSLFormat getAttribGLSLFormat (size_t pos) const;
    VkFormat getAttribInternalFormat (size_t pos) const;
    VertexAttributeGLSLFormat getAttribGLSLFormat (SemanticName semantic) const;
    VkFormat getAttribInternalFormat (SemanticName semantic) const;

    size_t GetNumComponentsInGLSLType(SemanticName semantic) const;
    size_t GetNumComponentsInVkType(SemanticName semantic) const;

    size_t getOffset(SemanticName semantic) const;
    size_t getOffset(size_t pos) const;

    template<typename ElementType>
    size_t getOffset(SemanticName semantic) const {
        size_t offset = -1;
        if(!m_semantic_pos_map.count(semantic)) return offset;
        offset = 0u;
        size_t to = m_semantic_pos_map.at(semantic);
        for(size_t i = 0u; i < to; ++i) {
            VkFormat curr_format = m_internal_format_pos.at(i);
            size_t bytes_ct = getBytesForType(curr_format);
            size_t target_type_size = sizeof(ElementType);
            size_t size_in_target_type = bytes_ct / target_type_size;
            offset += size_in_target_type;
        }
        return offset;
    };

    size_t getVertexAttribCount() const;
    size_t getVertexSize() const;

    VkIndexType getIndexType() const;
    uint32_t getIndexTypeBytesCount() const;
    void setIndexType(VkIndexType idx_type);

    VkVertexInputRate getInputRate() const;
    void setInputRate(VkVertexInputRate rate);

    size_t getBindingNum() const;
    void setBindingNum(size_t num);

    const std::string& getVertexBufferBindingName() const;
    void setVertexBufferBindingName(std::string name);

    const std::string& getIndexBufferBindingName() const;
    void setIndexBufferBindingName(std::string name);

    const std::string& getVertexBufferResourceType() const;
    void setVertexBufferResourceType(std::string res_type);

    const std::string& getIndexBufferResourceType() const;
    void setIndexBufferResourceType(std::string res_type);

private:
    VkVertexInputRate m_input_rate = VK_VERTEX_INPUT_RATE_VERTEX;
    size_t m_binding_num;
    VkIndexType m_index_type;
    std::string m_vertex_buffer_binding_name;
    std::string m_index_buffer_binding_name;
    std::string m_vertex_buffer_resource_type;
    std::string m_index_buffer_resource_type;

    std::vector<SemanticName> m_semantic_pos;
    std::vector<VertexAttributeGLSLFormat> m_glsl_format_pos;
    std::vector<VkFormat> m_internal_format_pos;
    std::unordered_map<SemanticName, size_t> m_semantic_pos_map;
};