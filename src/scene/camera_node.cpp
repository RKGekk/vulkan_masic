#include "camera_node.h"

CameraNode::CameraNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index) : SceneNode(std::move(scene), node_index) {}

CameraNode::CameraNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent) 
    : SceneNode(std::move(scene), std::move(name), parent)
    , m_projection(glm::mat4(1.0f)) {}

CameraNode::CameraNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 camera_transform, Scene::NodeIndex parent)
    : SceneNode(std::move(scene), std::move(name), camera_transform, parent)
    , m_projection(glm::mat4(1.0f)) {}

CameraNode::CameraNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 camera_transform, glm::mat4x4 proj, Scene::NodeIndex parent)
    : SceneNode(std::move(scene), std::move(name), camera_transform, parent)
    , m_projection(proj) {}

bool CameraNode::VOnRestore() {
    return SceneNode::VOnRestore();
}

bool CameraNode::VOnUpdate() {
    return SceneNode::VOnUpdate();
}

void CameraNode::SetProjection(const glm::mat4x4& proj) {
    m_projection = proj;
}

glm::mat4x4 CameraNode::GetWorldViewProjection(const glm::mat4x4& world) const {
    glm::mat4x4 view = Get().FromRoot();
	glm::mat4x4 world_view = world * view;
	return world_view * m_projection;
}

glm::mat4x4 CameraNode::GetWorldViewProjectionT(const glm::mat4x4& world) const {
	return glm::transpose(GetWorldViewProjection(world));
}

glm::mat4x4 CameraNode::GetViewProjection() const {
    glm::mat4x4 view = Get().FromRoot();
	return view * m_projection;
}

glm::mat4x4 CameraNode::GetViewProjectionT() const {
    return glm::transpose(GetViewProjection());
}

const glm::mat4x4& CameraNode::GetProjection() const {
    return m_projection;
}

glm::mat4x4 CameraNode::GetProjectionT() const {
    return glm::transpose(m_projection);
}

glm::mat4x4 CameraNode::GetView() const {
    return Get().FromRoot();
}

glm::mat4x4 CameraNode::GetViewT() const {
    return Get().FromRootT();
}