#include "bone_draw_component.h"

#include "transform_component.h"
#include "model_component.h"
#include "../../application.h"
#include "../../graphics/vulkan_renderer.h"
#include "../../scene/mesh_node_loader.h"
#include "../../scene/mesh_node_geometry_generator.h"
#include "../../scene/nodes/value_bag_node.h"
#include "../../scene/skeleton_manager.h"

#include <cassert>
#include <unordered_map>

const std::string BoneDrawComponent::g_name = "BoneDrawComponent";

BoneDrawComponent::BoneDrawComponent() {}

BoneDrawComponent::BoneDrawComponent(const pugi::xml_node& data) {
    Init(data);
}

BoneDrawComponent::~BoneDrawComponent() {}

bool BoneDrawComponent::VInit(const pugi::xml_node& data) {
    return Init(data);
}

void BoneDrawComponent::VDelegatePostInit() {}

const std::string& BoneDrawComponent::VGetName() const {
	return BoneDrawComponent::g_name;
}

pugi::xml_node BoneDrawComponent::VGenerateXml() {
	return pugi::xml_node();
}

std::shared_ptr<SceneNode> BoneDrawComponent::VGetSceneNode() {
    return m_loaded_scene_node;
}

const ComponentDependecyList& BoneDrawComponent::VGetComponentDependecy() const {
	static const ComponentDependecyList component_dep = {TransformComponent::g_name, ModelComponent::g_name};
    return component_dep;
}

void BoneDrawComponent::setLineWidth(float width) {
	if(m_anim_vis_scene_nodes.size() == 0u) return;
	
	for(std::shared_ptr<SceneNode>& scene_node : m_anim_vis_scene_nodes) {
		std::shared_ptr<ValueBagNode> value_bag_node = std::dynamic_pointer_cast<ValueBagNode>(scene_node->GetScene()->getProperty(scene_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_VALUE_BAG));
		if(value_bag_node) {
			value_bag_node->SetValue("u_line_width"s, &m_line_width);
		}		
	}
}

bool BoneDrawComponent::Init(const pugi::xml_node& data) {
	using namespace std::literals;

	m_line_width = 4.0f;
	m_resource_name = "objects/coord_arrows.gltf";
	if (m_resource_name.empty()) return false;
	std::filesystem::path p(m_resource_name);
	m_resource_directory = p.parent_path().string();

    std::shared_ptr<VulkanShadersManager> shader_manager = Application::Get().GetRenderer().getShadersManager();

    std::shared_ptr<Actor> act = GetOwner();
	std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>(ActorComponent::GetIdFromName("TransformComponent")).lock();
	std::shared_ptr<ModelComponent> mc = act->GetComponent<ModelComponent>(ActorComponent::GetIdFromName("ModelComponent")).lock();

    std::shared_ptr<SceneNode> transform_node = tc->GetSceneNode();
	const std::shared_ptr<Scene>& scene = transform_node->GetScene();
	const std::shared_ptr<SkeletonManager>& seleton_manager = scene->getSkeletonManager();

    MeshNodeLoader node_loader;
    m_loaded_scene_node = node_loader.ImportSceneNode(p, shader_manager, transform_node);

	if(!mc) return !!m_loaded_scene_node;

	MeshNodeGeometryGenerator geometry_gen;
	std::shared_ptr<SceneNode> bone_transform_scene_node = std::make_shared<SceneNode>(transform_node->GetScene(), "bone_transform"s, glm::mat4(1.0f));
    transform_node->GetScene()->addProperty(bone_transform_scene_node);
	std::string skin_name = *seleton_manager->getMeshSkins(mc->VGetSceneNode()).begin();
	std::shared_ptr<SceneNode> new_node = geometry_gen.GenerateBoneLine(act->GetName() + "_bone_line"s, skin_name, m_line_width, shader_manager, bone_transform_scene_node);
	std::shared_ptr<MeshNode> mesh_node = std::dynamic_pointer_cast<MeshNode>(new_node->GetScene()->getProperty(new_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_MESH));

	Application::Get().GetGameLogic()->GetHumanView()->VGetScene()->AddRenderNode(mesh_node);

	m_anim_vis_scene_nodes.push_back(std::move(new_node));
	

	return !!m_loaded_scene_node;
}