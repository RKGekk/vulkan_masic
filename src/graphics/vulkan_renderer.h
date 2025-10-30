#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../tools/string_tools.h"
#include "../tools/game_timer.h"
#include "../tools/thread_pool.h"
#include "../window_surface.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_drawable.h"
#include "vulkan_shader.h"
#include "vulkan_pipeline.h"
#include "vulkan_image_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_command_pool_type.h"
#include "vulkan_uniform_buffer.h"
#include "../engine/iengine_view.h"

class VulkanRenderer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, std::shared_ptr<ThreadPool> thread_pool);
    void destroy();
    void recreate();

    static std::shared_ptr<WindowSurface> CreateRenderWindow(const ApplicationOptions& options);

    static std::shared_ptr<WindowSurface> GetRenderWindow();
    const VulkanSwapChain& getSwapchain() const;
    const std::vector<VulkanImageBuffer>& getColorImages() const;
    const std::vector<VulkanImageBuffer>& getDepthImages() const;
    RenderTarget getRenderTarget() const;

    void recordCommandBuffer(CommandBatch& command_buffer);
    void drawFrame();
    void setFramebufferResized();
    void update_frame(const GameTimerDelta& delta, uint32_t image_index);
    void addDrawable(std::shared_ptr<IVulkanDrawable> drawable);

private:

    void createColorResources();
    void createDepthResources();

    std::shared_ptr<VulkanDevice> m_device;

    VulkanSwapChain m_swapchain;

    std::vector<VulkanImageBuffer> m_out_color_images;
    std::vector<VulkanImageBuffer> m_out_depth_images;

    std::vector<CommandBatch> m_command_buffers;
    std::shared_ptr<ThreadPool> m_thread_pool;

    std::vector<std::shared_ptr<IVulkanDrawable>> m_drawable_list;

    bool m_framebuffer_resized = false;
};