#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../tools/game_timer.h"
#include "render_resource.h"

#include <cstdint>
#include <vector>

struct RenderTarget {
    using Attachment = std::vector<VkImageView>;
    RenderTargetFormat render_target_fmt;
    uint32_t frame_count;
    std::vector<Attachment> frames;
};

class IVulkanDrawable {
public:
    virtual void reset(const RenderTarget& rt) = 0;
    virtual void destroy() = 0;
    virtual void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t frame_index) = 0;
    virtual void update(const GameTimerDelta& delta, uint32_t image_index) = 0;
};