#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class VertexFormat {
public:
    using SemanticName = std::string;
    using SemanticNumber = size_t;

    enum class VertexAttributeType : int32_t {
	    POSITION = 0,
	    NORMAL = 1,
	    TANGENT = 2,
	    BINORMAL = 3,
	    TEXCOORD = 4,
        COLOR = 20,
        JOINTS = 21,
	    WEIGHTS = 22
    };

    enum class VertexAttributeFormat : int32_t {
        FLOAT = 0,
        FLOAT_VEC2 = 1,
        FLOAT_VEC3 = 2,
        FLOAT_VEC4 = 3,
        INT = 4,
        INT_VEC2 = 5,
        INT_VEC3 = 6,
        INT_VEC4 = 7
    };

    void addVertexAttribute(const SemanticName& name, VertexAttributeFormat format);

    size_t getVertexAttribPos(const SemanticName& name) const;
    const SemanticName& getPosSemanticName(size_t pos) const;

    VertexAttributeFormat getAttribFormat (size_t pos) const;
    VertexAttributeFormat getAttribFormat (const SemanticName& name) const;

    size_t getStride(const SemanticName& name) const;
    size_t getStride(size_t pos) const;

    template<typename ElementType>
    size_t getOffset(const SemanticName& name) const {
        size_t result = -1;

        return result;
    };

    size_t getVertexAttribCount() const;

    size_t getVertexSize() const;

    template<typename ElementType>
    size_t getElementsCount() const {
        size_t result = -1;

        return result;
    }

private:
    static std::pair<VertexAttributeType, SemanticNumber> getSemanticParams(const SemanticName& name);

    std::vector<SemanticName> m_names_pos;
    std::vector<VertexAttributeFormat> m_format_pos;
    std::unordered_map<SemanticName, size_t> m_name_pos_map;
    std::unordered_map<SemanticName, SemanticNumber> m_semantic_name_number;
    std::unordered_map<SemanticName, VertexAttributeType> m_semantic_attrib_type;
};