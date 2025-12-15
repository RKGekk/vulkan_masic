#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#include "scene_node.h"
#include "camera_node.h"
#include "../../physics/bounding_frustum.h"

class BasicCameraNode : public CameraNode {
public:
	BasicCameraNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& camera_transform, float fovy, float aspect, float near_clip, float far_clip, Scene::NodeIndex parent = 0u);
	BasicCameraNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& camera_transform, const glm::mat4x4& proj, Scene::NodeIndex parent = 0u);
	BasicCameraNode(std::shared_ptr<Scene> scene, const std::string& name, const BoundingFrustum& frustum, Scene::NodeIndex parent = 0u);
	BasicCameraNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index, float fovy, float aspect, float near_clip, float far_clip);
	BasicCameraNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index, const glm::mat4x4& proj);
	BasicCameraNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index, const BoundingFrustum& frustum);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

	const BoundingFrustum& GetFrustum() const;
	void UpdateFrustum();

	void SetFovYRad(float fovy);
	void SetFovYDeg(float fovy);

	float GetFovYRad() const;
	float GetFovYDeg() const;

	void SetNear(float near_cut);
	void SetFar(float far_cut);

	float GetNear() const;
	float GetFar() const;

	void SetProjection(const BoundingFrustum& frustum);
	void SetProjection(float fovy, float aspect, float near_clip, float far_clip);

protected:
	void SetData(const glm::mat4x4& camera_transform, const glm::mat4x4& proj);
	void SetData(const BoundingFrustum& frustum);

private:
	BoundingFrustum m_frustum;
	float m_fovy;
	float m_aspect;
};