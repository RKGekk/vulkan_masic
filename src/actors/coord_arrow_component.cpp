#include "coord_arrow_component.h"

#include "transform_component.h"
#include "transform_animation_component.h"
#include "../application.h"
#include "../graphics/vulkan_renderer.h"
#include "../scene/mesh_node_loader.h"
#include "../scene/mesh_node_geometry_generator.h"
#include "../scene/nodes/value_bag_node.h"

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
	static const ComponentDependecyList component_dep = {TransformComponent::g_name, TransformAnimationComponent::g_name};
    return component_dep;
}

void CoordComponent::setLineWidth(float width) {
	if(m_loaded_scene_node) {
		std::shared_ptr<ValueBagNode> value_bag_node = std::dynamic_pointer_cast<ValueBagNode>(m_loaded_scene_node->GetScene()->getProperty(m_loaded_scene_node->GetChild()->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_VALUE_BAG));
		if(value_bag_node) {
			value_bag_node->SetValue("u_line_width"s, &m_line_width);
		}
	}
}

bool CoordComponent::Init(const pugi::xml_node& data) {
	m_line_width = 4.0f;
	m_resource_name = "objects/coord_arrows.gltf";
	if (m_resource_name.empty()) return false;
	std::filesystem::path p(m_resource_name);
	m_resource_directory = p.parent_path().string();

    std::shared_ptr<VulkanShadersManager> shader_manager = Application::Get().GetRenderer().getShadersManager();

    std::shared_ptr<Actor> act = GetOwner();
	std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>(ActorComponent::GetIdFromName("TransformComponent")).lock();
	std::shared_ptr<TransformAnimationComponent> ac = act->GetComponent<TransformAnimationComponent>(ActorComponent::GetIdFromName("TransformAnimationComponent")).lock();

    std::shared_ptr<SceneNode> transform_node = tc->GetSceneNode();

    MeshNodeLoader node_loader;
    m_loaded_scene_node = node_loader.ImportSceneNode(p, shader_manager, transform_node);

	if(!ac) return !!m_loaded_scene_node;

	MeshNodeGeometryGenerator geometry_gen;
	for(const auto&[anim_name, anim] : ac->GetAnimationMap()) {
		m_loaded_scene_node = geometry_gen.GenerateSceneNodeSpline(act->GetName(), m_line_width, anim->TranslationKeyframes, 16u, shader_manager, transform_node);
	}


	return !!m_loaded_scene_node;
}