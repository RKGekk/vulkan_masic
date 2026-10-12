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
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "nodes/bone_node.h"

class RenderNode;

class SkeletonManager {
public:
    struct SkinnedData {
        BoneNode::SkinName skeleton_name;

        std::vector<glm::mat4> inverse_bind_matrices;
        std::vector<glm::mat4> bind_matrices;
        std::shared_ptr<VulkanBuffer> bind_buffer;
        
        std::vector<glm::mat4> final_matrices;
        std::shared_ptr<VulkanBuffer> joint_buffer;
        
        std::vector<glm::mat2x4> dual_quats;
        std::shared_ptr<VulkanBuffer> dq_buffer;

        std::unordered_map<std::shared_ptr<BoneNode>, BoneNode::JointIndex> bone_to_joint_map;
        std::unordered_map<BoneNode::JointIndex, std::shared_ptr<BoneNode>> joint_to_bone_map;
        std::vector<BoneNode::JointIndex> root_joints;
    };

    SkeletonManager();
    bool recalculateSkinnedData();
    void recalculate_root_joints(BoneNode::SkinName skeleton_name);

    void AddBone(const std::shared_ptr<BoneNode>& node);
    void markAsChanged(const std::shared_ptr<BoneNode>& node, bool bind_changed);

    const std::shared_ptr<SkinnedData>& getSkinnedData(const BoneNode::SkinName& name) const;
    const std::unordered_map<BoneNode::SkinName, std::shared_ptr<SkinnedData>>& getSkinMap() const;
    void resetSkin(const BoneNode::SkinName& name);
    std::unordered_set<BoneNode::SkinName> getMeshSkins(const std::shared_ptr<SceneNode>& mesh_root_node) const;

    const std::shared_ptr<VulkanBuffer>& getSkeletonBindBuffer(const BoneNode::SkinName& name) const;
    const std::shared_ptr<VulkanBuffer>& getSkeletonJointBuffer(const BoneNode::SkinName& name) const;
    const std::shared_ptr<VulkanBuffer>& getSkeletonDQBuffer(const BoneNode::SkinName& name) const;

    static const std::string& getSkeletonResourceCfgName();
    static const std::string& getSkeletonBindSSBODescName();
    static const std::string& getSkeletonJointSSBODescName();
    static const std::string& getSkeletonDQSSBODescName();

    void DecorateRenderNode(const std::shared_ptr<RenderNode>& render_node, const BoneNode::SkinName& skin_name) const;

private:
    bool UpdateBoneData(const std::shared_ptr<BoneNode>& node, bool bind_changed);

    static const std::string m_skin_resource_cfg_name;

    static const std::string m_skin_bind_ssbo_desc_name;
    static const std::string m_skin_joint_ssbo_desc_name;
    static const std::string m_skin_dq_ssbo_desc_name;

    std::unordered_map<BoneNode::SkinName, std::shared_ptr<SkinnedData>> m_skinned_data;
    std::unordered_set<std::pair<std::shared_ptr<BoneNode>, bool>> m_dirty_at_bone;
};