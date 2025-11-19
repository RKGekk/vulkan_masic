#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../tools/game_timer.h"
#include "render_resource.h"
#include "vulkan_command_buffer.h"
#include "vulkan_image_buffer.h"
#include "vulkan_render_target.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

class IVulkanDrawable {
public:
    virtual void reset(const RenderTarget& rt) = 0;
    virtual void destroy() = 0;
    virtual void recordCommandBuffer(CommandBatch& command_buffer, const RenderTarget& rt, uint32_t frame_index) = 0;
    virtual void update(const GameTimerDelta& delta, uint32_t image_index) = 0;
};