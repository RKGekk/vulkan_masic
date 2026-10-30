#include "dependency_level.h"

#include "render_node.h"
#include "graphics_render_node.h"
#include "graphics_render_node_config.h"
#include "framebuffer_config.h"
#include "render_pass_config.h"
#include "pipeline_config.h"
#include "../api/vulkan_framebuffer.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_render_pass.h"

#include <algorithm>
#include <utility>

DependencyLevel::DependencyLevel(int level) : m_level(level) {}

const std::map<size_t, std::unordered_set<std::shared_ptr<VulkanFramebuffer>>>& DependencyLevel::getOrderToFBmap() const {
    return m_order_to_fb_map;
}

const std::unordered_map<std::shared_ptr<VulkanFramebuffer>,  std::unordered_set<std::shared_ptr<GraphicsRenderNode>>>& DependencyLevel::getFBtoNodeMap() const {
    return m_fb_to_node_map;
}

const std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::unordered_set<std::shared_ptr<VulkanPipeline>>>& DependencyLevel::getFBtoPipelineMap() const {
    return m_fb_to_pipeline_map;
}

const std::unordered_map<std::shared_ptr<VulkanPipeline>, std::unordered_set<std::shared_ptr<GraphicsRenderNode>>>& DependencyLevel::getPipelineToNodeMap() const {
    return m_pipeline_to_node_map;
}
    
int DependencyLevel::getLevel() const {
    return m_level;
}

void DependencyLevel::addNode(std::shared_ptr<RenderNode> node) {
    m_nodes.insert(node);

    std::shared_ptr<GraphicsRenderNode> graphics_node = std::dynamic_pointer_cast<GraphicsRenderNode>(node);
    if(!graphics_node) return;

    std::shared_ptr<VulkanFramebuffer> framebuffer = graphics_node->getFB();
    std::shared_ptr<VulkanPipeline> pipeline = graphics_node->getPipeline();
    m_order_to_fb_map[framebuffer->getRenderpass()->getRenderPassConfig()->getPriority()].insert(framebuffer);
    m_fb_to_node_map[framebuffer].insert(graphics_node);
    m_fb_to_pipeline_map[framebuffer].insert(pipeline);
    m_pipeline_to_node_map[pipeline].insert(graphics_node);
}

void DependencyLevel::TransitionResourcesToProperState(VkCommandBuffer primary_command_buffer) const {
    for(const std::shared_ptr<RenderNode>& render_node : m_nodes) {
        render_node->TransitionResourcesToProperState(primary_command_buffer);
    }
}

void DependencyLevel::TransitionResourcesToProperState(VkCommandBuffer primary_command_buffer, const std::shared_ptr<VulkanFramebuffer>& fb) const {
    for(const std::shared_ptr<RenderNode>& render_node : m_fb_to_node_map.at(fb)) {
        render_node->TransitionResourcesToProperState(primary_command_buffer);
    }
}