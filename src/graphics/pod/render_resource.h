#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

class RenderResource {
public:
    using ResourceName = std::string;

    enum class Type : uint32_t {
        VERTEX_BUFFER,
        INDEX_BUFFER,
        TEXTURE_BUFFER,
        UNIFORM_BUFFER
    };

    virtual void destroy() = 0;
    virtual const ResourceName& getName() const = 0;
    virtual Type getType() const = 0;
};