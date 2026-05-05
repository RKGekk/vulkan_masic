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

const std::vector<std::shared_ptr<RenderNode>>& DependencyLevel::getNodes() const {
    return m_nodes;
}

const std::shared_ptr<RenderNode>& DependencyLevel::getNode(size_t idx) const {
    return m_nodes.at(idx);
}

const std::unordered_set<DependencyLevel::FramebufferName>& DependencyLevel::getFramebuffers() const {
    return m_framebuffers;
}

const std::unordered_map<DependencyLevel::FramebufferName, std::vector<DependencyLevel::RenderPassName>>& DependencyLevel::getFBtoRenderpassMap() const {
    return m_framebuffer_to_renderpass_map;
}

const std::vector<DependencyLevel::RenderPassName>& DependencyLevel::getRenderpasses(const DependencyLevel::FramebufferName& fb_name) const {
    return m_framebuffer_to_renderpass_map.at(fb_name);
}

const std::unordered_map<DependencyLevel::RenderPassName, std::unordered_set<std::shared_ptr<VulkanPipeline>>>& DependencyLevel::getRenderpassToPipelineMap() const {
    return m_renderpass_to_pipeline_map;
}

const std::unordered_set<std::shared_ptr<VulkanPipeline>>& DependencyLevel::getPipelines(const DependencyLevel::RenderPassName& pass_name) const {
    return m_renderpass_to_pipeline_map.at(pass_name);
}

const std::unordered_map<DependencyLevel::PipelineName, std::vector<std::shared_ptr<GraphicsRenderNode>>>& DependencyLevel::getPipelineNodeMap() const {
    return m_pipeline_node_map;
}

const std::vector<std::shared_ptr<GraphicsRenderNode>>& DependencyLevel::getGraphicsNodes(const DependencyLevel::PipelineName& pipeline_name) const {
    return m_pipeline_node_map.at(pipeline_name);
}

int DependencyLevel::getLevel() const {
    return m_level;
}

void DependencyLevel::addNode(std::shared_ptr<RenderNode> node) {
    m_nodes.push_back(node);

    std::shared_ptr<GraphicsRenderNode> graphics_node = std::dynamic_pointer_cast<GraphicsRenderNode>(node);
    if(!graphics_node) return;

    const FramebufferName& framebuffer_name = graphics_node->getFB()->getFramebufferConfig()->getName();
    m_framebuffers.insert(framebuffer_name);

    const RenderPassName& renderpass_name = graphics_node->getPipeline()->getRenderPass()->getRenderPassConfig()->getName();
    const PipelineName& pipeline_name = graphics_node->getPipeline()->getPipelineConfig()->getName();
    m_framebuffer_to_renderpass_map[framebuffer_name].push_back(renderpass_name);
    m_renderpass_to_pipeline_map[renderpass_name].insert(graphics_node->getPipeline());
    m_pipeline_node_map[pipeline_name].push_back(graphics_node);
}

void DependencyLevel::sortPipelineNodes() {
    for (auto&[framebuffer_name, renderpass_names] : m_framebuffer_to_renderpass_map)  {
        std::sort(renderpass_names.begin(), renderpass_names.end());
        auto last = std::unique(renderpass_names.begin(), renderpass_names.end());
        renderpass_names.erase(last, renderpass_names.end());

        if(renderpass_names.size() <= 1) continue;
        std::sort(
            renderpass_names.begin(),
            renderpass_names.end(),
            [&](const auto& renderpass_name_a, const auto& renderpass_name_b) {
                const std::shared_ptr<VulkanPipeline>& pipeline_a = *m_renderpass_to_pipeline_map.at(renderpass_name_a).cbegin();
                const std::shared_ptr<VulkanPipeline>& pipeline_b = *m_renderpass_to_pipeline_map.at(renderpass_name_b).cbegin();

                return pipeline_a->getRenderPass()->getRenderPassConfig()->getPriority() < pipeline_b->getRenderPass()->getRenderPassConfig()->getPriority();
            }
        );
    }


    for(auto&[pipeline_name, render_nodes] : m_pipeline_node_map) {
        if(render_nodes.size() <= 1) continue;
        
        if(render_nodes[0]->getPipeline()->getPipelineConfig()->haveBlendEnableAttachments()) {
            std::sort(
                render_nodes.begin(),
                render_nodes.end(),
                [](const auto& a, const auto& b) {
                    return a->getExecutionOrder() < b->getExecutionOrder();
                }
            );
        }
    }
}

//void DependencyLevel::sortPipelines() {
    // for(auto&[pipeline_name, render_nodes] : m_pipeline_node_map) {
    //     if(render_nodes.size() <= 1) {
    //         continue;
    //     }
    //     std::sort(
    //         render_nodes.begin(),
    //         render_nodes.end(),
    //         [](const auto& a, const auto& b) {
    //             return !a->getPipeline()->getPipelineConfig()->haveBlendEnableAttachments();
    //         }
    //     );
    // }

    // for(auto&[pipeline_name, render_nodes] : m_pipeline_node_map) {
    //     if(!render_nodes[0]->getPipeline()->getPipelineConfig()->haveBlendEnableAttachments()) {
    //         m_pipeline_order.push_back(pipeline_name);
    //     }
    // }

    // for(auto&[pipeline_name, render_nodes] : m_pipeline_node_map) {
    //     if(render_nodes[0]->getPipeline()->getPipelineConfig()->haveBlendEnableAttachments()) {
    //         m_pipeline_order.push_back(pipeline_name);
    //     }
    // }

    // if(m_nodes.size() <= 1) {
    //     return;
    // }
    // std::sort(
    //     m_nodes.begin(),
    //     m_nodes.end(),
    //     [](auto& a, const auto& b) {
    //         if(std::shared_ptr<GraphicsRenderNode> graphics_node = std::dynamic_pointer_cast<GraphicsRenderNode>(a)) {
    //             return !graphics_node->getPipeline()->getPipelineConfig()->haveBlendEnableAttachments();
    //         }
    //         return false;
    //     }
    // );
//}

//const std::vector<DependencyLevel::PipelineName>& DependencyLevel::getOrderedPipelineNames() const {
//    return m_pipeline_order;
//}