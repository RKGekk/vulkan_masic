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
    VkFramebuffer getFramebuffer() const;

    const std::unordered_map<uint32_t, std::shared_ptr<VulkanDescriptor>>& getDescriptors() const;
    std::shared_ptr<GraphicsRenderNodeConfig>& getGraphicsRenderNodeConfig();

    virtual void TransitionResourcesToProperState(CommandBatch& command_buffer) override;

private:
    std::shared_ptr<VulkanPipeline> m_pipeline;

    std::shared_ptr<GraphicsRenderNodeConfig> m_node_config;
    std::shared_ptr<VulkanFramebuffer> m_frame_buffer;

    std::unordered_map<uint32_t, std::shared_ptr<VulkanDescriptor>> m_descs;
};