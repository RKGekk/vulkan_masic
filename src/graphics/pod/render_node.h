#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <pugixml.hpp>

#include "render_resource.h"

class VulkanDevice;
class VulkanPipeline;
class RenderNodeConfig;
class VulkanImageBuffer;
class VulkanDescriptor;
class VulkanSampler;

class RenderNode {
public:
    struct AttachmentSlot {
        std::shared_ptr<RenderResource> resource;
        RenderResource::ResourceName attached_as;
    };

    using GlobalName = RenderResource::ResourceName;
    using LocalName = RenderResource::ResourceName;

    using ResourceMap = std::unordered_map<GlobalName, AttachmentSlot>;
    using AttachMap = std::unordered_map<LocalName, AttachmentSlot>;

    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<RenderNodeConfig> node_config);
    void destroy();

    void addReadDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as, bool only_read = true);
    void addWriteDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as);

    bool isReadGlobal(const GlobalName& name) const;
    bool isWrittenGlobal(const GlobalName& name) const;
    bool isReadAttached(const LocalName& name) const;
    bool isWrittenAttached(const LocalName& name) const;

    const ResourceMap& getReadResourcesMap() const;
    const AttachMap& getReadAttachmentMap() const;

    const ResourceMap& getWrittenResourcesMap() const;
    const AttachMap& getWrittenAttachmentMap() const;

    void finishRenderNode();

    const std::shared_ptr<VulkanPipeline>& getPipeline();
    VkFramebuffer getFramebuffer() const;
    VkExtent2D getViewportExtent() const;

    std::shared_ptr<RenderResource> getAttachedResource(const LocalName& attached_as) const;
    std::shared_ptr<VulkanImageBuffer> getAttachedImageResource(const LocalName& attached_as);
    std::shared_ptr<VulkanImageBuffer> getWrittenAttachedImageResource(const LocalName& name);
    std::shared_ptr<VulkanImageBuffer> getReadAttachedImageResource(const LocalName& name);

private:
    std::vector<VkImageView> getAttachments() const;

    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanPipeline> m_pipeline;

    std::shared_ptr<RenderNodeConfig> m_node_config;

    std::vector<VkImageView> m_framebuffers_attachments;
    VkFramebufferCreateInfo m_framebuffer_info;
    VkFramebuffer m_frame_buffer;
    VkExtent2D m_viewport_extent;

    std::unordered_map<uint32_t, std::shared_ptr<VulkanDescriptor>> m_descs;

    ResourceMap m_read_resources;
    AttachMap m_read_attached;

    ResourceMap m_written_resources;
    AttachMap m_written_attached;
};