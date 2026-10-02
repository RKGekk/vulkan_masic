#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <string>

class VulkanDevice;

class RenderResource {
public:
    using ResourceName = std::string;
    static const std::shared_ptr<RenderResource> ROSOURCE_NULL_PTR;

    enum class Type : uint32_t {
        BUFFER,
        IMAGE,
        PUSH_CONSTANT
    };

    virtual void destroy() = 0;
    virtual const ResourceName& getName() const = 0;
    virtual Type getType() const = 0;
};