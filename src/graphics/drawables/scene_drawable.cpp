#include "scene_drawable.h"

#include "../../application.h"
#include "../../engine/base_engine_logic.h"
#include "../../actors/camera_component.h"
#include "../../scene/nodes/basic_camera_node.h"
#include "../api/vulkan_buffer.h"
#include "../api/vulkan_image_buffer.h"
#include "../api/vulkan_swapchain.h"
#include "../api/vulkan_resources_manager.h"
#include "../api/vulkan_descriptors_manager.h"
#include "../pod/render_node.h"
#include "../pod/render_node_config.h"
#include "../pod/format_config.h"
#include "../vulkan_renderer.h"

struct SceneUniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

bool SceneDrawable::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<Managers>& managers, int max_frames) {
    m_device = std::move(device);
    m_max_frames = max_frames;
    m_rt_aspect = Application::GetRenderer().getSwapchain()->getFormatConfig()->getAspect();
    m_viewport_extent = Application::GetRenderer().getSwapchain()->getFormatConfig()->getExtent2D();

    return true;
}

void SceneDrawable::reset() {

}

void SceneDrawable::destroy() {

}

void SceneDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    size_t sz = m_mesh_nodes.size();
    if(!sz) return;

    Application& app = Application::Get();
    std::shared_ptr<BaseEngineLogic> game_logic = app.GetGameLogic();
    std::shared_ptr<CameraComponent> camera_component = game_logic->GetHumanView()->VGetCamera();
    std::shared_ptr<BasicCameraNode> camera_node = camera_component->VGetCameraNode();
    for(size_t i = 0u; i < sz; ++i) {
        std::shared_ptr<MeshNode> mesh_node = m_mesh_nodes.at(i);
        std::shared_ptr<Renderable> renderable = m_renderables.at(i);
        const SceneNodeProperties& node_props = mesh_node->Get();

        SceneUniformBufferObject ubo{};
        ubo.model = node_props.ToRoot();
        ubo.view = camera_node->GetView();
        ubo.proj = camera_node->GetProjection();
        //ubo.proj[1][1] *= -1.0f;

        renderable->uniform_buffers.at(image_index)->update(&ubo, sizeof(SceneUniformBufferObject));
    }
}

int SceneDrawable::order() {
    return 0;
}

std::shared_ptr<RenderNodeConfig> getMeshRenderNodeConfig(const std::shared_ptr<VulkanDevice>& device) {
    std::shared_ptr<RenderNodeConfig> render_node_config = std::make_shared<RenderNodeConfig>();

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file("graphics_pipelines.xml");
	if (!parse_res) { return nullptr;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return nullptr; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node render_nodes_node = root_node.child("RenderNodes");
	if (!render_nodes_node) return nullptr;

    render_node_config->init(device, render_nodes_node.child("mesh_render"));

    return render_node_config;
}

void SceneDrawable::addRendeNode(std::shared_ptr<MeshNode> model, std::shared_ptr<Managers>& managers) {
    const MeshNode::MeshList& mesh_list = model->GetMeshes();
    std::shared_ptr<RenderNodeConfig> render_node_config = getMeshRenderNodeConfig(m_device);
    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();
    std::vector<std::shared_ptr<VulkanImageBuffer>>& color_images = Application::GetRenderer().getOutColorImages();
    std::vector<std::shared_ptr<VulkanImageBuffer>>& depth_images = Application::GetRenderer().getOutDepthImages();
    size_t msz = mesh_list.size();
    for (size_t i = 0u; i < msz; ++i) {
        m_mesh_nodes.push_back(model);
        std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>();
        std::shared_ptr<ModelData> model_data = mesh_list.at(i);
        std::shared_ptr<Material> material = model_data->GetMaterial();

        renderable->render_nodes[i] = std::make_shared<RenderNode>();
        renderable->render_nodes[i]->init(m_device, render_node_config);

        renderable->texture = material->GetTexture();

        renderable->uniform_buffers.resize(m_max_frames);
        
        std::shared_ptr<VulkanBuffer> ubo = managers->resources_manager->create_buffer(nullptr, 0, "basic_uniform_resource");
        renderable->uniform_buffers[i] = std::move(ubo);
        
        renderable->vertex_buffer = model_data->GetVertexBuffer();
        renderable->index_buffer = model_data->GetIndexBuffer();

        m_renderables.push_back(std::move(renderable));

        std::shared_ptr<VulkanShader> vertex_shader = renderable->render_nodes[i]->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
        std::shared_ptr<DescSetLayout> desc_set_layout = managers->descriptors_manager->getDescSetLayout(vertex_shader->getShaderSignature()->getDescSetNames()[0]);

        renderable->render_nodes[i]->addReadDependency(renderable->vertex_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getVertexBufferBindingName());
        renderable->render_nodes[i]->addReadDependency(renderable->index_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getIndexBufferBindingName());
        renderable->render_nodes[i]->addReadDependency(renderable->uniform_buffers[i], desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
        renderable->render_nodes[i]->addReadDependency(renderable->texture, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
        renderable->render_nodes[i]->addWriteDependency(swapchain_images[i], "resolve_attachment");
        renderable->render_nodes[i]->addWriteDependency(color_images[i], "color_attachment");
        renderable->render_nodes[i]->addWriteDependency(depth_images[i], "depth_attachment");

        renderable->render_nodes[i]->finishRenderNode();
        Application::GetRenderer().addRenderNode(renderable->render_nodes[i]);
    }
}