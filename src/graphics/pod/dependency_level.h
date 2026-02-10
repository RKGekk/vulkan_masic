#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

class RenderNode;

class DependencyLevel {
public:
    bool init(int level);
    void destroy();

    const std::vector<std::shared_ptr<RenderNode>>& getNodes() const;
    int getLevel() const;
    void addNode(std::shared_ptr<RenderNode> node);

private:
    std::vector<std::shared_ptr<RenderNode>> m_nodes;
    int m_level;
};