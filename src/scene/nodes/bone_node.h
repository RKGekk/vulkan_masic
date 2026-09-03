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
#include <string>
#include <unordered_map>

#include "scene_node.h"

class BoneNode : public SceneNode {
public:
	using JointIndex = uint32_t;
	using SkinName = std::string;

	struct BoneData {
		glm::mat4 inverse_bind_matrice;
		JointIndex joint_index;
		std::shared_ptr<SceneNode> mesh_root_node;
	};

	BoneNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	BoneNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u);
	BoneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent = 0u);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

	const glm::mat4x4& getInverseBindMatrice(const SkinName& skin_name) const;
    const glm::mat4x4& getInverseBindMatrice() const;
	glm::mat4x4 getInverseBindMatriceT() const;
	glm::mat4x4 getInverseBindMatriceT(const SkinName& skin_name) const;
    void setInverseBindMatrice(const glm::mat4x4& inverse_bind_matrice);
	void setInverseBindMatrice(const glm::mat4x4& inverse_bind_matrice, const SkinName& skin_name);

	const glm::mat4x4& getBindMatrice() const;
	void setBindMatrice(glm::mat4x4 bind_matrice);
	void applyBindMatrice();

	JointIndex getJointIndex() const;
	JointIndex getJointIndex(const SkinName& skin_name) const;
	void setJointIndex(JointIndex joint_idx);
	void setJointIndex(JointIndex joint_idx, const SkinName& skin_name);

	void setBoneData(BoneData bone_data);
	void setBoneData(BoneData bone_data, const SkinName& skin_name);

	const std::unordered_map<SkinName, BoneData>& getBoneDataMap() const;

private:
    std::unordered_map<SkinName, BoneData> m_bone_data;
	glm::mat4x4 m_bind_transform;
};