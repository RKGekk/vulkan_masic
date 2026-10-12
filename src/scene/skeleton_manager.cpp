#include "skeleton_manager.h"

#include "../application.h"
#include "../graphics/vulkan_renderer.h"
#include "../graphics/api/vulkan_resources_manager.h"
#include "../graphics/pod/render_node.h"

const std::string SkeletonManager::m_skin_resource_cfg_name = "joint_bind_storage_resource";

const std::string SkeletonManager::m_skin_bind_ssbo_desc_name = "joint_bind_ssbo";
const std::string SkeletonManager::m_skin_joint_ssbo_desc_name = "joint_ssbo";
const std::string SkeletonManager::m_skin_dq_ssbo_desc_name = "joint_dq_ssbo";

SkeletonManager::SkeletonManager() {}

void SkeletonManager::AddBone(const std::shared_ptr<BoneNode>& node) {
    using namespace std::literals;
    if(!node) return;

    VulkanRenderer& renderer = Application::GetRenderer();
    std::shared_ptr<VulkanResourcesManager>& resources_manager = renderer.getResourcesManager();

    for(const auto&[skin_name, bone_data] : node->getBoneDataMap()) {
        if(!m_skinned_data.contains(skin_name)) {
            m_skinned_data[skin_name] = std::make_shared<SkinnedData>();
            m_skinned_data[skin_name]->skeleton_name = skin_name;

            std::string skin_bind_buffer_name = "skinned_data_bind_buffer_"s + skin_name;
            m_skinned_data[skin_name]->bind_buffer = resources_manager->create_buffer(nullptr, 1u * sizeof(glm::mat4), std::move(skin_bind_buffer_name), m_skin_resource_cfg_name);

            std::string skin_joint_buffer_name = "skinned_data_joint_buffer_"s + skin_name;
            m_skinned_data[skin_name]->joint_buffer = resources_manager->create_buffer(nullptr, 1u * sizeof(glm::mat4), std::move(skin_joint_buffer_name), m_skin_resource_cfg_name);

            std::string skin_dq_buffer_name = "skinned_data_dq_buffer_"s + skin_name;
            m_skinned_data[skin_name]->dq_buffer = resources_manager->create_buffer(nullptr, 1u * sizeof(glm::mat2x4), std::move(skin_dq_buffer_name), m_skin_resource_cfg_name);
        }

        const std::shared_ptr<SkinnedData>& skinned_data = m_skinned_data[skin_name];

        if(skinned_data->inverse_bind_matrices.size() <= bone_data.joint_index) {
            skinned_data->bind_matrices.resize(bone_data.joint_index + 1u);
            skinned_data->inverse_bind_matrices.resize(bone_data.joint_index + 1u);
            skinned_data->final_matrices.resize(bone_data.joint_index + 1u);
            skinned_data->dual_quats.resize(bone_data.joint_index + 1u);
        }
        skinned_data->bind_matrices[bone_data.joint_index] = bone_data.bind_matrice;
        skinned_data->inverse_bind_matrices[bone_data.joint_index] = bone_data.inverse_bind_matrice;

        skinned_data->bone_to_joint_map[node] = bone_data.joint_index;
        skinned_data->joint_to_bone_map[bone_data.joint_index] = node;
    }
    
    markAsChanged(node, true);
}

void SkeletonManager::markAsChanged(const std::shared_ptr<BoneNode>& node, bool bind_changed) {
    m_dirty_at_bone.insert({node, bind_changed});
}

