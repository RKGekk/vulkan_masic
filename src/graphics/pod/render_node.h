#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "render_resource.h"
#include "../api/vulkan_pipeline.h"

class VulkanDevice;

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

    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<VulkanPipeline> pipeline);
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
    VkFramebuffer getFramebuffer(uint32_t frame_index) const;
    VkExtent2D getViewportExtent() const;

    const std::shared_ptr<RenderResource>& getAttachedResource(LocalName attached_as, uint32_t frame_index = -1) const;

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanPipeline> m_pipeline;

    uint32_t m_max_frames;

    std::vector<VkFramebuffer> m_frame_buffers;
    VkExtent2D m_viewport_extent;

    ResourceMap m_read_resources;
    AttachMap m_read_attached;

    ResourceMap m_written_resources;
    ResourceMap m_written_attached;
};