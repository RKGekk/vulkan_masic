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
#include "api/vulkan_device.h"
#include "api/vulkan_swapchain.h"
#include "drawables/vulkan_drawable.h"
#include "api/vulkan_descriptors_manager.h"
#include "api/vulkan_shaders_manager.h"
#include "api/vulkan_shader.h"
#include "api/vulkan_pipelines_manager.h"
#include "api/vulkan_pipeline.h"
#include "api/vulkan_image_buffer.h"
#include "api/vulkan_command_buffer.h"
#include "api/vulkan_command_pool_type.h"
#include "api/vulkan_uniform_buffer.h"
#include "../engine/views/iengine_view.h"

class VulkanRenderer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, std::shared_ptr<ThreadPool> thread_pool);
    void destroy();
    void recreate();

    const std::shared_ptr<VulkanSwapChain>& getSwapchain() const;
    const RenderTarget& getRenderTarget(uint32_t image_index = 0u) const;
    std::shared_ptr<VulkanDevice> GetDevice();
    std::shared_ptr<VulkanDescriptorsManager> getShadersManager();
    std::shared_ptr<VulkanShadersManager> getShadersManager();
    std::shared_ptr<VulkanPipelinesManager> getPipelinesManager();

    void recordCommandBuffer(CommandBatch& command_buffer);
    void drawFrame();
    void setFramebufferResized();
    void update_frame(const GameTimerDelta& delta, uint32_t image_index);
    void addDrawable(std::shared_ptr<IVulkanDrawable> drawable);

private:

    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanDescriptorsManager> m_descriptors_manager;
    std::shared_ptr<VulkanShadersManager> m_shaders_manager;
    std::shared_ptr<VulkanPipelinesManager> m_pipelines_manager;
    std::shared_ptr<VulkanSwapChain> m_swapchain;
    std::vector<RenderTarget> m_render_targets;

    std::vector<CommandBatch> m_command_buffers;
    std::shared_ptr<ThreadPool> m_thread_pool;

    std::vector<std::shared_ptr<IVulkanDrawable>> m_drawable_list;

    bool m_framebuffer_resized = false;
};