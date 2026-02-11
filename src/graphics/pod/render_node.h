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
    void addDynamicReadDependency(std::string resource_name);

    void addWriteDependency(std::shared_ptr<RenderResource> resource);
    void addDynamicWriteDependency(std::string resource_name);

    bool isRead(const RenderResource::ResourceName& name) const;
    bool isWritten(const RenderResource::ResourceName& name) const;

    const ResourceMap& getReadResourcesMap() const;
    const std::unordered_set<std::string>& getDynamicReadResourcesSet() const;

    const ResourceMap& getWrittenResourcesMap() const;
    const std::unordered_set<std::string>& getDynamicWrittenResourcesSet() const;

private:
    std::shared_ptr<VulkanPipeline> m_pipeline;

    ResourceMap m_read_resources;
    std::unordered_set<std::string> m_dynamic_read_resources;

    ResourceMap m_written_resources;
    std::unordered_set<std::string> m_dynamic_written_resources;
};