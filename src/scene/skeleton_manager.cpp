#include "skeleton_manager.h"

SkeletonManager::SkeletonManager() {}

void SkeletonManager::AddBone(const std::shared_ptr<BoneNode>& node) {
    if(!node) return;

    const std::shared_ptr<Scene>& scene = node->GetScene();

    for(const auto&[skin_name, bone_data] : node->getBoneDataMap()) {
        if(!m_skinned_data.contains(skin_name)) {
            m_skinned_data[skin_name] = std::make_shared<SkinnedData>();
            m_skinned_data[skin_name]->skeleton_name = skin_name;
        }

        const std::shared_ptr<SkinnedData>& skinned_data = m_skinned_data[skin_name];

        if(skinned_data->inverse_bind_matrices.size() <= bone_data.joint_index) {
            skinned_data->inverse_bind_matrices.resize(bone_data.joint_index + 1u);
            skinned_data->final_matrices.resize(bone_data.joint_index + 1u);
            skinned_data->dual_quats.resize(bone_data.joint_index + 1u);
        }
        skinned_data->inverse_bind_matrices[bone_data.joint_index] = bone_data.inverse_bind_matrice;

        skinned_data->bone_to_joint_map[node] = bone_data.joint_index;
        skinned_data->joint_to_bone_map[bone_data.joint_index] = node;
    }
    UpdateBoneData(node);
    markAsChanged(node);
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

const std::shared_ptr<SkeletonManager::SkinnedData>& SkeletonManager::getSkinnedData(const BoneNode::SkinName& name) const {
    return m_skinned_data.at(name);
}

const std::unordered_map<BoneNode::SkinName, std::shared_ptr<SkeletonManager::SkinnedData>>& SkeletonManager::getSkinMap() const {
    return m_skinned_data;
}

void SkeletonManager::resetSkin(const BoneNode::SkinName& name) {
    if(!m_skinned_data.contains(name)) return;

    const std::shared_ptr<SkinnedData>& skin_data = m_skinned_data[name];
    for(const auto&[joint_idx, bone_ptr] : skin_data->joint_to_bone_map) {
        bone_ptr->applyBindMatrice();
    }
}

bool SkeletonManager::UpdateBoneData(const std::shared_ptr<BoneNode>& node) {
    bool was_updated = false;

    for(const auto&[skin_name, bone_data] : node->getBoneDataMap()) {

        const std::shared_ptr<SkinnedData>& skinned_data = m_skinned_data[skin_name];
        glm::mat4 model_from_root = node->getBoneDataMap().at(skin_name).m_mesh_root_node->Get().FromRoot();
        glm::mat4 to_root = node->Get().ToRoot();
        skinned_data->final_matrices[bone_data.joint_index] = model_from_root * to_root * skinned_data->inverse_bind_matrices[bone_data.joint_index];
        //skinned_data->final_matrices[bone_data.joint_index] = to_root * skinned_data->inverse_bind_matrices[bone_data.joint_index];

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
                skinned_data->dual_quats[bone_data.joint_index] = glm::mat2x4_cast(dq);
            }
        }

        was_updated = true;
    }

    return was_updated;
}

void SkeletonManager::recalculate_root_joints(BoneNode::SkinName skeleton_name) {
    if(!m_skinned_data.contains(skeleton_name)) return;

    const std::shared_ptr<SkinnedData>& skinned_data = m_skinned_data[skeleton_name];
    const std::shared_ptr<Scene>& scene = skinned_data->joint_to_bone_map.begin()->second->GetScene();
    skinned_data->root_joints.clear();
    for(const auto&[current_idx, bone_ptr] : skinned_data->joint_to_bone_map) {
        const std::shared_ptr<SceneNode>& parent_node = bone_ptr->GetParent();
        if(std::shared_ptr<BoneNode> parent_bone = std::dynamic_pointer_cast<BoneNode>(scene->getProperty(parent_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_BONE))) {
            if(!skinned_data->bone_to_joint_map.contains(parent_bone)) {
                skinned_data->root_joints.push_back(current_idx);
            }
        }
        else {
            skinned_data->root_joints.push_back(current_idx);
        }
    }
}