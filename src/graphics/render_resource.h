#pragma once

#include <cstdint>

class RenderResource {
    enum class Type : uint32_t {
        VERTEX_BUFFER,
        INDEX_BUFFER,
        TEXTURE_BUFFER
    };
};