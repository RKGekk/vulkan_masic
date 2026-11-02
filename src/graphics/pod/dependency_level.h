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
class CompositeCommandBatch;

class DependencyLevel {
public:
    DependencyLevel(int level, uint32_t image_index);

    const std::unordered_set<std::shared_ptr<RenderNode>>& getNodes() const;

    const std::map<size_t, std::unordered_set<std::shared_ptr<VulkanFramebuffer>>>& getOrderToFBmap() const;
    const std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::unordered_set<std::shared_ptr<GraphicsRenderNode>>>& getFBtoNodeMap() const;
    const std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::unordered_set<std::shared_ptr<VulkanPipeline>>>& getFBtoPipelineMap() const;
    const std::unordered_map<std::shared_ptr<VulkanPipeline>, std::unordered_set<std::shared_ptr<GraphicsRenderNode>>>& getPipelineToNodeMap() const;
    const std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::shared_ptr<CompositeCommandBatch>> getFBtoCmdBufferMap() const;

    int getLevel() const;
    void addNode(std::shared_ptr<RenderNode> node);
    void TransitionResourcesToProperState(VkCommandBuffer primary_command_buffer) const;
    void TransitionResourcesToProperState(VkCommandBuffer primary_command_buffer, const std::shared_ptr<VulkanFramebuffer>& fb) const;

    void recordCommandBuffer();
    void beginFrame();

    bool isDirtyWrite(const std::shared_ptr<VulkanFramebuffer>& fb) const;
    bool isDirtyRead(const std::shared_ptr<VulkanPipeline>& pipeline) const;

private:
    std::unordered_set<std::shared_ptr<RenderNode>> m_nodes;
    std::map<size_t, std::unordered_set<std::shared_ptr<VulkanFramebuffer>>> m_order_to_fb_map;
    std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::unordered_set<std::shared_ptr<GraphicsRenderNode>>> m_fb_to_node_map;
    std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::unordered_set<std::shared_ptr<VulkanPipeline>>> m_fb_to_pipeline_map;
    std::unordered_map<std::shared_ptr<VulkanPipeline>, std::unordered_set<std::shared_ptr<GraphicsRenderNode>>> m_pipeline_to_node_map;
    int m_level;
    uint32_t m_image_index;

    std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::shared_ptr<CompositeCommandBatch>> m_fb_to_command_buffer_map;
};