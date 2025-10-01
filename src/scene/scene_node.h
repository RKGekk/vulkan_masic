#pragma once

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

class SceneNode {
public:
	SceneNode(std::string name) {};
	SceneNode(std::string name, uint32_t group_id) {};
	SceneNode(std::string name, glm::mat4x4 to) {};
	SceneNode(std::string name, uint32_t group_id, glm::mat4x4 to) {};

	virtual ~SceneNode() {};

	virtual bool VOnRestore() {};
	virtual bool VOnUpdate() {};
	virtual bool VOnLostDevice() {};

	virtual bool VAddChild(const SceneNode& kid) {};
	virtual bool VRemoveChild(const SceneNode& cid) {};
	virtual const Scene::NodeIndex VGetChild() const {};
    virtual const Scene::NodeIndex VGetSibling() const {};

	const SceneNodeProperties& Get() const {};

	void UpdateCumulativeTransform() {};

	void SetTransform(glm::mat4x4 to_parent) {};

	void SetTranslation3(glm::vec3 pos) {};
	void SetTranslation4(glm::vec4 pos) {};

	void SetParent(const SceneNode& parent_node) {};
    void SetParent(Scene::NodeIndex parent_node) {};
	const SceneNode& GetParent() {};
    Scene::NodeIndex GetParentIndex() {};

	void SetName(std::string name) {};

	void SetDirtyFlags(uint32_t flags) {};
	void AddDirtyFlags(uint32_t flags) {};
	void RemoveDirtyFlags(uint32_t flags) {};

	void SetGroupID(uint32_t id) {};

protected:
    Scene::NodeIndex m_node_index;
    std::shared_ptr<Scene> m_scene;
	SceneNodeProperties m_props;
};
