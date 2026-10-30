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
    enum class BindFlags : uint32_t {
        None = 0,
        Pipeline = 1 << 0, // 0x01
        PushConstants = 1 << 1, // 0x02
        Viewport = 1 << 2, // 0x04
        Scissor  = 1 << 3, // 0x08
        DescriptorSets = 1 << 4, // 0x10
        VertexIndexInstanceBuffers = 1 << 5, // 0x20
        Draw = 1 << 6 // 0x40
    };
    const static BindFlags ALL_BIND_FLAGS;

    virtual bool init(std::string render_node_name, unsigned frame_index, std::shared_ptr<VulkanDevice> device, const std::string& node_config_name, bool instance_config, std::weak_ptr<RenderGraph> render_graph) override;
    virtual void destroy() override;

    virtual void render(VkCommandBuffer secondary_command_buffer, uint32_t bind_flags) override;
    virtual void finishRenderNode() override;
    virtual RenderNodeType getRenderNodeType() const override;
    virtual void updateDescriptors() override;
    virtual void updateDescriptor(const std::string& desc_binding_name) override;
    virtual void updateDescriptor(uint32_t desc_slot, uint32_t desc_binding_num) override;

    void initFramebuffer();

    const std::shared_ptr<VulkanPipeline>& getPipeline();
    VkFramebuffer getVkFramebuffer() const;
    const std::shared_ptr<VulkanFramebuffer>& getFB() const;

    std::shared_ptr<GraphicsRenderNodeConfig>& getGraphicsRenderNodeConfig();

    virtual void TransitionResourcesToProperState(VkCommandBuffer primary_command_buffer) override;

private:
    std::shared_ptr<VulkanPipeline> m_pipeline;

    std::vector<VkBuffer> m_vertex_buffers;
    std::vector<VkDeviceSize> m_vertex_buffers_offsets;
    uint32_t m_first_binding;
    uint32_t m_vertex_count;
    uint32_t m_instance_count;

    std::shared_ptr<GraphicsRenderNodeConfig> m_node_config;
    std::shared_ptr<VulkanFramebuffer> m_frame_buffer;
};

constexpr GraphicsRenderNode::BindFlags operator|(GraphicsRenderNode::BindFlags lhs, GraphicsRenderNode::BindFlags rhs) {
    return static_cast<GraphicsRenderNode::BindFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr GraphicsRenderNode::BindFlags operator&(GraphicsRenderNode::BindFlags lhs, GraphicsRenderNode::BindFlags rhs) {
    return static_cast<GraphicsRenderNode::BindFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr bool has_flag(GraphicsRenderNode::BindFlags mask, GraphicsRenderNode::BindFlags flag) {
    return (mask & flag) == flag;
}

constexpr GraphicsRenderNode::BindFlags operator~(GraphicsRenderNode::BindFlags val) {
    return static_cast<GraphicsRenderNode::BindFlags>(~static_cast<uint32_t>(val));
}

// Optional: Overload the compound AND assignment operator for convenience
inline GraphicsRenderNode::BindFlags& operator&=(GraphicsRenderNode::BindFlags& lhs, GraphicsRenderNode::BindFlags rhs) {
    lhs = lhs & rhs;
    return lhs;
}