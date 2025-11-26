#include "shader_signature.h"

const VertexFormat& ShaderSignature::getVertexFormat() const {
    return m_vertex_format;
}

void ShaderSignature::setVertexFormat(VertexFormat format) {
    m_vertex_format = std::move(format);
}