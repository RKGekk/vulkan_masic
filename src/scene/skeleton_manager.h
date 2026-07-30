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
#include <glm/gtx/dual_quaternion.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "nodes/bone_node.h"

class SkeletonManager {
public:
    struct SkinnedData {
        BoneNode::SkinName skeleton_name;
        std::vector<glm::mat4> inverse_bind_matrices;
        std::vector<glm::mat4> to_root_transforms;
        std::vector<glm::mat4> final_matrices;
        std::vector<glm::mat2x4> m_dual_quats;
        std::unordered_map<std::shared_ptr<BoneNode>, BoneNode::JointIndex> bone_to_joint_map;
        std::unordered_map<BoneNode::JointIndex, std::shared_ptr<BoneNode>> joint_to_bone_map;
    };

    SkeletonManager();
    bool recalculateSkinnedData();

    void AddBone(const std::shared_ptr<BoneNode>& node);
    void markAsChanged(const std::shared_ptr<BoneNode>& node);

private:
    bool UpdateBoneData(const std::shared_ptr<BoneNode>& node);

    std::unordered_map<BoneNode::SkinName, std::shared_ptr<SkinnedData>> m_skinned_data;
    std::unordered_set<std::shared_ptr<BoneNode>> m_dirty_at_bone;
};