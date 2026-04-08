#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <pugixml.hpp>

#include "render_node.h"

class GraphicsRenderNode : public RenderNode {
public:
    virtual bool init(std::shared_ptr<VulkanDevice> device, const std::string& node_config_name, std::weak_ptr<RenderGraph> render_graph) override;
    virtual void destroy() override;

    virtual void render(CommandBatch& command_buffer) override;
    virtual void finishRenderNode() override;

    const std::shared_ptr<VulkanPipeline>& getPipeline();
    VkFramebuffer getFramebuffer() const;
    VkExtent2D getViewportExtent() const;

    const std::unordered_map<uint32_t, std::shared_ptr<VulkanDescriptor>>& getDescriptors() const;
    const std::shared_ptr<GraphicsRenderNodeConfig>& getGraphicsRenderNodeConfig() const;

    virtual void TransitionResourcesToProperState(CommandBatch& command_buffer) override;

private:

    std::vector<VkImageView> getAttachments() const;

    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanPipeline> m_pipeline;
    std::weak_ptr<RenderGraph> m_render_graph;

    std::shared_ptr<GraphicsRenderNodeConfig> m_node_config;

    std::vector<VkImageView> m_framebuffers_attachments;
    VkFramebufferCreateInfo m_framebuffer_info;
    VkFramebuffer m_frame_buffer;
    VkExtent2D m_viewport_extent;

    std::unordered_map<uint32_t, std::shared_ptr<VulkanDescriptor>> m_descs;
};