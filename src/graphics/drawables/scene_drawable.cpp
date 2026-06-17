#include "scene_drawable.h"

#include "../../application.h"
#include "../../engine/base_engine_logic.h"
#include "../../actors/camera_component.h"
#include "../../scene/nodes/basic_camera_node.h"
#include "../../scene/nodes/value_bag_node.h"
#include "../api/vulkan_buffer.h"
#include "../api/vulkan_image_buffer.h"
#include "../api/vulkan_swapchain.h"
#include "../api/vulkan_resources_manager.h"
#include "../api/vulkan_descriptors_manager.h"
#include "../api/vulkan_push_constant.h"
#include "../pod/graphics_render_node.h"
#include "../pod/graphics_render_node_config.h"
#include "../pod/descriptor_set_layout.h"
#include "../pod/format_config.h"
#include "../pod/push_constant_config.h"
#include "../vulkan_renderer.h"
#include "../../tools/string_tools.h"

struct SceneUniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct PhongMaterial {
    glm::vec3 fresnelR0;
    float roughness;
};

bool SceneDrawable::init(std::shared_ptr<VulkanDevice> device, int max_frames) {
    using namespace std::literals;

    m_device = std::move(device);
    m_max_frames = max_frames;
    m_rt_aspect = Application::GetRenderer().getSwapchain()->getFormatConfig()->getAspect();
    m_viewport_extent = Application::GetRenderer().getSwapchain()->getFormatConfig()->getExtent2D();

    m_per_frame.resize(max_frames);
    for(int frame = 0; frame < m_max_frames; ++frame) {
        m_per_frame[frame] = std::make_shared<RenderPerFrame>();
        m_per_frame[frame]->light_buffer = Application::GetRenderer().getResourcesManager()->getBufferResource(LightManager::getLightBufferName() + std::to_string(frame));
    }

    return true;
}

void SceneDrawable::reset() {

}

void SceneDrawable::destroy() {
    // size_t sz = m_renderables.size();
    // for(size_t i = 0u; i < sz; ++i) {
    //     std::shared_ptr<Renderable>& renderable = m_renderables[i];
    //     renderable->uniform_buffer->destroy();
    //     renderable->vertex_buffer->destroy();
    //     renderable->index_buffer->destroy();
    //     renderable->texture->destroy();
    //     //renderable->render_node->destroy();
    // }
}

void SceneDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    size_t sz = m_per_frame[image_index]->renderables.size();
    if(!sz) return;

    if(m_per_frame[image_index]->light_buffer) {
        Application& app = Application::Get();
        const std::shared_ptr<LightManager>& light_manager = app.GetGameLogic()->GetHumanView()->VGetScene()->getLightManager();
        m_per_frame[image_index]->light_buffer->update(light_manager->getLightsData().data(), light_manager->getLightsData().size() * sizeof(LightNodeProperties));
    }
    
    for(size_t render_id = 0u; render_id < sz; ++render_id) {
        const std::shared_ptr<Renderable>& renderable = m_per_frame[image_index]->renderables.at(render_id);
        if(!renderable->mesh_node) continue;

        if(renderable->const_params.size() == 0u) continue;
        updatePushConstants(image_index, render_id);
    }
}

int SceneDrawable::order() {
    return 0;
}

std::string makeRenderNodeName(const std::shared_ptr<Material>& material) {
    std::string render_name = makeRenderName(material->GetName(), "_render"s);
    return render_name;
}

