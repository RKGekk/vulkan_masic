#include "dependency_level.h"

#include "render_node.h"

#include <utility>

bool DependencyLevel::init(int level) {
    m_level = level;

    return true;
}

void DependencyLevel::destroy() {

}

const std::vector<std::shared_ptr<RenderNode>>& DependencyLevel::getNodes() const {
    return m_nodes;
}

int DependencyLevel::getLevel() const {
    return m_level;
}

void DependencyLevel::addNode(std::shared_ptr<RenderNode> node) {
    m_nodes.push_back(std::move(node));
}