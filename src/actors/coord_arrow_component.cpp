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

const std::shared_ptr<SceneNode>& CoordComponent::VGetSceneNode() {
    return m_loaded_scene_node;
}

const ComponentDependecyList& CoordComponent::VGetComponentDependecy() const {
	static const ComponentDependecyList component_dep = {TransformComponent::g_name, TransformAnimationComponent::g_name};
    return component_dep;
}

void CoordComponent::setLineWidth(float width) {
	if(m_anim_vis_scene_nodes.size() == 0u) return;
	
	for(std::shared_ptr<SceneNode>& scene_node : m_anim_vis_scene_nodes) {
		std::shared_ptr<ValueBagNode> value_bag_node = std::dynamic_pointer_cast<ValueBagNode>(scene_node->GetScene()->getProperty(scene_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_VALUE_BAG));
		if(value_bag_node) {
			value_bag_node->SetValue("u_line_width"s, &m_line_width);
		}		
	}
}

bool CoordComponent::Init(const pugi::xml_node& data) {
	using namespace std::literals;

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
		if(!anim->TranslationKeyframes.size()) continue;

		std::shared_ptr<SceneNode> spline_transform_scene_node = std::make_shared<SceneNode>(transform_node->GetScene(), "spline_transform"s, glm::mat4(1.0f));
    	transform_node->GetScene()->addProperty(spline_transform_scene_node);
		std::shared_ptr<SceneNode> new_node = geometry_gen.GenerateSceneNodeSpline(act->GetName() + "_anim_spline"s, m_line_width, anim->TranslationKeyframes, 16u, shader_manager, spline_transform_scene_node);
		std::shared_ptr<MeshNode> mesh_node = std::dynamic_pointer_cast<MeshNode>(new_node->GetScene()->getProperty(new_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_MESH));

		Application::Get().GetGameLogic()->GetHumanView()->VGetScene()->AddRenderNode(mesh_node);

		m_anim_vis_scene_nodes.push_back(std::move(new_node));
	}

	return !!m_loaded_scene_node;
}