bool SkeletonManager::recalculateSkinnedData() {
    bool was_updated = false;
    bool was_bind_changed = false;

    std::unordered_set<std::pair<const BoneNode::SkinName&, bool>> skins;
    for (const auto&[dirty_bone, bind_changed] : m_dirty_at_bone) {
        was_updated |= UpdateBoneData(dirty_bone, bind_changed);
        if(was_updated) {
            was_bind_changed |= bind_changed;
            for(const auto&[skin_name, bone_data] : dirty_bone->getBoneDataMap()) {
                skins.insert({skin_name, bind_changed});
            }
        }
    }

    for (const auto& [dirty_skin_name, bind_changed] : skins) {
        const std::shared_ptr<SkinnedData>& skinned_data = m_skinned_data[dirty_skin_name];
        if(bind_changed) {
            skinned_data->bind_buffer->update(skinned_data->bind_matrices.data(), sizeof(glm::mat4) * skinned_data->bind_matrices.size());
        }
        skinned_data->joint_buffer->update(skinned_data->final_matrices.data(), sizeof(glm::mat4) * skinned_data->final_matrices.size());
        skinned_data->dq_buffer->update(skinned_data->dual_quats.data(), sizeof(glm::mat2x4) * skinned_data->dual_quats.size());
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

std::unordered_set<BoneNode::SkinName> SkeletonManager::getMeshSkins(const std::shared_ptr<SceneNode>& mesh_root_node) const {
    std::unordered_set<BoneNode::SkinName> result;
    for (const auto&[skin_name, skinned_data] : m_skinned_data) {
        const std::shared_ptr<BoneNode>& bone_node = (*skinned_data->bone_to_joint_map.begin()).first;
        for(const auto&[skin_in_bone_name, bone_data] : bone_node->getBoneDataMap()) {
            if(bone_data.mesh_root_node == mesh_root_node) {
                result.insert(skin_in_bone_name);
            }
        }
    }
    return result;
}

const std::shared_ptr<VulkanBuffer>& SkeletonManager::getSkeletonBindBuffer(const BoneNode::SkinName& name) const {
    return m_skinned_data.at(name)->bind_buffer;
}

const std::shared_ptr<VulkanBuffer>& SkeletonManager::getSkeletonJointBuffer(const BoneNode::SkinName& name) const {
    return m_skinned_data.at(name)->joint_buffer;
}

const std::shared_ptr<VulkanBuffer>& SkeletonManager::getSkeletonDQBuffer(const BoneNode::SkinName& name) const {
    return m_skinned_data.at(name)->dq_buffer;
}

const std::string& SkeletonManager::getSkeletonResourceCfgName() {
    return m_skin_resource_cfg_name;
}

const std::string& SkeletonManager::getSkeletonBindSSBODescName() {
    return m_skin_bind_ssbo_desc_name;
}

const std::string& SkeletonManager::getSkeletonJointSSBODescName() {
    return m_skin_joint_ssbo_desc_name;
}

const std::string& SkeletonManager::getSkeletonDQSSBODescName() {
    return m_skin_dq_ssbo_desc_name;
}

void SkeletonManager::DecorateRenderNode(const std::shared_ptr<RenderNode>& render_node, const BoneNode::SkinName& skin_name) const {
    if(!m_skinned_data.contains(skin_name)) return;
    const std::shared_ptr<SkinnedData> skinned_data = m_skinned_data.at(skin_name);
    for(const auto&[desc_slot, desc] : render_node->getDescriptors()) {
        const std::shared_ptr<DescSetLayout>& desc_set_layout = desc->getBindings();
        for(const auto&[desc_layout_bind_name, bind_num] : desc_set_layout->getBindingMap()) {
            const VkDescriptorSetLayoutBinding& vk_layout_binding = desc_set_layout->getBinding(bind_num);
            
            bool has_skeletal_bind_ssbo = vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER && desc_layout_bind_name == m_skin_bind_ssbo_desc_name;
            bool has_skeletal_joint_ssbo = vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER && desc_layout_bind_name == m_skin_joint_ssbo_desc_name;
            bool has_skeletal_dq_ssbo = vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER && desc_layout_bind_name == m_skin_dq_ssbo_desc_name;
            
            if(has_skeletal_bind_ssbo) {
                const std::shared_ptr<VulkanBuffer>& bind_buffer = skinned_data->bind_buffer;
                bind_buffer->AddUpdateMetadataFn(
                    [render_node, &desc_layout_bind_name]
                    (const std::shared_ptr<RenderResource>& bo){
                        render_node->changeReadDependency(bo, desc_layout_bind_name);
                        render_node->updateDescriptor(desc_layout_bind_name);
                    }
                );
                render_node->addReadDependency(bind_buffer, desc_layout_bind_name);
            }

            if(has_skeletal_joint_ssbo) {
                const std::shared_ptr<VulkanBuffer>& joint_buffer = skinned_data->joint_buffer;
                joint_buffer->AddUpdateMetadataFn(
                    [render_node, &desc_layout_bind_name]
                    (const std::shared_ptr<RenderResource>& bo){
                        render_node->changeReadDependency(bo, desc_layout_bind_name);
                        render_node->updateDescriptor(desc_layout_bind_name);
                    }
                );
                render_node->addReadDependency(joint_buffer, desc_layout_bind_name);
            }

            if(has_skeletal_dq_ssbo) {
                const std::shared_ptr<VulkanBuffer>& dq_buffer = skinned_data->dq_buffer;
                dq_buffer->AddUpdateMetadataFn(
                    [render_node, &desc_layout_bind_name]
                    (const std::shared_ptr<RenderResource>& bo){
                        render_node->changeReadDependency(bo, desc_layout_bind_name);
                        render_node->updateDescriptor(desc_layout_bind_name);
                    }
                );
                render_node->addReadDependency(dq_buffer, desc_layout_bind_name);
            }
        }
    }
}

bool SkeletonManager::UpdateBoneData(const std::shared_ptr<BoneNode>& node, bool bind_changed) {
    bool was_updated = false;

    for(const auto&[skin_name, bone_data] : node->getBoneDataMap()) {

        const std::shared_ptr<SkinnedData>& skinned_data = m_skinned_data[skin_name];
        glm::mat4 model_from_root = node->getBoneDataMap().at(skin_name).mesh_root_node->Get().FromRoot();
        glm::mat4 to_root = node->Get().ToRoot();
        if(bind_changed) {
            skinned_data->bind_matrices[bone_data.joint_index] = skinned_data->bind_matrices[bone_data.joint_index];
        }
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