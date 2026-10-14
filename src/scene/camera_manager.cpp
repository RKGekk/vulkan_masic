#include "camera_manager.h"

#include "../application.h"
#include "../graphics/vulkan_renderer.h"
#include "../graphics/api/vulkan_resources_manager.h"
#include "../graphics/pod/render_node.h"
#include "../graphics/api/vulkan_swapchain.h"

const std::string CameraManager::m_camera_resource_cfg_name = "camera_uniform_resource";

const std::string CameraManager::m_matrix_desc_name = "camera_ubo";

CameraManager::CameraManager() {
    VulkanRenderer& renderer = Application::GetRenderer();
    const std::shared_ptr<VulkanSwapChain>& swapchain = renderer.getSwapchain();
    m_max_frames = swapchain->getMaxFrames();
}

bool CameraManager::recalculateCameraData(FrameNumber frame_number) {
    bool was_updated = false;

    for (const std::shared_ptr<CameraNode>& dirty_camera : m_dirty_at_node[frame_number]) {
        const std::shared_ptr<VulkanBuffer>& camera_data = m_camera_data[dirty_camera];

        CameraUniformBufferObject ubo{};
        ubo.view = dirty_camera->GetView();
        ubo.proj = dirty_camera->GetProjection();
        ubo.inv_view = dirty_camera->GetInvView();
        ubo.inv_proj = dirty_camera->GetInvProjection();

        camera_data->update(&ubo, sizeof(CameraUniformBufferObject));
    }

    m_dirty_at_node[frame_number].clear();

    return was_updated;
}

void CameraManager::AddCamera(const std::shared_ptr<CameraNode>& node) {
    using namespace std::literals;
    if(!node) return;

    VulkanRenderer& renderer = Application::GetRenderer();
    std::shared_ptr<VulkanResourcesManager>& resources_manager = renderer.getResourcesManager();

    std::string camera_name = node->Get().Name() + "_camera_buffer"s;
    m_camera_data[node] = resources_manager->create_buffer(nullptr, sizeof(CameraUniformBufferObject), std::move(camera_name), m_camera_resource_cfg_name);
    
    markAsChanged(node);
}

void SkeletonManager::markAsChanged(const std::shared_ptr<BoneNode>& node, bool bind_changed) {
    for(int frame = 0; frame < m_max_frames; ++frame) {
        m_dirty_at_bone[frame].insert({node, bind_changed});
    }
}

const std::shared_ptr<SkeletonManager::SkinnedData>& SkeletonManager::getSkinnedData(const BoneNode::SkinName& name, FrameNumber frame_numbe) const {
    return m_skinned_data.at(name).at(frame_numbe);
}

const std::unordered_map<BoneNode::SkinName, SkeletonManager::SkinnedDataFrame>& SkeletonManager::getSkinMap() const {
    return m_skinned_data;
}

void SkeletonManager::resetSkin(const BoneNode::SkinName& name) {
    if(!m_skinned_data.contains(name)) return;

    const std::shared_ptr<SkinnedData>& skin_data = m_skinned_data[name][0];
    for(const auto&[joint_idx, bone_ptr] : skin_data->skeleton_structure->joint_to_bone_map) {
        bone_ptr->applyBindMatrice();
    }
}

std::unordered_set<BoneNode::SkinName> SkeletonManager::getMeshSkins(const std::shared_ptr<SceneNode>& mesh_root_node) const {
    std::unordered_set<BoneNode::SkinName> result;
    for (const auto&[skin_name, skinned_data_frame] : m_skinned_data) {
        const std::shared_ptr<BoneNode>& bone_node = (*skinned_data_frame.at(0)->skeleton_structure->bone_to_joint_map.begin()).first;
        for(const auto&[skin_in_bone_name, bone_data] : bone_node->getBoneDataMap()) {
            if(bone_data.mesh_root_node == mesh_root_node) {
                result.insert(skin_in_bone_name);
            }
        }
    }
    return result;
}

const std::shared_ptr<VulkanBuffer>& SkeletonManager::getSkeletonBindBuffer(const BoneNode::SkinName& name, FrameNumber frame_number) const {
    return m_skinned_data.at(name).at(frame_number)->bind_buffer;
}

const std::shared_ptr<VulkanBuffer>& SkeletonManager::getSkeletonJointBuffer(const BoneNode::SkinName& name, FrameNumber frame_number) const {
    return m_skinned_data.at(name).at(frame_number)->joint_buffer;
}

const std::shared_ptr<VulkanBuffer>& SkeletonManager::getSkeletonDQBuffer(const BoneNode::SkinName& name, FrameNumber frame_number) const {
    return m_skinned_data.at(name).at(frame_number)->dq_buffer;
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

    int frame = render_node->getFrameNumber();
    const std::shared_ptr<SkinnedData> skinned_data = m_skinned_data.at(skin_name).at(frame);
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