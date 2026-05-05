#include "coord_arrow_component.h"

#include "transform_component.h"
#include "../application.h"
#include "../graphics/vulkan_renderer.h"
#include "../scene/mesh_node_loader.h"

#include <cassert>
#include <unordered_map>

const std::string CoordComponent::g_name = "CoordArrowComponent";

CoordComponent::CoordComponent() {}

CoordComponent::CoordComponent(const pugi::xml_node& data) {
    Init(data);
}

CoordComponent::~CoordComponent() {}

bool CoordComponent::VInit(const pugi::xml_node& data) {
    return Init(data);
}

void CoordComponent::VDelegatePostInit() {}

const std::string& CoordComponent::VGetName() const {
	return CoordComponent::g_name;
}

pugi::xml_node CoordComponent::VGenerateXml() {
	return pugi::xml_node();
}

std::shared_ptr<SceneNode> CoordComponent::VGetSceneNode() {
    return m_loaded_scene_node;
}

const ComponentDependecyList& CoordComponent::VGetComponentDependecy() const {
	static const ComponentDependecyList component_dep = {TransformComponent::g_name};
    return component_dep;
}

bool CoordComponent::Init(const pugi::xml_node& data) {
	m_resource_name = "objects/coord_arrows.gltf";
	if (m_resource_name.empty()) return false;
	std::filesystem::path p(m_resource_name);
	m_resource_directory = p.parent_path().string();

    std::shared_ptr<VulkanShadersManager> shader_manager = Application::Get().GetRenderer().getShadersManager();

    std::shared_ptr<Actor> act = GetOwner();

	std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>(ActorComponent::GetIdFromName("TransformComponent")).lock();
	if (!tc) {
		return false;
	}

    std::shared_ptr<SceneNode> transform_node = tc->GetSceneNode();

    MeshNodeLoader node_loader;
    m_loaded_scene_node = node_loader.ImportSceneNode(p, shader_manager, transform_node);

	return !!m_loaded_scene_node;
}