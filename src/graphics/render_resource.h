#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>

class RenderResource {
public:
    enum class Type : uint32_t {
        VERTEX_BUFFER,
        INDEX_BUFFER,
        TEXTURE_BUFFER
    };

    virtual void destroy() = 0;
};