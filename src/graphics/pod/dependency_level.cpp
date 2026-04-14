#include "dependency_level.h"

#include "render_node.h"
#include "graphics_render_node.h"
#include "graphics_render_node_config.h"
#include "framebuffer_config.h"
#include "render_pass_config.h"
#include "pipeline_config.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_render_pass.h"

#include <algorithm>
#include <utility>

DependencyLevel::DependencyLevel(int level) : m_level(level) {}

const std::vector<std::shared_ptr<RenderNode>>& DependencyLevel::getNodes() const {
    return m_nodes;
}

const std::unordered_map<DependencyLevel::FramebufferName, std::vector<std::shared_ptr<GraphicsRenderNode>>>& DependencyLevel::getFramebufferNodeMap() const {
    return m_framebuffer_node_map;
}

const std::unordered_map<DependencyLevel::PipelineName, std::vector<std::shared_ptr<GraphicsRenderNode>>>& DependencyLevel::getPipelineNodeMap() const {
    return m_pipeline_node_map;
}

const std::unordered_map<DependencyLevel::FramebufferName, std::vector<DependencyLevel::PipelineName>>& DependencyLevel::getFramebufferToPipelineMap() const {
    return m_framebuffer_to_pipeline_map;
}

int DependencyLevel::getLevel() const {
    return m_level;
}

void DependencyLevel::addNode(std::shared_ptr<RenderNode> node) {
    m_nodes.push_back(node);
    if(std::shared_ptr<GraphicsRenderNode> graphics_node = std::dynamic_pointer_cast<GraphicsRenderNode>(node)) {
        const FramebufferName& framebuffer_name = graphics_node->getGraphicsRenderNodeConfig()->getFramebufferConfig()->getName();
        m_framebuffer_node_map[framebuffer_name].push_back(graphics_node);

        const PipelineName& pipeline_name = graphics_node->getPipeline()->getPipelineConfig()->getName();
        m_pipeline_node_map[pipeline_name].push_back(graphics_node);

        m_framebuffer_to_pipeline_map[framebuffer_name].push_back(pipeline_name);
    }
}

void DependencyLevel::sortPipelines() {
    for(auto&[pipeline_name, render_nodes] : m_pipeline_node_map) {
        std::sort(
            render_nodes.begin(),
            render_nodes.end(),
            [](const auto& a, const auto& b) {
                return !a->getPipeline()->getPipelineConfig()->haveBlendEnableAttachments();
            }
        );
    }
}