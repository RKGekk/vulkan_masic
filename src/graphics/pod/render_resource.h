#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

class RenderResource {
public:
    enum class Type : uint32_t {
        VERTEX_BUFFER,
        INDEX_BUFFER,
        TEXTURE_BUFFER,
        UNIFORM_BUFFER
    };

    virtual void destroy() = 0;
    virtual const std::string& getName() const = 0;
};