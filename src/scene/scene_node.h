#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <stdint.h>
#include <string>

#include "scene.h"
#include "scene_node_properties.h"

class SceneNode : public std::enable_shared_from_this<SceneNode> {
public:
	SceneNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	SceneNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u);
	SceneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 to, Scene::NodeIndex parent = 0u);

	virtual ~SceneNode();

	virtual bool VOnRestore();
	virtual bool VOnUpdate();
	virtual bool VOnLostDevice();

	virtual Scene::NodeIndex VGetChild() const;
    virtual Scene::NodeIndex VGetSibling() const;
	virtual Scene::NodeIndex VGetNodeIndex() const;
	virtual Scene::NodeIndex GetParentIndex() const;

	const SceneNode& GetParent();
	const SceneNodeProperties& Get() const;

	void SetTransform(const glm::mat4x4& to_parent);

	void SetTranslation3(const glm::vec3& pos);
	void SetTranslation4(const glm::vec4& pos);

	void SetName(std::string name);
	void SetNodeType(uint32_t flags);

protected:
    std::shared_ptr<Scene> m_scene;
	Scene::NodeIndex m_node_index;
	SceneNodeProperties m_props;
};
