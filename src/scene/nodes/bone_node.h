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

class BoneNode : public SceneNode {
public:

	BoneNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	BoneNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u);
	BoneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent = 0u);
    BoneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, glm::mat4 inverse_bind_matrice, Scene::NodeIndex parent = 0u);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

    const glm::mat4x4& getInverseBindMatrice() const;
	glm::mat4x4 getInverseBindMatriceT() const;
    void setInverseBindMatrice(const glm::mat4x4& inverse_bind_matrice);

private:
    glm::mat4 m_inverse_bind_matrice;
};