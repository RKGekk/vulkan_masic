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
#include <vector>

#include "nodes/bone_node.h"

class SkeletonManager {
public:
    using JointIndex = uint32_t;

    SkeletonManager();

    void AddBone(const std::shared_ptr<BoneNode>& node);
    void SetBone(const std::shared_ptr<BoneNode>& node, JointIndex joint_idx);

    void markAsChanged(const std::shared_ptr<BoneNode>& node);
    void markAsChanged(JointIndex joint_idx);

private:
    std::vector<glm::mat4> m_inverse_bind_matrices;
    std::vector<glm::mat4> m_final_matrices;
    std::unordered_map<std::shared_ptr<BoneNode>, JointIndex> m_bone_to_joint_map;
    std::unordered_map<JointIndex, std::shared_ptr<BoneNode>> m_joint_to_bone_map;
    std::vector<JointIndex> m_dirty_at_joint;
};