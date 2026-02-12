#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "render_resource.h"
#include "../api/vulkan_pipeline.h"

class RenderNode {
public:
    using ResourceMap = std::unordered_map<RenderResource::ResourceName, std::shared_ptr<RenderResource>>;

    bool init(std::shared_ptr<VulkanPipeline> pipeline);
    void destroy();

    void addReadDependency(std::shared_ptr<RenderResource> resource);
    void addWriteDependency(std::shared_ptr<RenderResource> resource);

    bool isRead(const RenderResource::ResourceName& name) const;
    bool isWritten(const RenderResource::ResourceName& name) const;

    const ResourceMap& getReadResourcesMap() const;
    const ResourceMap& getWrittenResourcesMap() const;

private:
    std::shared_ptr<VulkanPipeline> m_pipeline;

    ResourceMap m_read_resources;
    ResourceMap m_written_resources;
};