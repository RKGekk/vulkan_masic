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

void RenderNode::addDynamicReadDependency(std::string resource_name) {
    m_dynamic_read_resources.insert(std::move(resource_name));
}

void RenderNode::addWriteDependency(std::shared_ptr<RenderResource> resource) {
    m_written_resources[resource->getName()] = std::move(resource);
}

void RenderNode::addDynamicWriteDependency(std::string resource_name) {
    m_dynamic_written_resources.insert(std::move(resource_name));
}

bool RenderNode::isRead(const RenderResource::ResourceName& name) const {
    return m_read_resources.count(name) || m_dynamic_read_resources.count(name);
}

bool RenderNode::isWritten(const RenderResource::ResourceName& name) const {
    return m_written_resources.count(name) || m_dynamic_written_resources.count(name);
}

const RenderNode::ResourceMap& RenderNode::getReadResourcesMap() const {
    return m_read_resources;
}

const std::unordered_set<std::string>& RenderNode::getDynamicReadResourcesSet() const {
    return m_dynamic_read_resources;
}

const RenderNode::ResourceMap& RenderNode::getWrittenResourcesMap() const {
    return m_written_resources;
}

const std::unordered_set<std::string>& RenderNode::getDynamicWrittenResourcesSet() const {
    return m_dynamic_written_resources;
}