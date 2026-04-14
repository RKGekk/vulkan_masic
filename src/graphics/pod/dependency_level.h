#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <unordered_map>
#include <vector>

class RenderNode;
class GraphicsRenderNode;

class DependencyLevel {
public:
    using FramebufferName = std::string;
    using PipelineName = std::string;

    DependencyLevel(int level);

    const std::vector<std::shared_ptr<RenderNode>>& getNodes() const;
    const std::unordered_map<FramebufferName, std::vector<std::shared_ptr<GraphicsRenderNode>>>& getFramebufferNodeMap() const;
    const std::unordered_map<PipelineName, std::vector<std::shared_ptr<GraphicsRenderNode>>>& getPipelineNodeMap() const;
    const std::unordered_map<FramebufferName, std::vector<PipelineName>>& getFramebufferToPipelineMap() const;
    int getLevel() const;
    void addNode(std::shared_ptr<RenderNode> node);

    void sortPipelines();

private:
    std::vector<std::shared_ptr<RenderNode>> m_nodes;
    std::unordered_map<FramebufferName, std::vector<std::shared_ptr<GraphicsRenderNode>>> m_framebuffer_node_map;
    std::unordered_map<PipelineName, std::vector<std::shared_ptr<GraphicsRenderNode>>> m_pipeline_node_map;
    std::unordered_map<FramebufferName, std::vector<PipelineName>> m_framebuffer_to_pipeline_map;
    int m_level;
};