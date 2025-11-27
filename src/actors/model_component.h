#pragma once

#include <filesystem>
#include <string>
#include <memory>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "tiny_gltf.h"
#include <pugixml.hpp>

#include "actor_component.h"
#include "base_scene_node_component.h"
#include "../scene/nodes/mesh_node.h"

class ModelComponent : public BaseSceneNodeComponent {
public:
	static const std::string g_name;

	ModelComponent();
	ModelComponent(const pugi::xml_node& data);
	virtual ~ModelComponent();

	virtual const std::string& VGetName() const override;
	virtual pugi::xml_node VGenerateXml() override;

    virtual const std::shared_ptr<SceneNode>& VGetSceneNode() override;

	const std::string& GetResourceName();
	const std::string& GetResourceDirecory();

protected:
	virtual void VDelegatePostInit() override;

private:
    bool Init(const pugi::xml_node& data);

	std::string m_resource_name;
	std::string m_resource_directory;

	std::shared_ptr<SceneNode> m_loaded_scene_node;
};