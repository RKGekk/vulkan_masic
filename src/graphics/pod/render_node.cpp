#include "render_node.h"

#include <utility>

bool RenderNode::init(std::shared_ptr<VulkanPipeline> pipeline) {
    m_pipeline = std::move(pipeline);

    return true;
}

void RenderNode::destroy() {

}

void RenderNode::addReadDependency(std::shared_ptr<RenderResource> resource) {
    m_read_resources[resource->getName()] = std::move(resource);
}

void RenderNode::addWriteDependency(std::shared_ptr<RenderResource> resource) {
    m_written_resources[resource->getName()] = std::move(resource);
}

bool RenderNode::isRead(const RenderResource::ResourceName& name) const {
    return m_read_resources.count(name);
}

bool RenderNode::isWritten(const RenderResource::ResourceName& name) const {
    return m_written_resources.count(name);
}

const RenderNode::ResourceMap& RenderNode::getReadResourcesMap() const {
    return m_read_resources;
}

const RenderNode::ResourceMap& RenderNode::getWrittenResourcesMap() const {
    return m_written_resources;
}