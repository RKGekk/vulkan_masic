#include "model_component.h"

#include "transform_component.h"
#include "../application.h"
#include "../scene/mesh_node_loader.h"

#include <cassert>
#include <unordered_map>

const std::string ModelComponent::g_name = "ModelComponent";

ModelComponent::ModelComponent() {}

ModelComponent::ModelComponent(const pugi::xml_node& data) {
    Init(data);
}

ModelComponent::~ModelComponent() {}

bool ModelComponent::VInit(const pugi::xml_node& data) {
    return Init(data);
}

void ModelComponent::VDelegatePostInit() {}

const std::string& ModelComponent::VGetName() const {
	return ModelComponent::g_name;
}

pugi::xml_node ModelComponent::VGenerateXml() {
	return pugi::xml_node();
}

std::shared_ptr<SceneNode> ModelComponent::VGetSceneNode() {
    return m_loaded_scene_node;
}

const ComponentDependecyList& ModelComponent::VGetComponentDependecy() const {
	static const ComponentDependecyList component_dep = {TransformComponent::g_name};
    return component_dep;
}

const std::string& ModelComponent::GetResourceName() {
    return m_resource_name;
}

const std::string& ModelComponent::GetResourceDirecory() {
    return m_resource_directory;
}

bool ModelComponent::Init(const pugi::xml_node& data) {
	m_resource_name = data.child("FilePath").child_value();
	if (m_resource_name.empty()) return false;
	std::filesystem::path p(m_resource_name);
	m_resource_directory = p.parent_path().string();

    VertexFormat vertex_format;
    vertex_format.addVertexAttribute("POSITION", VertexFormat::VertexAttributeFormat::FLOAT_VEC3);
    vertex_format.addVertexAttribute("COLOR_0", VertexFormat::VertexAttributeFormat::FLOAT_VEC3);
    vertex_format.addVertexAttribute("TEXCOORD_0", VertexFormat::VertexAttributeFormat::FLOAT_VEC2);

    ShaderSignature shader_signature;
    shader_signature.setVertexFormat(vertex_format);

    std::shared_ptr<Actor> act = GetOwner();

	std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>(ActorComponent::GetIdFromName("TransformComponent")).lock();
	if (!tc) {
		return false;
	}

    std::shared_ptr<SceneNode> transform_node = tc->GetSceneNode();

    MeshNodeLoader node_loader;
    m_loaded_scene_node = node_loader.ImportSceneNode(p, shader_signature, transform_node);

	return !!m_loaded_scene_node;
}