void SceneDrawable::addRendeNode(std::shared_ptr<MeshNode> model) {
    using namespace std::literals;

    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();
    std::shared_ptr<Scene> scene = model->GetScene();

    std::shared_ptr<ValueBagNode> value_bag_node = std::dynamic_pointer_cast<ValueBagNode>(scene->getProperty(model->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_VALUE_BAG));
    const MeshNode::MeshList& mesh_list = model->GetMeshes();
    size_t msz = mesh_list.size();
    for(int frame = 0; frame < m_max_frames; ++frame) {
        m_per_frame[frame]->renderables.reserve(m_per_frame[frame]->renderables.size() + msz);
    }
    for (size_t i = 0u; i < msz; ++i) {
        std::shared_ptr<ModelData> model_data = mesh_list.at(i);
        std::shared_ptr<Material> material = model_data->GetMaterial();

        for(int frame = 0; frame < m_max_frames; ++frame) {
            std::shared_ptr<RenderPerFrame>& per_frame_data = m_per_frame[frame];
            size_t renderable_id = per_frame_data->renderables.size();

            std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>();
            per_frame_data->renderables.push_back(renderable);
            renderable->mesh_node = model;
            
            renderable->texture = material->GetTexture();

            renderable->vertex_buffer = model_data->GetVertexBuffer();
            renderable->index_buffer = model_data->GetIndexBuffer();

            std::string render_name = makeRenderNodeName(material);
            if(!Application::GetRenderer().getFrameData(frame)->render_graph->hasGraphicsRenderNodeConfig(render_name)) {
                render_name = "mesh_render"s;
            }

            renderable->render_node = std::make_shared<GraphicsRenderNode>();
            renderable->render_node->init(m_device, render_name, false, Application::GetRenderer().getFrameData(frame)->render_graph);

            const std::shared_ptr<GraphicsRenderNodeConfig>& render_node_cfg = renderable->render_node->getGraphicsRenderNodeConfig();

            std::shared_ptr<VulkanShader> vertex_shader = renderable->render_node->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
            if(vertex_shader && vertex_shader->getShaderSignature()->getPushConstants()) {
                renderable->const_params.push_back(vertex_shader->getShaderSignature()->getPushConstants());
            }

            std::shared_ptr<VulkanShader> pixel_shader = renderable->render_node->getPipeline()->getShader(VK_SHADER_STAGE_FRAGMENT_BIT);
            if(pixel_shader && pixel_shader->getShaderSignature()->getPushConstants()) {
                renderable->const_params.push_back(pixel_shader->getShaderSignature()->getPushConstants());
            }

            if(value_bag_node && renderable->const_params.size() > 0u) {
                updatePushConstants(frame, renderable_id);
            }
            
            if(renderable->vertex_buffer) {
                renderable->render_node->addReadDependency(renderable->vertex_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getVertexBufferBindingName());
            }
            if(renderable->index_buffer) {
                renderable->render_node->addReadDependency(renderable->index_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getIndexBufferBindingName());
            }

            renderable->render_node->add_update_function(
                "mvp_matrices_update"s,
                [&, frame, renderable_id](std::shared_ptr<VulkanBuffer>& uniform_buffer){
                    updateMVPMatrices(m_per_frame[frame]->renderables.at(renderable_id)->mesh_node, uniform_buffer);
                }
            );

            renderable->render_node->add_update_function(
                "invmvp_matrices_update"s,
                [&, frame, renderable_id](std::shared_ptr<VulkanBuffer>& uniform_buffer){
                    updateInvMVPMatrices(m_per_frame[frame]->renderables.at(renderable_id)->mesh_node, uniform_buffer);
                }
            );

            renderable->render_node->add_update_function(
                "material_prop_update"s,
                [&, material](std::shared_ptr<VulkanBuffer>& uniform_buffer){
                    updateMaterialProps(material, uniform_buffer);
                }
            );

            for(const auto&[desc_slot, desc_set_name] : vertex_shader->getShaderSignature()->getDescSetNames()) {
                const std::shared_ptr<DescSetLayout>& desc_set_layout = Application::GetRenderer().getDescriptorsManager()->getDescSetLayout(desc_set_name);

                for (const auto&[desc_layout_bind_name, bind_num] : desc_set_layout->getBindingMap()) {
                    const VkDescriptorSetLayoutBinding& vk_layout_binding = desc_set_layout->getBinding(bind_num);
                    const std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>& update_metadata = render_node_cfg->getBindingsMetadata().at(desc_layout_bind_name);
                    if(vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && update_metadata->creation_point == GraphicsRenderNodeConfig::CreationPoint::RENDER_NODE_CREATION_TIME) {
                        std::shared_ptr<VulkanBuffer> ubo = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, model_data->GetName() + desc_layout_bind_name + "_uniform_frame_"s + std::to_string(frame), update_metadata->buffer_resource_type_name);
                        renderable->render_node->addReadDependency(ubo, desc_layout_bind_name);
                        renderable->uniform_buffers[desc_layout_bind_name] = std::move(ubo);
                    }
                    else if(vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && update_metadata->creation_point == GraphicsRenderNodeConfig::CreationPoint::EXTERNAL) {
                        std::shared_ptr<VulkanBuffer> ubo = Application::GetRenderer().getResourcesManager()->getBufferResource(desc_layout_bind_name + std::to_string(frame));
                        renderable->render_node->addReadDependency(ubo, desc_layout_bind_name);
                    }
                    else if(vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && renderable->texture) {
                        renderable->render_node->addReadDependency(renderable->texture, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
                    }
                }
            }

            renderable->render_node->addWriteDependency(swapchain_images[frame], "resolve_attachment");
            renderable->render_node->addWriteDependency(Application::GetRenderer().getOutColorImage(frame), "color_attachment");
            renderable->render_node->addWriteDependency(Application::GetRenderer().getOutDepthImage(frame), "depth_attachment");
            renderable->render_node->finishRenderNode();

            Application::GetRenderer().addRenderNode(renderable->render_node, frame);
        }
    }
}

