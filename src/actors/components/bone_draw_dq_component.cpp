#include "bone_draw_dq_component.h"

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

const std::string BoneDrawDQComponent::g_name = "BoneDrawDQComponent";

BoneDrawDQComponent::BoneDrawDQComponent() {}

BoneDrawDQComponent::BoneDrawDQComponent(const pugi::xml_node& data) {
    Init(data);
}

BoneDrawDQComponent::~BoneDrawDQComponent() {}

bool BoneDrawDQComponent::VInit(const pugi::xml_node& data) {
    return Init(data);
}

void BoneDrawDQComponent::VDelegatePostInit() {}

const std::string& BoneDrawDQComponent::VGetName() const {
	return BoneDrawDQComponent::g_name;
}

pugi::xml_node BoneDrawDQComponent::VGenerateXml() {
	return pugi::xml_node();
}

std::shared_ptr<SceneNode> BoneDrawDQComponent::VGetSceneNode() {
    return m_loaded_scene_node;
}

const ComponentDependecyList& BoneDrawDQComponent::VGetComponentDependecy() const {
	static const ComponentDependecyList component_dep = {TransformComponent::g_name, ModelComponent::g_name};
    return component_dep;
}

void BoneDrawDQComponent::setLineWidth(float width) {
	if(m_bone_scene_nodes.size() == 0u) return;
	
	for(std::shared_ptr<SceneNode>& scene_node : m_bone_scene_nodes) {
		std::shared_ptr<ValueBagNode> value_bag_node = std::dynamic_pointer_cast<ValueBagNode>(scene_node->GetScene()->getProperty(scene_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_VALUE_BAG));
		if(value_bag_node) {
			value_bag_node->SetValue("u_line_width"s, &m_line_width);
		}		
	}
}

bool BoneDrawDQComponent::Init(const pugi::xml_node& data) {
	using namespace std::literals;

	m_line_width = 4.0f;

    const std::shared_ptr<VulkanShadersManager>& shader_manager = Application::Get().GetRenderer().getShadersManager();

    std::shared_ptr<Actor> act = GetOwner();
	std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>(ActorComponent::GetIdFromName("TransformComponent")).lock();
	std::shared_ptr<ModelComponent> mc = act->GetComponent<ModelComponent>(ActorComponent::GetIdFromName("ModelComponent")).lock();

	const std::shared_ptr<Scene>& scene = tc->GetSceneNode()->GetScene();
    //std::shared_ptr<SceneNode> transform_node = tc->GetSceneNode();
	std::shared_ptr<SceneNode> bone_transform_scene_node = std::make_shared<SceneNode>(scene, "bone_transform"s, glm::mat4(1.0f));
    scene->addProperty(bone_transform_scene_node);

	//const std::shared_ptr<Scene>& scene = transform_node->GetScene();
	scene->recalculateGlobalTransforms();
	const std::shared_ptr<SkeletonManager>& seleton_manager = scene->getSkeletonManager();

    //m_loaded_scene_node = transform_node;

	if(!mc) return !!mc;

	MeshNodeGeometryGenerator geometry_gen;
	std::string skin_name = *seleton_manager->getMeshSkins(mc->VGetSceneNode()).begin();
	std::shared_ptr<SceneNode> new_node = geometry_gen.GenerateBoneLineInstanced(act->GetName() + "_bone_line_dq"s, skin_name, m_line_width, shader_manager, bone_transform_scene_node);
	std::shared_ptr<MeshNode> mesh_node = std::dynamic_pointer_cast<MeshNode>(new_node->GetScene()->getProperty(new_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_MESH));
	m_loaded_scene_node = new_node;

	Application::Get().GetGameLogic()->GetHumanView()->VGetScene()->AddRenderNode(mesh_node);

	m_bone_scene_nodes.push_back(std::move(new_node));
	
	m_initialized = true;

	return !!m_loaded_scene_node;
}