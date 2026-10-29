#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <pugixml.hpp>

#include "render_node.h"

class VulkanSwapChain;

class PresentRenderNode : public RenderNode {
public:
    virtual bool init(std::string render_node_name, unsigned frame_index, std::shared_ptr<VulkanDevice> device, const std::string& node_config_name, bool instance_config, std::weak_ptr<RenderGraph> render_graph) override;
    virtual void destroy() override;

    virtual void render(VkCommandBuffer secondary_command_buffer) override;
    void present();
    virtual RenderNodeType getRenderNodeType() const override;
    virtual void finishRenderNode() override;

    virtual void TransitionResourcesToProperState(VkCommandBuffer primary_command_buffer) override;

    void addWaitSemaphore(VkSemaphore present_wait_for_semaphore);
    void clearWaitSemaphores();

private:
    std::shared_ptr<VulkanSwapChain> m_swapchain;
    std::vector<VkSemaphore> m_present_wait_sem;
    VkPresentInfoKHR m_present_info;
    size_t m_present_count;
};