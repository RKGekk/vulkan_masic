#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>

class RenderResource {
    enum class Type : uint32_t {
        VERTEX_BUFFER,
        INDEX_BUFFER,
        TEXTURE_BUFFER
    };
};

struct RenderTargetFormat {
    VkSurfaceFormatKHR colorAttachmentFormat;
    VkFormat depthAttachmentFormat;
    VkExtent2D viewportExtent;
};