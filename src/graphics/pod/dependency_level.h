#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
    using FramebufferName = std::string;
    using PipelineName = std::string;
    using RenderPassName = std::string;

    DependencyLevel(int level);

    const std::vector<std::shared_ptr<RenderNode>>& getNodes() const;
    const std::shared_ptr<RenderNode>& getNode(size_t idx) const;

    const std::unordered_set<FramebufferName>& getFramebuffers() const;

    const std::unordered_map<FramebufferName, std::vector<RenderPassName>>& getFBtoRenderpassMap() const;
    const std::vector<RenderPassName>& getRenderpasses(const FramebufferName& fb_name) const;

    const std::unordered_map<RenderPassName, std::unordered_set<std::shared_ptr<VulkanPipeline>>>& getRenderpassToPipelineMap() const;
    const std::unordered_set<std::shared_ptr<VulkanPipeline>>& getPipelines(const RenderPassName& pass_name) const;

    const std::unordered_map<PipelineName, std::vector<std::shared_ptr<GraphicsRenderNode>>>& getPipelineNodeMap() const;
    const std::vector<std::shared_ptr<GraphicsRenderNode>>& getGraphicsNodes(const PipelineName& pipeline_name) const;

    int getLevel() const;
    void addNode(std::shared_ptr<RenderNode> node);

    void sortPipelineNodes();
    //void sortPipelines();
    //const std::vector<PipelineName>& getOrderedPipelineNames() const;

private:
    std::vector<std::shared_ptr<RenderNode>> m_nodes;
    std::unordered_set<FramebufferName> m_framebuffers;
    std::unordered_map<FramebufferName, std::vector<RenderPassName>> m_framebuffer_to_renderpass_map;
    std::unordered_map<RenderPassName, std::unordered_set<std::shared_ptr<VulkanPipeline>>> m_renderpass_to_pipeline_map;
    std::unordered_map<PipelineName, std::vector<std::shared_ptr<GraphicsRenderNode>>> m_pipeline_node_map;
    int m_level;
};