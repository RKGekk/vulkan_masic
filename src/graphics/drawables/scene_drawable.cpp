#include "scene_drawable.h"

#include "../../application.h"
#include "../../engine/base_engine_logic.h"
#include "../../actors/components/camera_component.h"
#include "../../scene/scene.h"
#include "../../scene/nodes/basic_camera_node.h"
#include "../../scene/nodes/value_bag_node.h"
#include "../../scene/skeleton_manager.h"
#include "../../scene/camera_manager.h"
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
#include "../pod/buffer_config.h"
#include "../vulkan_renderer.h"
#include "../../tools/string_tools.h"

struct ModelUniformBufferObject {
    glm::mat4 model;
    glm::mat4 inv_model;
};

struct PhongMaterial {
    glm::vec4 fresnelR0_roughness;
};

bool SceneDrawable::init(std::shared_ptr<Scene> scene) {
    using namespace std::literals;

    m_scene = std::move(scene);

    VulkanRenderer& renderer = Application::GetRenderer();
	std::shared_ptr<VulkanDevice> device = renderer.GetDevice();

    const std::shared_ptr<VulkanSwapChain>& swapchain = renderer.getSwapchain();
    const std::shared_ptr<FormatConfig>& format_cfg = swapchain->getFormatConfig();

    m_device = std::move(device);
    m_max_frames = renderer.getSwapchain()->getMaxFrames();
    m_rt_aspect = format_cfg->getAspect();
    m_viewport_extent = format_cfg->getExtent2D();

    m_per_frame.resize(m_max_frames);
    for(int frame = 0; frame < m_max_frames; ++frame) {
        m_per_frame[frame] = std::make_shared<RenderPerFrame>();
    }

    return true;
}

void SceneDrawable::reset() {

}

void SceneDrawable::destroy() {

}

void SceneDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    size_t sz = m_per_frame[image_index]->renderables.size();
    if(!sz) return;
    const std::shared_ptr<LightManager>& light_manager = m_scene->getLightManager();

    for(size_t render_id = 0u; render_id < sz; ++render_id) {
        const std::shared_ptr<Renderable>& renderable = m_per_frame[image_index]->renderables.at(render_id);
        if(!renderable->mesh_node) continue;

        light_manager->DecorateValueBag(renderable->mesh_node);

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

    Application& app = Application::Get();
    VulkanRenderer& renderer = Application::GetRenderer();
    std::shared_ptr<VulkanResourcesManager>& resources_manager = renderer.getResourcesManager();
    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = renderer.getSwapchain()->getSwapchainImages();
    const std::shared_ptr<Scene>& scene = model->GetScene();
    const std::shared_ptr<SkeletonManager>& skeleton_manager = scene->getSkeletonManager();

    const std::shared_ptr<CameraManager>& camera_manager = scene->getCameraManager();
    const std::shared_ptr<BaseEngineLogic>& game_logic = app.GetGameLogic();
    const std::shared_ptr<CameraComponent>& camera_component = game_logic->GetHumanView()->VGetCamera();
    const std::shared_ptr<BasicCameraNode>& camera_node = camera_component->VGetCameraNode();
    
    std::shared_ptr<ValueBagNode> value_bag_node = std::dynamic_pointer_cast<ValueBagNode>(scene->getProperty(model->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_VALUE_BAG));
    const MeshNode::MeshList& mesh_list = model->GetMeshes();
    bool has_skeleton = model->GetSkinName().size() > 0u;
    size_t msz = mesh_list.size();
    for(int frame = 0; frame < m_max_frames; ++frame) {
        m_per_frame[frame]->renderables.reserve(m_per_frame[frame]->renderables.size() + msz);
    }
    for (size_t i = 0u; i < msz; ++i) {
        const std::shared_ptr<ModelData>& model_data = mesh_list.at(i);
        const std::shared_ptr<Material>& material = model_data->GetMaterial();

        for(int frame = 0; frame < m_max_frames; ++frame) {
            std::shared_ptr<PerFrame>& global_frame_data = renderer.getFrameData(frame);
            std::shared_ptr<RenderPerFrame>& per_frame_data = m_per_frame[frame];
            size_t renderable_id = per_frame_data->renderables.size();
            std::shared_ptr<RenderGraph>& frame_render_graph = global_frame_data->render_graph;

            std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>();
            per_frame_data->renderables.push_back(renderable);

            renderable->mesh_id = i;

            std::string render_name = makeRenderNodeName(material);
            if(!frame_render_graph->hasGraphicsRenderNodeConfig(render_name)) {
                render_name = "mesh_render"s;
            }

            std::shared_ptr<GraphicsRenderNode> render_node = std::make_shared<GraphicsRenderNode>();
            std::string render_node_name = model_data->GetName() + "GraphicsRenderNode"s;
            render_node->init(render_node_name, frame, m_device, render_name, false, frame_render_graph);

            const std::shared_ptr<GraphicsRenderNodeConfig>& render_node_cfg = render_node->getGraphicsRenderNodeConfig();
            const std::shared_ptr<VulkanPipeline>& pipeline = render_node->getPipeline();
            const std::shared_ptr<VulkanShader>& vertex_shader = pipeline->getShader(VK_SHADER_STAGE_VERTEX_BIT);
            const std::shared_ptr<ShaderSignature>& shader_signature = vertex_shader->getShaderSignature();
            //VertexFormat::BindingNum vertex_binding = shader_signature->getFirstInputAttribute([](const VertexFormat& vf){ return vf.getInputRate() == VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX; });
            //VertexFormat::BindingNum instance_binding = shader_signature->getFirstInputAttribute([](const VertexFormat& vf){ return vf.getInputRate() == VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE; });

            renderable->mesh_node = model;

            for(const VertexFormat& vf : shader_signature->getInputAttributes()) {
                VertexFormat::BindingNum binding_num = vf.getBindingNum();
                const std::shared_ptr<VulkanBuffer>& vertex_buffer = model_data->GetVertexBuffer(binding_num);
                vertex_buffer->AddUpdateMetadataFn(
                    [render_node, &shader_signature, binding_num]
                    (const std::shared_ptr<RenderResource>& bo) {
                        render_node->changeReadDependency(bo, shader_signature->getInputAttributes(binding_num).getVertexBufferBindingName());
                    }
                );
                render_node->addReadDependency(vertex_buffer, shader_signature->getInputAttributes(binding_num).getVertexBufferBindingName());
            }

            if(vertex_shader && shader_signature->getPushConstants()) {
                renderable->const_params.push_back(shader_signature->getPushConstants());
            }

            std::shared_ptr<VulkanShader> pixel_shader = pipeline->getShader(VK_SHADER_STAGE_FRAGMENT_BIT);
            if(pixel_shader && pixel_shader->getShaderSignature()->getPushConstants()) {
                renderable->const_params.push_back(pixel_shader->getShaderSignature()->getPushConstants());
            }

            if(value_bag_node && renderable->const_params.size() > 0u) {
                updatePushConstants(frame, renderable_id);
            }
            
            if(const std::shared_ptr<VulkanBuffer>& index_buffer = model_data->GetIndexBuffer()) {
                index_buffer->AddUpdateMetadataFn(
                    [render_node, &shader_signature]
                    (const std::shared_ptr<RenderResource>& bo) {
                        render_node->changeReadDependency(bo, shader_signature->getIndexBufferBindingName());
                    }
                );
                render_node->addReadDependency(index_buffer, shader_signature->getIndexBufferBindingName());
            }

            render_node->add_update_function(
                "model_matrices_update"s,
                [&, frame, renderable_id]
                (std::shared_ptr<VulkanBuffer>& uniform_buffer, const std::string& desc_set_layout_bind_name){
                    updateModelMatrices(frame, renderable_id, uniform_buffer, desc_set_layout_bind_name);
                }
            );

            render_node->add_update_function(
                "material_prop_update"s,
                [&, frame, renderable_id]
                (std::shared_ptr<VulkanBuffer>& uniform_buffer, const std::string& desc_set_layout_bind_name){
                    updateMaterialProps(frame, renderable_id, uniform_buffer, desc_set_layout_bind_name);
                }
            );

            camera_manager->DecorateRenderNode(render_node, camera_node->Get().Name());
            if(has_skeleton) {
                skeleton_manager->DecorateRenderNode(render_node, model->GetSkinName());
            }

            render_node->addWriteDependency(swapchain_images[frame], "resolve_attachment");
            render_node->addWriteDependency(renderer.getOutColorImage(frame), "color_attachment");
            render_node->addWriteDependency(renderer.getOutDepthImage(frame), "depth_attachment");
            render_node->finishRenderNode();

            renderable->render_node = render_node;
            renderer.addRenderNode(std::move(render_node), frame);
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

void SceneDrawable::updateDescBuffer(int frame, RenderableId render_id, std::shared_ptr<VulkanBuffer>& uniform_buffer, const std::string& desc_set_layout_bind_name, const void* src_data, VkDeviceSize buffer_size, std::string object_name) {
    if(uniform_buffer) {
        uniform_buffer->update(src_data, buffer_size);
        return;
    }
    
    // проверить размер и обновить буффер и дескрипторы
    VulkanRenderer& renderer = Application::GetRenderer();
    std::shared_ptr<VulkanResourcesManager>& resources_manager = renderer.getResourcesManager();
    const std::shared_ptr<GraphicsRenderNode>& render_node = m_per_frame[frame]->renderables.at(render_id)->render_node;
    std::shared_ptr<GraphicsRenderNodeConfig>& render_node_cfg = render_node->getGraphicsRenderNodeConfig();
    const std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>& update_metadata = render_node_cfg->getBindingsMetadata().at(desc_set_layout_bind_name);
    const std::shared_ptr<VulkanDescriptor>& desc = render_node->getDescriptorByLayoutName(desc_set_layout_bind_name);
    const std::shared_ptr<DescSetLayout>& desc_set_layout = desc->getBindings();
    VkDescriptorSetLayoutBinding vk_desc_bind = desc_set_layout->getBinding(desc_set_layout_bind_name);
    std::string mvp_buffer_name;
    if(vk_desc_bind.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
        mvp_buffer_name = object_name + "_"s + desc_set_layout_bind_name + "_uniform_frame_"s + std::to_string(frame);
    }
    else if(vk_desc_bind.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        mvp_buffer_name = object_name + "_"s + desc_set_layout_bind_name + "_storage_frame_"s + std::to_string(frame);
    }
    std::shared_ptr<VulkanBuffer> bo = resources_manager->create_buffer(src_data, buffer_size, mvp_buffer_name, update_metadata->buffer_resource_type_name);
    bo->AddUpdateMetadataFn(
        [render_node, desc_set_layout_bind_name]
        (const std::shared_ptr<RenderResource>& bo){
            render_node->changeReadDependency(bo, desc_set_layout_bind_name);
            render_node->updateDescriptor(desc_set_layout_bind_name);
        }
    );
    //bo->update(src_data, buffer_size);
    render_node->addReadDependency(std::move(bo), desc_set_layout_bind_name);
    render_node->updateDescriptor(desc_set_layout_bind_name);
}

void SceneDrawable::updateModelMatrices(int frame, RenderableId render_id, std::shared_ptr<VulkanBuffer>& uniform_buffer, const std::string& desc_set_layout_bind_name) {
    const std::shared_ptr<MeshNode>& mesh_node = m_per_frame[frame]->renderables.at(render_id)->mesh_node;
    const SceneNodeProperties& node_props = mesh_node->Get();
    
    ModelUniformBufferObject ubo{};
    ubo.model = node_props.ToRoot();
    ubo.inv_model = node_props.FromRoot();

    updateDescBuffer(frame, render_id, uniform_buffer, desc_set_layout_bind_name, &ubo, sizeof(ModelUniformBufferObject), mesh_node->Get().Name());
}

void SceneDrawable::updateMaterialProps(int frame, RenderableId render_id, std::shared_ptr<VulkanBuffer>& uniform_buffer, const std::string& desc_set_layout_bind_name) {
    const std::shared_ptr<Renderable>& renderable = m_per_frame[frame]->renderables.at(render_id);
    const std::shared_ptr<MeshNode>& mesh_node = renderable->mesh_node;
    std::shared_ptr<ModelData> model_data = mesh_node->GetMesh(renderable->mesh_id);
    const std::shared_ptr<Material>& material = model_data->GetMaterial();

    PhongMaterial mat;
    mat.fresnelR0_roughness = material->GetReflectance();
    mat.fresnelR0_roughness.a = material->GetRoughnessFactor();

    updateDescBuffer(frame, render_id, uniform_buffer, desc_set_layout_bind_name, &mat, sizeof(PhongMaterial), mesh_node->Get().Name());
}