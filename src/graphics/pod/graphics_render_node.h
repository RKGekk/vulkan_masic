#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <pugixml.hpp>

#include "render_node.h"

class VulkanFramebuffer;

class GraphicsRenderNode : public RenderNode {
public:
    virtual bool init(std::shared_ptr<VulkanDevice> device, const std::string& node_config_name, bool instance_config, std::weak_ptr<RenderGraph> render_graph) override;
    virtual void destroy() override;

    virtual void render(CommandBatch& command_buffer, unsigned image_index) override;
    virtual void finishRenderNode() override;

    const std::shared_ptr<VulkanPipeline>& getPipeline();
    VkFramebuffer getVkFramebuffer() const;
    const std::shared_ptr<VulkanFramebuffer>& getFB() const;

    std::shared_ptr<GraphicsRenderNodeConfig>& getGraphicsRenderNodeConfig();

    virtual void TransitionResourcesToProperState(CommandBatch& command_buffer) override;

private:
    std::shared_ptr<VulkanPipeline> m_pipeline;

    std::vector<const VkBuffer> m_vertex_buffers;
    std::vector<VkDeviceSize> m_vertex_buffers_offsets;
    uint32_t m_first_binding;
    uint32_t m_vertex_count;
    uint32_t m_instance_count;
    size_t m_index_buffer_bind_num;

    std::shared_ptr<GraphicsRenderNodeConfig> m_node_config;
    std::shared_ptr<VulkanFramebuffer> m_frame_buffer;
};