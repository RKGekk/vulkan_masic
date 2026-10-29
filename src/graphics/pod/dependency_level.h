#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class RenderNode;
class GraphicsRenderNode;
class VulkanFramebuffer;
class VulkanPipeline;

class DependencyLevel {
public:
    DependencyLevel(int level);

    const std::unordered_set<std::shared_ptr<RenderNode>>& getNodes() const;

    const std::map<size_t, std::unordered_set<std::shared_ptr<VulkanFramebuffer>>>& getOrderToFBmap() const;
    const std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::unordered_set<std::shared_ptr<GraphicsRenderNode>>>& getFBtoNodeMap() const;

    int getLevel() const;
    void addNode(std::shared_ptr<RenderNode> node);
    void TransitionResourcesToProperState(VkCommandBuffer primary_command_buffer) const;

private:
    std::unordered_set<std::shared_ptr<RenderNode>> m_nodes;
    std::map<size_t, std::unordered_set<std::shared_ptr<VulkanFramebuffer>>> m_order_to_fb_map;
    std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::unordered_set<std::shared_ptr<GraphicsRenderNode>>> m_fb_to_node_map;
    int m_level;
};