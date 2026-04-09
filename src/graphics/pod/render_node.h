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
class GraphicsRenderNodeConfig;
class VulkanImageBuffer;
class VulkanBuffer;
class VulkanDescriptor;
class VulkanSampler;
class RenderGraph;
class CommandBatch;

class RenderNode : public std::enable_shared_from_this<RenderNode> {
public:
    struct AttachmentSlot {
        std::shared_ptr<RenderResource> resource;
        RenderResource::ResourceName attached_as;
    };

    using GlobalName = RenderResource::ResourceName;
    using LocalName = RenderResource::ResourceName;

    using ResourceMap = std::unordered_map<GlobalName, AttachmentSlot>;
    using AttachMap = std::unordered_map<LocalName, AttachmentSlot>;

    virtual bool init(std::shared_ptr<VulkanDevice> device, const std::string& node_config_name, std::weak_ptr<RenderGraph> render_graph) = 0;
    virtual void destroy() = 0;

    virtual void render(CommandBatch& command_buffer, unsigned image_index) = 0;

    void addReadDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as, bool only_read = true);
    void addWriteDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as);
    void changeWriteDependency(std::shared_ptr<RenderResource> resource, LocalName attached_as);

    bool isReadGlobal(const GlobalName& name) const;
    bool isWrittenGlobal(const GlobalName& name) const;
    bool isReadAttached(const LocalName& name) const;
    bool isWrittenAttached(const LocalName& name) const;

    const ResourceMap& getReadResourcesMap() const;
    const AttachMap& getReadAttachmentMap() const;

    const ResourceMap& getWrittenResourcesMap() const;
    const AttachMap& getWrittenAttachmentMap() const;

    virtual void finishRenderNode() = 0;

    std::shared_ptr<RenderResource> getAttachedResource(const LocalName& attached_as) const;
    std::shared_ptr<VulkanImageBuffer> getAttachedImageResource(const LocalName& attached_as) const;
    std::shared_ptr<VulkanImageBuffer> getWrittenAttachedImageResource(const LocalName& name) const;
    std::shared_ptr<VulkanImageBuffer> getReadAttachedImageResource(const LocalName& name) const;
    std::shared_ptr<VulkanBuffer> getAttachedBufferResource(const LocalName& attached_as) const;
    std::shared_ptr<VulkanBuffer> getWrittenAttachedBufferResource(const LocalName& name) const;
    std::shared_ptr<VulkanBuffer> getReadAttachedBufferResource(const LocalName& name) const;

    virtual void TransitionResourcesToProperState(CommandBatch& command_buffer) = 0;

protected:
    std::shared_ptr<VulkanDevice> m_device;
    std::weak_ptr<RenderGraph> m_render_graph;

private:
    ResourceMap m_read_resources;
    AttachMap m_read_attached;

    ResourceMap m_written_resources;
    AttachMap m_written_attached;
};