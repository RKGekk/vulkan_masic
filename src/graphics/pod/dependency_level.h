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

    DependencyLevel(int level);

    const std::vector<std::shared_ptr<RenderNode>>& getNodes() const;
    const std::unordered_map<FramebufferName, std::shared_ptr<GraphicsRenderNode>>& getFramebufferNodeMap() const;
    int getLevel() const;
    void addNode(std::shared_ptr<RenderNode> node);

private:
    std::vector<std::shared_ptr<RenderNode>> m_nodes;
    std::unordered_map<FramebufferName, std::shared_ptr<GraphicsRenderNode>> m_framebuffer_node_map;
    int m_level;
};