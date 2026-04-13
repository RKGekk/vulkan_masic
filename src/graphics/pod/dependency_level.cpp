#include "dependency_level.h"

#include "render_node.h"
#include "graphics_render_node.h"
#include "graphics_render_node_config.h"
#include "framebuffer_config.h"

#include <utility>

DependencyLevel::DependencyLevel(int level) : m_level(level) {}

const std::vector<std::shared_ptr<RenderNode>>& DependencyLevel::getNodes() const {
    return m_nodes;
}

const std::unordered_map<DependencyLevel::FramebufferName, std::shared_ptr<GraphicsRenderNode>>& DependencyLevel::getFramebufferNodeMap() const {
    return m_framebuffer_node_map;
}

int DependencyLevel::getLevel() const {
    return m_level;
}

void DependencyLevel::addNode(std::shared_ptr<RenderNode> node) {
    m_nodes.push_back(node);
    if(std::shared_ptr<GraphicsRenderNode> graphics_node = std::dynamic_pointer_cast<GraphicsRenderNode>(node)) {
        const FramebufferName& framebuffer_name = graphics_node->getGraphicsRenderNodeConfig()->getFramebufferConfig().getName();
        m_framebuffer_node_map[framebuffer_name] = std::move(graphics_node);
    }
}