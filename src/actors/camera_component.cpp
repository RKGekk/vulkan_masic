#include "camera_component.h"

#include "../tools/string_tools.h"
#include "../scene/nodes/camera_node.h"
#include "../scene/nodes/basic_camera_node.h"
#include "../application.h"
#include "transform_component.h"

const std::string CameraComponent::g_name = "CameraComponent";

CameraComponent::CameraComponent() {
	std::shared_ptr<Scene> scene_ptr = Application::Get().GetGameLogic()->VGetScene();

	m_camera_node = std::make_shared<BasicCameraNode>(scene_ptr, g_name, glm::mat4(1.0f),  glm::radians(90.0f), 16.0f/9.0f, 0.1f, 1.0f);
	scene_ptr->addProperty(m_camera_node->VGetNodeIndex(), m_camera_node);
}

CameraComponent::CameraComponent(const pugi::xml_node& data) {
	std::shared_ptr<Scene> scene_ptr = Application::Get().GetGameLogic()->VGetScene();

	m_camera_node = std::make_shared<BasicCameraNode>(scene_ptr, g_name, glm::mat4(1.0f),  glm::radians(90.0f), 16.0f/9.0f, 0.1f, 1.0f);
	scene_ptr->addProperty(m_camera_node->VGetNodeIndex(), m_camera_node);

	Init(data);
}

CameraComponent::~CameraComponent() {}

bool CameraComponent::Init(const pugi::xml_node& data) {
	std::shared_ptr<Actor> act = GetOwner();

	float fov =  glm::radians(data.child("Fov").text().as_float(90.0f));
	float near = data.child("Near").text().as_float(0.1f);
	float far = data.child("Far").text().as_float(1.0f);
	float aspect_ratio = Application::Get().GetApplicationOptions().GetAspect();

	std::shared_ptr<Scene> scene_ptr = Application::Get().GetGameLogic()->VGetScene();

	m_camera_node->SetProjection(fov, aspect_ratio, near, far);

	return true;
}

const std::string& CameraComponent::VGetName() const {
	return CameraComponent::g_name;
}

bool CameraComponent::VInit(const pugi::xml_node& pData) {
    return Init(pData);
}

const ComponentDependecyList& CameraComponent::VGetComponentDependecy() const {
	static const ComponentDependecyList component_dep = {TransformComponent::g_name};
    return component_dep;
}

pugi::xml_node CameraComponent::VGenerateXml() {
	return pugi::xml_node();
}

const std::shared_ptr<BasicCameraNode>& CameraComponent::VGetCameraNode() {
	return m_camera_node;
}

const std::shared_ptr<SceneNode>& CameraComponent::VGetSceneNode() {
	return m_camera_node;
}

float CameraComponent::GetFov() {
	return m_camera_node->GetFovYRad();
}

void CameraComponent::SetFov(float fov) {
	m_camera_node->SetFovYRad(fov);
}

float CameraComponent::GetNear() {
	return m_camera_node->GetNear();
}

void CameraComponent::SetNear(float near_cut) {
	m_camera_node->SetNear(near_cut);
}

float CameraComponent::GetFar() {
	return m_camera_node->GetFar();
}

void CameraComponent::SetFar(float far_cut) {
	m_camera_node->SetFar(far_cut);
}