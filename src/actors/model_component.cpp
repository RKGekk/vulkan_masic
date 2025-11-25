#include "model_component.h"

#include "transform_component.h"
#include "../application.h"

#include <cassert>
#include <unordered_map>

const std::string ModelComponent::g_name = "ModelComponent";

std::shared_ptr<SceneNode> ImportSceneNode(const std::filesystem::path& model_path) {
    Application& app = Application::Get();
    VulkanRenderer& renderer = app.GetRenderer();
    std::shared_ptr<VulkanDevice> device = renderer.GetDevice();
    tinygltf::Model gltf_model;
    tinygltf::TinyGLTF gltf_ctx;

    bool store_original_json_for_extras_and_extensions = true;
    gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(store_original_json_for_extras_and_extensions);

    bool load_result = false;
    std::string ext = model_path.extension().string().c_str();
    std::string load_error;
    std::string load_warning;
    if (ext.compare(".glb") == 0) {
    	load_result = gltf_ctx.LoadBinaryFromFile(&gltf_model, &load_error, &load_warning, model_path.string().c_str());
    }
    else {
    	load_result = gltf_ctx.LoadASCIIFromFile(&gltf_model, &load_error, &load_warning, model_path.string().c_str());
    }

    if (!load_result || gltf_model.scenes.empty()) return nullptr;
    
    
}

ModelComponent::ModelComponent() {}

ModelComponent::ModelComponent(const pugi::xml_node& data) {
    Init(data);
}

ModelComponent::~ModelComponent() {}

void ModelComponent::VDelegatePostInit() {
    using namespace std::literals;
    std::shared_ptr<Actor> act = GetOwner();
    std::string name = act->GetName() + "-MeshComponent"s;
    std::shared_ptr<SceneNode> scene_node = VGetSceneNode();
    scene_node->SetName(name);
}

const std::string& ModelComponent::VGetName() const {
	return ModelComponent::g_name;
}

pugi::xml_node ModelComponent::VGenerateXml() {
	return pugi::xml_node();
}

const std::shared_ptr<SceneNode>& ModelComponent::VGetSceneNode() {
    return m_loaded_scene_node;
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
	return LoadModel(p);
}

bool ModelComponent::LoadModel(const std::filesystem::path& file_name) {
    std::string file_path_str = file_name.string();

    m_loaded_scene_node = ImportSceneNode(file_name);
    
    return true;
}