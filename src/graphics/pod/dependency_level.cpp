#include "dependency_level.h"

#include "render_node.h"
#include "graphics_render_node.h"
#include "graphics_render_node_config.h"
#include "framebuffer_config.h"
#include "render_pass_config.h"
#include "pipeline_config.h"
#include "composite_command_batch.h"
#include "../api/vulkan_framebuffer.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_render_pass.h"
#include "composite_command_batch.h"
#include "../api/vulkan_command_buffer.h"

#include <algorithm>
#include <utility>

DependencyLevel::DependencyLevel(int level, uint32_t image_index) : m_level(level), m_image_index(image_index) {}

void DependencyLevel::destroy() {
    // for(const auto&[fb, command_batch] : m_fb_to_command_buffer_map) {
    //     command_batch->destroy();
    // }
}

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

const std::unordered_map<std::shared_ptr<VulkanFramebuffer>, std::shared_ptr<CompositeCommandBatch>> DependencyLevel::getFBtoCmdBufferMap() const {
    return m_fb_to_command_buffer_map;
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

void DependencyLevel::recordCommandBuffer() {
    using namespace std::literals;

    GraphicsRenderNode::BindFlags pipeline_bind_all_flags = GraphicsRenderNode::ALL_BIND_FLAGS;
    GraphicsRenderNode::BindFlags no_pipeline_bind_flags = pipeline_bind_all_flags & ~GraphicsRenderNode::BindFlags::Pipeline;

    for (const auto&[rp_order, fb_set] : m_order_to_fb_map) {
        for(const std::shared_ptr<VulkanFramebuffer>& fb : fb_set) {

            if(!m_fb_to_command_buffer_map.contains(fb)) {
                std::string command_batch_name = fb->getName() + "_composite_command_batch"s + std::to_string(m_image_index);
                std::shared_ptr<CompositeCommandBatch> command_batch = std::make_shared<CompositeCommandBatch>();
                command_batch->init(fb, m_image_index, 0u, m_fb_to_pipeline_map.at(fb).size(), command_batch_name);
                m_fb_to_command_buffer_map[fb] = std::move(command_batch);
            }

            const std::shared_ptr<CompositeCommandBatch>& cmd_batch = m_fb_to_command_buffer_map[fb];

            size_t pipeline_num = 0u;
            for(const auto& pipeline : m_fb_to_pipeline_map.at(fb)) {
                GraphicsRenderNode::BindFlags pipeline_bind_flags = pipeline_bind_all_flags;
                bool is_recorded = cmd_batch->isRecorded(VK_COMMAND_BUFFER_LEVEL_SECONDARY, pipeline_num);
                bool is_dirty_read = isDirtyRead(pipeline);
                bool need_record_cmd = !is_recorded || is_dirty_read;
                if(need_record_cmd) {
                    cmd_batch->beginSecondaryCommandBuffer(fb->getRenderpass()->getRenderPassConfig()->getSubpassName(0u), pipeline_num);
                    for(const std::shared_ptr<GraphicsRenderNode>& graphics_node : m_pipeline_to_node_map.at(pipeline)) {
                        graphics_node->render(cmd_batch->getSecondaryCommandBuffer()->getCommandBufer(pipeline_num), static_cast<uint32_t>(pipeline_bind_flags));
                        pipeline_bind_flags = no_pipeline_bind_flags;
                    }
                    cmd_batch->endSecondaryCommandBuffer(pipeline_num);
                }
                ++pipeline_num;
            }

            bool is_dirty_write = isDirtyWrite(fb);
            if(is_dirty_write) {
                cmd_batch->beginPrimaryCommandBuffer(
                    [this, &fbuff = fb]
                    (VkCommandBuffer primary_command_buffer){
                        TransitionResourcesToProperState(primary_command_buffer, fbuff);
                    }
                );
                cmd_batch->endPrimaryCommandBuffer();
            }
        }
    }
}

bool DependencyLevel::isDirtyWrite(const std::shared_ptr<VulkanFramebuffer>& fb) const {
    for(const std::shared_ptr<GraphicsRenderNode>& gnode : m_fb_to_node_map.at(fb)) {
        if (gnode->isDirtyWrite()) return true;
    }
    return false;
}

bool DependencyLevel::isDirtyRead(const std::shared_ptr<VulkanPipeline>& pipeline) const {
    for(const std::shared_ptr<GraphicsRenderNode>& gnode : m_pipeline_to_node_map.at(pipeline)) {
        if (gnode->isDirtyRead()) return true;
    }
    return false;
}