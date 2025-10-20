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

#include <memory>
#include <string>

#include "scene.h"

class SceneNode;

class SceneNodeProperties {
public:
	friend SceneNode;

	SceneNodeProperties(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);

	const glm::mat4x4& ToParent() const;
	glm::mat4x4 ToParentT() const;

	const glm::mat4x4& ToRoot() const;
	glm::mat4x4 ToRootT() const;

	const glm::vec4& ToParentTranslation4() const;
	glm::vec3 ToParentTranslation3() const;

	const glm::vec4& ToRootTranslation4() const;
	glm::vec3 ToRootTranslation3() const;

	glm::vec3 ToParentDirection() const;
	glm::vec3 ToParentUp() const;

	glm::vec3 ToRootDirection() const;
	glm::vec3 ToRootUp() const;

	glm::mat4x4 FromParent() const;
	glm::mat4x4 FromParentT() const;

	glm::mat4x4 FromRoot() const;
	glm::mat4x4 FromRootT() const;

	const char* NameCstr() const;
	const std::string& Name() const;

	Scene::NodeType GetNodeType() const;

private:
	std::shared_ptr<Scene> m_scene;
	Scene::NodeIndex m_node_index;
	Scene::NodeType m_node_type;
};