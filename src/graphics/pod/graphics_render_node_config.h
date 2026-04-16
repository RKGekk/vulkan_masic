#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "render_resource.h"
#include "framebuffer_config.h"

class VulkanDevice;
class VulkanSampler;
class VulkanResourcesManager;
class VulkanPipelinesManager;
class VulkanPipeline;
class WindowSurface;
class SwapchainSupportDetails;

class GraphicsRenderNodeConfig {
public:
    enum class ExtentSource { AUTO, AS_SWAPCHAIN, EXACT };
    enum class IndexCountType { ALL, EXACT };

    struct UpdateMetadata {
        std::string name;
        enum class SamplerType { NONE, INLINE, DEFAULT, FROM_IMAGEBUFFER };

        RenderResource::Type resource_type;
        std::string buffer_resource_type_name;

        SamplerType sampler_type;
        std::shared_ptr<VulkanSampler> image_sampler;
        std::string image_resource_type_name;
        std::string image_view_type_name;
        VkImageLayout read_image_layout;
    };

    bool init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<WindowSurface>& window, const SwapchainSupportDetails& swapchain_support_details, const std::shared_ptr<VulkanResourcesManager>& resources_manager, const std::shared_ptr<VulkanPipelinesManager>& pipelines_manager, const pugi::xml_node& node_data);

    const std::string& getName() const;
    const std::shared_ptr<VulkanPipeline>& getPipeline() const;

    VkViewport getViewport() const;
    const VkViewport* getViewportPtr() const;
    ExtentSource getViewportSource() const;
    void setViewport(VkViewport viewport);

    VkRect2D getScissor() const;
    const VkRect2D* getScissorPtr() const;
    ExtentSource getScissorSource() const;
    void setScissor(VkRect2D scissor);

    IndexCountType getIndexCountType() const;
    uint32_t getIndexCount() const;
    void setIndexCount(uint32_t index_count);
    uint32_t getFirstIndex() const;
    void setFirstIndex(uint32_t first_index);
    int32_t getVertexOffset() const;
    void setVertexOffset(int32_t vertex_offset);

    const std::shared_ptr<FramebufferConfig>& getFramebufferConfig() const;
    const std::vector<std::shared_ptr<FramebufferConfig::FrameBufferAttachment>>& getAttachmentsConfig() const;
    const std::shared_ptr<FramebufferConfig::FrameBufferAttachment>& getAttachmentData(const std::string& attached_name) const;

    const std::unordered_map<std::string, std::shared_ptr<UpdateMetadata>>& getBindingsMetadata() const;
    const std::shared_ptr<UpdateMetadata>& getUpdateMetadata(const std::string& binding_name);

    std::shared_ptr<GraphicsRenderNodeConfig> makeInstance(std::string name) const;

private:
    std::string m_name;
    std::shared_ptr<FramebufferConfig> m_framebuffer_config;
    std::shared_ptr<VulkanPipeline> m_pipeline;

    VkViewport m_viewport;
    ExtentSource m_viewport_source;
    VkRect2D m_scissor;
    ExtentSource m_scissor_source;

    IndexCountType m_index_count_type;
    uint32_t m_index_count;
    uint32_t m_first_index;
    int32_t m_vertex_offset;

    std::unordered_map<std::string, std::shared_ptr<UpdateMetadata>> m_bindings_metadata;
};