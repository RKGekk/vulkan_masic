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

struct PerFrame {
    bool init(std::shared_ptr<VulkanDevice> device, unsigned index);
    void destroy();
	
	bool wait(uint64_t timeout);
	void begin();

	unsigned frame_index;

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
    
    std::shared_ptr<VulkanDescriptorsManager>& getDescriptorsManager();
    std::shared_ptr<VulkanShadersManager>& getShadersManager();
    std::shared_ptr<VulkanPipelinesManager>& getPipelinesManager();
	std::shared_ptr<VulkanFenceManager>& getFenceManager();
	std::shared_ptr<VulkanSemaphoresManager>& getSemaphoreManager();
    std::shared_ptr<VulkanCommandManager>& getCommandManager();
    std::shared_ptr<VulkanRenderPassesManager>& getRenderPassesManager();
    std::shared_ptr<VulkanFormatManager>& getFormatManager();
    std::shared_ptr<VulkanResourcesManager>& getResourcesManager();

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
    
    std::shared_ptr<VulkanDescriptorsManager> m_descriptors_manager;
    std::shared_ptr<VulkanShadersManager> m_shaders_manager;
    std::shared_ptr<VulkanPipelinesManager> m_pipelines_manager;
	std::shared_ptr<VulkanFenceManager> m_fence_manager;
	std::shared_ptr<VulkanSemaphoresManager> m_semaphore_manager;
    std::shared_ptr<VulkanCommandManager> m_command_manager;
    std::shared_ptr<VulkanRenderPassesManager> m_render_passes_manager;
    std::shared_ptr<VulkanFormatManager> m_format_manager;
    std::shared_ptr<VulkanResourcesManager> m_resources_manager;

    std::shared_ptr<RenderGraph> m_render_graph;
    
    std::shared_ptr<ThreadPool> m_thread_pool;
};