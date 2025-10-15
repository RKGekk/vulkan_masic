#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#include "scene_node.h"

class CameraNode : public SceneNode {
public:
	CameraNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	CameraNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u);
	CameraNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 camera_transform, Scene::NodeIndex parent = 0u);
	CameraNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 camera_transform, glm::mat4x4 proj, Scene::NodeIndex parent = 0u);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

	void SetProjection(const glm::mat4x4& proj);

	glm::mat4x4 GetWorldViewProjection(const glm::mat4x4& world) const;
	glm::mat4x4 GetWorldViewProjectionT(const glm::mat4x4& world) const;

	glm::mat4x4 GetViewProjection() const;
	glm::mat4x4 GetViewProjectionT() const;

	const glm::mat4x4& GetProjection() const;
	glm::mat4x4 GetProjectionT() const;

	glm::mat4x4 GetView() const;
	glm::mat4x4 GetViewT() const;

protected:
	glm::mat4x4 m_projection;
};