#include "aabb_node.h"

AABBNode::AABBNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index) : SceneNode(std::move(scene), node_index) {}

AABBNode::AABBNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent) : SceneNode(std::move(scene), std::move(name), parent) {}

AABBNode::AABBNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent) : SceneNode(std::move(scene), std::move(name), transform, parent) {}

AABBNode::AABBNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, BoundingBox aabb, Scene::NodeIndex parent)
    : SceneNode(std::move(scene), std::move(name), transform, parent)
    , m_aabb(aabb) {}

AABBNode::AABBNode(std::shared_ptr<Scene> scene, std::string name, BoundingBox aabb, Scene::NodeIndex parent)
    : SceneNode(std::move(scene), std::move(name), parent)
    , m_aabb(aabb) {}

bool AABBNode::VOnRestore() {
    return true;
}

bool AABBNode::VOnUpdate() {
    return true;
}

void AABBNode::setAABB(const BoundingBox& aabb) {
    m_aabb = aabb;
}

const BoundingBox& AABBNode::getAABB() const {
    return m_aabb;
}