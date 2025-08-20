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
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_drawable.h"
#include "vulkan_shader.h"
#include "vulkan_pipeline.h"
#include "basic_vertex.h"
#include "basic_uniform.h"
#include "basic_drawable.h"
#include "vulkan_uniform_buffer.h"

class VulkanRenderer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window);
    void destroy();
    void recreate();

    const VulkanSwapChain& getSwapchain() const;
    const std::vector<ImageBufferAndView>& getColorImages() const;
    const std::vector<ImageBufferAndView>& getDepthImages() const;
    RenderTarget getRenderTarget() const;

    void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index);
    void drawFrame();
    void setFramebufferResized();
    void update_frame(const GameTimerDelta& delta, uint32_t image_index);
    void addDrawable(std::shared_ptr<IVulkanDrawable> drawable);

private:

    void createColorResources();
    void createDepthResources();
    void createSyncObjects();

    std::shared_ptr<VulkanDevice> m_device;

    VulkanSwapChain m_swapchain;

    std::vector<ImageBufferAndView> m_out_color_images;
    std::vector<ImageBufferAndView> m_out_depth_images;

    std::vector<VkCommandBuffer> m_command_buffers;

    std::vector<std::shared_ptr<IVulkanDrawable>> m_drawable_list;
    
    std::vector<VkSemaphore> m_image_available; // signaled when the presentation engine is finished using the image.
    std::vector<VkSemaphore> m_render_finished;
    std::vector<VkFence> m_in_flight_frame; // will be signaled when the command buffers finish

    bool m_framebuffer_resized = false;
};