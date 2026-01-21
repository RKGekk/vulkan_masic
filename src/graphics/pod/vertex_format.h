#pragma once

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
    bool init(std::string semantic_name);

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

enum class VertexAttributeFormat : int32_t {
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
    static size_t getBytesForType(VertexAttributeFormat format);
    static size_t GetNumComponentsInType(VertexAttributeFormat format);

    void addVertexAttribute(SemanticName semantic_name, VertexAttributeFormat format);
    void setVertexAttribute(SemanticName semantic_name, VertexAttributeFormat format, int location);

    bool checkVertexAttribExist(SemanticName semantic) const;
    size_t getVertexAttribPos(SemanticName semantic) const;
    SemanticName getPosSemantic(size_t pos) const;

    VertexAttributeFormat getAttribFormat (size_t pos) const;
    VertexAttributeFormat getAttribFormat (SemanticName semantic) const;

    size_t GetNumComponentsInType(SemanticName semantic) const;

    size_t getOffset(SemanticName semantic) const;
    size_t getOffset(size_t pos) const;

    template<typename ElementType>
    size_t getOffset(SemanticName semantic) const {
        size_t offset = -1;
        if(!m_semantic_pos_map.count(semantic)) return offset;
        offset = 0u;
        size_t to = m_semantic_pos_map.at(semantic);
        for(size_t i = 0u; i < to; ++i) {
            VertexAttributeFormat curr_format = m_format_pos.at(i);
            size_t bytes_ct = getBytesForType(curr_format);
            size_t target_type_size = sizeof(ElementType);
            size_t size_in_target_type = bytes_ct / target_type_size;
            offset += size_in_target_type;
        }
        return offset;
    };

    size_t getVertexAttribCount() const;
    size_t getVertexSize() const;

private:
    std::vector<SemanticName> m_semantic_pos;
    std::vector<VertexAttributeFormat> m_format_pos;
    std::unordered_map<SemanticName, size_t> m_semantic_pos_map;
};