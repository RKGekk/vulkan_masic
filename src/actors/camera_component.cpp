#include "camera_component.h"

#include "../tools/string_tools.h"
#include "../scene/camera_node.h"
#include "../scene/basic_camera_node.h"
#include "../application.h"
#include "../events/cicadas/evt_data_destroy_scene_component.h"
#include "../events/ievent_manager.h"
#include "transform_component.h"

const std::string CameraComponent::g_name = "CameraComponent";

CameraComponent::CameraComponent() {}

CameraComponent::CameraComponent(const pugi::xml_node& data) {
	Init(data);
}

CameraComponent::~CameraComponent() {}

bool CameraComponent::Init(const pugi::xml_node& data) {
	std::shared_ptr<Actor> act = GetOwner();

	float fov = DirectX::XMConvertToRadians(data.child("Fov").text().as_float(90.0f));
	float near = data.child("Near").text().as_float(0.1f);
	float far = data.child("Far").text().as_float(1.0f);
	float aspect_ratio = Application::Get().GetApplicationOptions().GetAspect();
	std::shared_ptr<Scene> scene_ptr = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();
    Scene::NodeIndex node_index = scene_ptr->addNode();
    std::shared_ptr<Actor> act = GetOwner();
	std::string name = act->GetName();
	std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>(ActorComponent::GetIdFromName("TransformComponent")).lock();
	Scene::NodeIndex node_idx = tc->GetSceneNodeIndex();

	m_camera_node = std::make_shared<BasicCameraNode>(std::move(scene_ptr), node_idx, std::move(name), fov, aspect_ratio, near, far);
}



const std::string& CameraComponent::VGetName() const {
	return CameraComponent::g_name;
}

const ComponentDependecyList& CameraComponent::VGetComponentDependecy() const {
	static const ComponentDependecyList component_dep = {TransformComponent::g_name};
    return component_dep;
}

pugi::xml_node CameraComponent::VGenerateXml() {
	return pugi::xml_node();
}

std::shared_ptr<BasicCameraNode> CameraComponent::VGetCameraNode() {
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