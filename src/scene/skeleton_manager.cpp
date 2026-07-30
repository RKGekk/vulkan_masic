#include "skeleton_manager.h"

SkeletonManager::SkeletonManager() {}

void SkeletonManager::AddBone(const std::shared_ptr<BoneNode>& node) {
    if(!node) return;

    for(const auto&[skin_name, bone_data] : node->getBoneDataMap()) {
        if(m_skinned_data.contains(skin_name)) continue;

        std::shared_ptr<SkinnedData> skinned_data = std::make_shared<SkinnedData>();

        if(skinned_data->inverse_bind_matrices.size() <= bone_data.joint_index) {
            skinned_data->inverse_bind_matrices.resize(bone_data.joint_index + 1u);
            skinned_data->final_matrices.resize(bone_data.joint_index + 1u);
            skinned_data->to_root_transforms.resize(bone_data.joint_index + 1u);
            skinned_data->m_dual_quats.resize(bone_data.joint_index + 1u);
        }
        skinned_data->inverse_bind_matrices[bone_data.joint_index] = bone_data.inverse_bind_matrice;

        skinned_data->skeleton_name = skin_name;
        skinned_data->bone_to_joint_map[node] = bone_data.joint_index;
        skinned_data->joint_to_bone_map[bone_data.joint_index] = node;
        m_skinned_data[skin_name] = std::move(skinned_data);
    }
    UpdateBoneData(node);
}

void SkeletonManager::markAsChanged(const std::shared_ptr<BoneNode>& node) {
    m_dirty_at_bone.insert(node);
}

bool SkeletonManager::recalculateSkinnedData() {
    bool was_updated = false;

    for (const std::shared_ptr<BoneNode>& dirty_bone : m_dirty_at_bone) {
        was_updated |= UpdateBoneData(dirty_bone);
    }

    m_dirty_at_bone.clear();

    return was_updated;
}

bool SkeletonManager::UpdateBoneData(const std::shared_ptr<BoneNode>& node) {
    bool was_updated = false;

    for(const auto&[skin_name, bone_data] : node->getBoneDataMap()) {

            const std::shared_ptr<SkinnedData>& skinned_data = m_skinned_data[skin_name];
            skinned_data->to_root_transforms[bone_data.joint_index] = node->Get().FromRoot();
            skinned_data->final_matrices[bone_data.joint_index] = skinned_data->to_root_transforms[bone_data.joint_index] * skinned_data->inverse_bind_matrices[bone_data.joint_index];

            {
                glm::quat orientation;
                glm::vec3 scale;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::dualquat dq;
                if (glm::decompose(skinned_data->final_matrices[bone_data.joint_index], scale, orientation, translation, skew, perspective)) {
                    dq[0] = orientation;
                    dq[1] = glm::quat(0.0f, translation.x, translation.y, translation.z) * orientation * 0.5f;
                    skinned_data->m_dual_quats[bone_data.joint_index] = glm::mat2x4_cast(dq);
                }
            }

            was_updated = true;
        }

    return was_updated;
}