#include "skeleton_manager.h"

SkeletonManager::SkeletonManager() {}

void SkeletonManager::AddBone(const std::shared_ptr<BoneNode>& node) {
    JointIndex joint_idx = m_inverse_bind_matrices.size();
    m_bone_to_joint_map[node] = joint_idx;
    m_joint_to_bone_map[joint_idx] = node;
    m_inverse_bind_matrices.push_back(node->getInverseBindMatrice());
    m_dirty_at_joint.push_back(joint_idx);
}

void SkeletonManager::SetBone(const std::shared_ptr<BoneNode>& node, JointIndex joint_idx) {
    if(m_inverse_bind_matrices.size() < joint_idx) {
        m_inverse_bind_matrices.resize(joint_idx);
    }

    m_bone_to_joint_map[node] = joint_idx;
    m_joint_to_bone_map[joint_idx] = node;
    m_inverse_bind_matrices[joint_idx] = node->getInverseBindMatrice();
}

void SkeletonManager::markAsChanged(const std::shared_ptr<BoneNode>& node) {

}

void SkeletonManager::markAsChanged(JointIndex joint_idx) {
    
}