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
        const std::shared_ptr<VulkanBuffer>& camera_data = m_camera_data[dirty_camera][frame_number];

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

    for(int frame = 0; frame < m_max_frames; ++frame) {
        std::string camera_buffer_name = node->Get().Name() + "_camera_buffer"s + std::to_string(frame);
        m_camera_data[node][frame] = resources_manager->create_buffer(nullptr, sizeof(CameraUniformBufferObject), std::move(camera_buffer_name), m_camera_resource_cfg_name);
    }
    
    markAsChanged(node);
}

void CameraManager::markAsChanged(const std::shared_ptr<CameraNode>& node) {
    for(int frame = 0; frame < m_max_frames; ++frame) {
        m_dirty_at_node[frame].insert(node);
    }
}

const std::shared_ptr<VulkanBuffer>& CameraManager::getCameraMatrixBuffer(std::string camera_name, FrameNumber frame_number) const {
    for (const auto&[camera_node, cam_data_frame] : m_camera_data) {
        if(camera_node->Get().Name() == camera_name) return cam_data_frame.at(frame_number);
    }
    return VulkanBuffer::BUFFER_NULL_PTR;
}

const std::shared_ptr<VulkanBuffer>& CameraManager::getCameraMatrixBuffer(const std::shared_ptr<CameraNode> camera_node, FrameNumber frame_number) const {
    if(!m_camera_data.contains(camera_node)) return VulkanBuffer::BUFFER_NULL_PTR;
    return m_camera_data.at(camera_node).at(frame_number);
}

const std::string& CameraManager::getCameraResourceCfgName() {
    return m_camera_resource_cfg_name;
}

const std::string& CameraManager::getMatrixUBODescName() {
    return m_matrix_desc_name;
}

void CameraManager::DecorateRenderNode(const std::shared_ptr<RenderNode>& render_node, std::string camera_name) const {
    int frame = render_node->getFrameNumber();
    const std::shared_ptr<VulkanBuffer>& matrix_buffer = getCameraMatrixBuffer(camera_name, frame);
    for(const auto&[desc_slot, desc] : render_node->getDescriptors()) {
        const std::shared_ptr<DescSetLayout>& desc_set_layout = desc->getBindings();
        for(const auto&[desc_layout_bind_name, bind_num] : desc_set_layout->getBindingMap()) {
            const VkDescriptorSetLayoutBinding& vk_layout_binding = desc_set_layout->getBinding(bind_num);
            bool has_camera_ubo = vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && desc_layout_bind_name == m_matrix_desc_name;
            if(has_camera_ubo) {
                if(render_node->isReadAttached(desc_layout_bind_name)) {
                    std::shared_ptr<VulkanBuffer> old_buffer = render_node->getAttachedBufferResource(desc_layout_bind_name);
                    old_buffer->ClearUpdateMetadataFn();

                    matrix_buffer->AddUpdateMetadataFn(
                        [render_node, &desc_layout_bind_name]
                        (const std::shared_ptr<RenderResource>& bo){
                            render_node->changeReadDependency(bo, desc_layout_bind_name);
                            render_node->updateDescriptor(desc_layout_bind_name);
                        }
                    );
                    render_node->changeReadDependency(matrix_buffer, desc_layout_bind_name);
                }
                else {
                    matrix_buffer->AddUpdateMetadataFn(
                        [render_node, &desc_layout_bind_name]
                        (const std::shared_ptr<RenderResource>& bo){
                            render_node->changeReadDependency(bo, desc_layout_bind_name);
                            render_node->updateDescriptor(desc_layout_bind_name);
                        }
                    );
                    render_node->addReadDependency(matrix_buffer, desc_layout_bind_name);
                }
            }
        }
    }
}