#pragma once

#include "vertex_format.h"

class ShaderSignature {
public:
    const VertexFormat& getVertexFormat() const;
    void setVertexFormat(VertexFormat format);

private:
    VertexFormat m_vertex_format;
};