#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <memory>

#include "scene_node.h"

class CameraNode : public SceneNode {
public:
	CameraNode(std::string name) : SceneNode(name) {};
	CameraNode(std::string name, glm::mat4x4 camera_transform) : SceneNode(name) {};
	CameraNode(std::string name, glm::mat4x4 camera_transform, glm::mat4x4 proj) : SceneNode(name) {};

	virtual bool VOnRestore() override {};
	virtual bool VOnUpdate() override {};

	void SetProjection(glm::mat4x4 proj) {};

	glm::mat4x4 GetWorldViewProjection(glm::mat4x4 world) const {};
	glm::mat4x4 GetWorldViewProjectionT(glm::mat4x4 world) const {};

	glm::mat4x4 GetViewProjection() const {};
	glm::mat4x4 GetViewProjectionT() const {};

	glm::mat4x4 GetProjection() const {};
	glm::mat4x4 GetProjectionT() const {};

	glm::mat4x4 GetView() const {};
	glm::mat4x4 GetViewT() const {};
};