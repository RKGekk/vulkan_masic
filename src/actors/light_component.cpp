#include "light_component.h"

#include "transform_component.h"
#include "../application.h"
#include "../graphics/vulkan_renderer.h"
#include "../scene/mesh_node_loader.h"
#include "../tools/string_tools.h"

#include <cassert>
#include <unordered_map>

const std::string LightComponent::g_name = "LightComponent";

LightComponent::LightComponent() {}

LightComponent::LightComponent(const pugi::xml_node& data) {
    Init(data);
}

LightComponent::~LightComponent() {}

bool LightComponent::VInit(const pugi::xml_node& data) {
    return Init(data);
}

void LightComponent::VDelegatePostInit() {}

const std::string& LightComponent::VGetName() const {
	return LightComponent::g_name;
}

pugi::xml_node LightComponent::VGenerateXml() {
	return pugi::xml_node();
}

const std::shared_ptr<SceneNode>& LightComponent::VGetSceneNode() {
    return m_loaded_scene_node;
}

const ComponentDependecyList& LightComponent::VGetComponentDependecy() const {
	static const ComponentDependecyList component_dep = {TransformComponent::g_name};
    return component_dep;
}

LightNode::LightType LightComponent::GetLightType(const std::string& light_type_string) {
	LightNode::LightType res = LightNode::LightType::DIRECTIONAL;
	if(light_type_string == "PointLight") {
		res = LightNode::LightType::POINT;
	}
	else if(light_type_string == "SpotLight") {
		res = LightNode::LightType::SPOT;
	}

	return res;
}

bool LightComponent::Init(const pugi::xml_node& data) {
	pugi::xml_node light_node_data = data.child("Light");
	if (!light_node_data) return false;

    std::shared_ptr<Actor> act = GetOwner();
	std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>(ActorComponent::GetIdFromName("TransformComponent")).lock();
	if (!tc) {
		return false;
	}
    std::shared_ptr<SceneNode> transform_node = tc->GetSceneNode();
    const std::shared_ptr<Scene>& scene = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();

	std::shared_ptr<LightNode> light_node = std::make_shared<LightNode>(scene, transform_node->VGetNodeIndex());
	scene->addProperty(light_node);
    m_loaded_scene_node = light_node;

	std::string light_type_string = light_node_data.attribute("type").as_string();
    LightNode::LightType light_type = GetLightType(light_type_string);
	light_node->setLightType(light_type);

	LightNodeProperties props;
    glm::vec3 default_strength = { 1.0f, 1.0f, 1.0f };
	props.strength = colorfromattr3f(light_node_data.child("strength"), default_strength);
	props.falloff_start = light_node_data.child("falloff_start").text().as_float();
	props.falloff_end = light_node_data.child("falloff_end").text().as_float();
	props.spot_power = light_node_data.child("spot_power").text().as_float();
	props.outer_angle = glm::radians(light_node_data.child("outer_angle").text().as_float());
	props.inner_angle = glm::radians(light_node_data.child("inner_angle").text().as_float());
	
	light_node->SetLightProperties(props);

	return !!m_loaded_scene_node;
}