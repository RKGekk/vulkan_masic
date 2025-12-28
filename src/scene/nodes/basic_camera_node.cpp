#include "basic_camera_node.h"

#include "../../application.h"
#include "../../tools/memory_utility.h"

BasicCameraNode::BasicCameraNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& camera_transform, float fovy, float aspect, float near_clip, float far_clip, Scene::NodeIndex parent) 
	: CameraNode(std::move(scene), std::move(name), camera_transform, parent)
	, m_fovy(fovy)
	, m_aspect(aspect) {
	SetData(Get().ToParent(), glm::perspective(fovy, aspect, near_clip, far_clip));
}

BasicCameraNode::BasicCameraNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& camera_transform, const glm::mat4x4& proj, Scene::NodeIndex parent)
	: CameraNode(std::move(scene), std::move(name), camera_transform, parent) {
	SetData(Get().ToParent(), proj);
}

BasicCameraNode::BasicCameraNode(std::shared_ptr<Scene> scene, const std::string& name, const BoundingFrustum& frustum, Scene::NodeIndex parent) 
	: CameraNode(std::move(scene), std::move(name), parent) {
	SetData(frustum);
}

BasicCameraNode::BasicCameraNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index, float fovy, float aspect, float near_clip, float far_clip)
	: CameraNode(std::move(scene), node_index)
	, m_fovy(fovy)
	, m_aspect(aspect) {
	
	SetData(Get().ToParent(), glm::perspective(fovy, aspect, near_clip, far_clip));
}

BasicCameraNode::BasicCameraNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index, const glm::mat4x4& proj) 
	: CameraNode(std::move(scene), node_index) {
	SetData(Get().ToParent(), proj);
}

BasicCameraNode::BasicCameraNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index, const BoundingFrustum& frustum)
	: CameraNode(std::move(scene), node_index) {
	SetData(frustum);
}

bool BasicCameraNode::VOnRestore() {
	float new_aspect = Application::Get().GetApplicationOptions().GetAspect();
	if (m_aspect == new_aspect) return true;

	m_aspect = new_aspect;
	SetData(m_props.ToParent(), glm::perspective(m_fovy, m_aspect, m_frustum.Near, m_frustum.Far));

	return CameraNode::VOnRestore();
}

bool BasicCameraNode::VOnUpdate() {
	UpdateFrustum();
	return CameraNode::VOnUpdate();
}

const BoundingFrustum& BasicCameraNode::GetFrustum() const {
	return m_frustum;
}

void BasicCameraNode::UpdateFrustum() {
	SetData(Get().ToParent(), m_projection);
}

void BasicCameraNode::SetFovYRad(float fovy) {
	if (m_fovy == fovy) return;

	m_fovy = fovy;
	SetData(m_props.ToParent(), glm::perspective(m_fovy, m_aspect, m_frustum.Near, m_frustum.Far));
}

void BasicCameraNode::SetFovYDeg(float fovy) {
	SetFovYRad(glm::radians(fovy));
}

float BasicCameraNode::GetFovYRad() const {
	return m_fovy;
}

float BasicCameraNode::GetFovYDeg() const {
	return glm::radians(m_fovy);
}

void BasicCameraNode::SetNear(float near_cut) {
	if (m_frustum.Near == near_cut) return;

	m_frustum.Near = near_cut;
	m_projection = glm::perspective(m_fovy, m_aspect, m_frustum.Near, m_frustum.Far);
}

float BasicCameraNode::GetNear() const {
	return m_frustum.Near;
}

float BasicCameraNode::GetFar() const {
	return m_frustum.Far;
}

void BasicCameraNode::SetFar(float far_cut) {
	if (m_frustum.Far == far_cut) return;

	m_frustum.Far = far_cut;
	m_projection = glm::perspective(m_fovy, m_aspect, m_frustum.Near, m_frustum.Far);
}

void BasicCameraNode::SetProjection(const BoundingFrustum& frustum) {
	SetData(frustum);
}

void BasicCameraNode::SetProjection(float fovy, float aspect, float near_clip, float far_clip) {
	SetData(Get().ToParent(), glm::perspective(fovy, aspect, near_clip, far_clip));
}

void BasicCameraNode::SetData(const glm::mat4x4& camera_transform, const glm::mat4x4& proj) {
	BoundingFrustum frustum(proj);
	frustum.Origin = glm::vec3(camera_transform[3].x, camera_transform[3].y, camera_transform[3].z);
	frustum.Orientation = frustum.Orientation * glm::quat(camera_transform);
	SetData(frustum);
}

void BasicCameraNode::SetData(const BoundingFrustum& frustum) {
	glm::mat4x4 view_rot = glm::mat4x4(frustum.Orientation);
	SetTransform(view_rot);
	SetTranslation3(frustum.Origin);

	m_fovy = 2.0f * atanf(frustum.TopSlope);
	m_aspect = frustum.RightSlope / frustum.TopSlope;

    m_projection = glm::perspective(m_fovy, m_aspect, frustum.Near, frustum.Far);
    m_projection[1].y *= -1.0f;
	m_frustum = frustum;
}