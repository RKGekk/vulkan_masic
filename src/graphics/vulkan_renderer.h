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

#include "api/vulkan_command_pool_type.h"
#include "../engine/views/iengine_view.h"

class VulkanDevice;
class VulkanImageBuffer;
class CommandBatch;
class VulkanSwapChain;
class VulkanDescriptorsManager;
class VulkanShadersManager;
class VulkanPipelinesManager;
class VulkanFenceManager;
class VulkanSemaphoresManager;
class VulkanCommandManager;
class VulkanRenderPassesManager;
class VulkanFormatManager;
class VulkanResourcesManager;
class RenderNode;
class RenderGraph;

struct Managers {
    std::shared_ptr<VulkanDescriptorsManager> descriptors_manager;
    std::shared_ptr<VulkanShadersManager> shaders_manager;
    std::shared_ptr<VulkanPipelinesManager> pipelines_manager;
	std::shared_ptr<VulkanFenceManager> fence_manager;
	std::shared_ptr<VulkanSemaphoresManager> semaphore_manager;
    std::shared_ptr<VulkanCommandManager> command_manager;
    std::shared_ptr<VulkanRenderPassesManager> render_passes_manager;
    std::shared_ptr<VulkanFormatManager> format_manager;
    std::shared_ptr<VulkanResourcesManager> resources_manager;
};

struct PerFrame {
    bool init(std::shared_ptr<VulkanDevice> device, unsigned index);
    void destroy();
	
	bool wait(uint64_t timeout);
	void begin();

	unsigned frame_index;
	std::shared_ptr<Managers> managers_ptr;

	std::vector<VkSemaphore> timeline_semaphores;
	std::vector<uint64_t> timeline_fences;
    std::shared_ptr<VulkanImageBuffer> out_color_image;
    std::shared_ptr<VulkanImageBuffer> out_depth_image;

	std::vector<VkFence> wait_and_recycle_fences;

	std::shared_ptr<CommandBatch> command_buffer;
	std::vector<VkSemaphore> recycled_semaphores;
	std::vector<VkEvent> recycled_events;
	std::vector<VkSemaphore> consumed_semaphores;
};

class VulkanRenderer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<WindowSurface> window, std::shared_ptr<ThreadPool> thread_pool);
    void destroy();

    const std::shared_ptr<VulkanSwapChain>& getSwapchain() const;
    std::shared_ptr<VulkanDevice>& GetDevice();
    std::shared_ptr<Managers>& getManagers();
    std::vector<std::shared_ptr<VulkanImageBuffer>>& getOutColorImages();
    std::vector<std::shared_ptr<VulkanImageBuffer>>& getOutDepthImages();

    void recordCommandBuffer(CommandBatch& command_buffer);
    void drawFrame();
    
    void update_frame(const GameTimerDelta& delta, uint32_t image_index);
    void addRenderNode(std::shared_ptr<RenderNode> render_node);

private:
    void TransitionResourcesToProperState(const std::shared_ptr<RenderNode>& render_node, CommandBatch& command_buffer);

    std::shared_ptr<VulkanDevice> m_device;
    
    std::shared_ptr<VulkanSwapChain> m_swapchain;

    std::vector<std::shared_ptr<PerFrame>> m_per_frame;
    std::vector<std::shared_ptr<VulkanImageBuffer>> m_out_color_images;
    std::vector<std::shared_ptr<VulkanImageBuffer>> m_out_depth_images;
    std::shared_ptr<Managers> m_managers;
    std::shared_ptr<RenderGraph> m_render_graph;
    
    std::shared_ptr<ThreadPool> m_thread_pool;
};