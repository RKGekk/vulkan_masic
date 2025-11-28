#pragma once

#define GLM_ENABLE_EXPERIMENTAL
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
#include "../../physics/bounding_box.h"

class AABBNode : public SceneNode {
public:
	AABBNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	AABBNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u);
	AABBNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent = 0u);
	AABBNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, BoundingBox aabb, Scene::NodeIndex parent = 0u);
    AABBNode(std::shared_ptr<Scene> scene, std::string name, BoundingBox aabb, Scene::NodeIndex parent = 0u);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

	void setAABB(const BoundingBox& aabb);
    const BoundingBox& getAABB() const;

protected:
	BoundingBox m_aabb;
};