void SceneDrawable::updatePushConstants(int frame, RenderableId render_id) {
    if(m_per_frame[frame]->renderables[render_id]->const_params.size() == 0u) return;

    const std::shared_ptr<MeshNode> mesh_node = m_per_frame[frame]->renderables[render_id]->mesh_node;
    std::shared_ptr<Scene> scene = mesh_node->GetScene();
    std::shared_ptr<ValueBagNode> const_params_value_bag_node = std::dynamic_pointer_cast<ValueBagNode>(scene->getProperty(mesh_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_VALUE_BAG));
    if(!const_params_value_bag_node) {
        const_params_value_bag_node = std::make_shared<ValueBagNode>(scene, mesh_node->VGetNodeIndex());
        scene->addProperty(const_params_value_bag_node);
    }
    
    for(const auto&[const_name, metadata_id] : const_params_value_bag_node->GetMetadata()) {
        for(std::shared_ptr<VulkanPushConstant>& push_const : m_per_frame[frame]->renderables[render_id]->const_params) {
            if(push_const->getConstConfig()->hasPushConstantsName(const_name)) {
                push_const->SetValue(const_name, const_params_value_bag_node->GetValue(const_name));
            }
        }
    }
}

void SceneDrawable::updateMVPMatrices(const std::shared_ptr<SceneNode>& scene_node, std::shared_ptr<VulkanBuffer>& uniform_buffer) {
    Application& app = Application::Get();
    const std::shared_ptr<BaseEngineLogic>& game_logic = app.GetGameLogic();
    const std::shared_ptr<CameraComponent>& camera_component = game_logic->GetHumanView()->VGetCamera();
    const std::shared_ptr<BasicCameraNode>& camera_node = camera_component->VGetCameraNode();
    const SceneNodeProperties& node_props = scene_node->Get();
    
    SceneUniformBufferObject ubo{};
    ubo.model = node_props.ToRoot();
    ubo.view = camera_node->GetView();
    ubo.proj = camera_node->GetProjection();
    //ubo.proj[1][1] *= -1.0f;

    uniform_buffer->update(&ubo, sizeof(SceneUniformBufferObject));
}


void SceneDrawable::updateInvMVPMatrices(const std::shared_ptr<SceneNode>& scene_node, std::shared_ptr<VulkanBuffer>& uniform_buffer) {\
    Application& app = Application::Get();
    const std::shared_ptr<BaseEngineLogic>& game_logic = app.GetGameLogic();
    const std::shared_ptr<CameraComponent>& camera_component = game_logic->GetHumanView()->VGetCamera();
    const std::shared_ptr<BasicCameraNode>& camera_node = camera_component->VGetCameraNode();
    const SceneNodeProperties& node_props = scene_node->Get();
    
    SceneUniformBufferObject ubo{};
    ubo.model = node_props.FromRoot();
    ubo.view = camera_node->GetInvView();
    ubo.proj = camera_node->GetInvProjection();
    //ubo.proj[1][1] *= -1.0f;

    uniform_buffer->update(&ubo, sizeof(SceneUniformBufferObject));
}

void SceneDrawable::updateMaterialProps(const std::shared_ptr<Material>& material, std::shared_ptr<VulkanBuffer>& uniform_buffer) {
    PhongMaterial mat;

    mat.fresnelR0 = glm::vec3(material->GetReflectance());
    mat.roughness = material->GetRoughnessFactor();

    uniform_buffer->update(&mat, sizeof(PhongMaterial));
}