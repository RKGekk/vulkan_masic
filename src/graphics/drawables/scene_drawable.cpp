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

bool SceneDrawable::init(std::shared_ptr<VulkanDevice> device, int max_frames) {
    m_device = std::move(device);
    m_max_frames = max_frames;
    m_rt_aspect = Application::GetRenderer().getSwapchain()->getFormatConfig()->getAspect();
    m_viewport_extent = Application::GetRenderer().getSwapchain()->getFormatConfig()->getExtent2D();

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
    size_t sz = m_renderables.size();
    if(!sz) return;

    Application& app = Application::Get();
    std::shared_ptr<BaseEngineLogic> game_logic = app.GetGameLogic();
    std::shared_ptr<CameraComponent> camera_component = game_logic->GetHumanView()->VGetCamera();
    std::shared_ptr<BasicCameraNode> camera_node = camera_component->VGetCameraNode();
    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();
    for(size_t render_id = 0u; render_id < sz; ++render_id) {
        const std::shared_ptr<Renderable>& renderable = m_renderables.at(render_id);
        if(renderable->frame != image_index) continue;

        const std::shared_ptr<MeshNode>& mesh_node = renderable->mesh_node;
        const SceneNodeProperties& node_props = mesh_node->Get();

        SceneUniformBufferObject ubo{};
        ubo.model = node_props.ToRoot();
        ubo.view = camera_node->GetView();
        ubo.proj = camera_node->GetProjection();
        //ubo.proj[1][1] *= -1.0f;

        renderable->uniform_buffer->update(&ubo, sizeof(SceneUniformBufferObject));

        //renderable->render_node->changeWriteDependency(swapchain_images[image_index], "resolve_attachment");
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

    for (pugi::xml_node render_node = render_nodes_node.first_child(); render_node; render_node = render_node.next_sibling()) {
        std::string node_name = render_node.attribute("name").as_string();
        if(node_name == "mesh_render"s) {
            std::shared_ptr<RenderNodeConfig> render_node_config = std::make_shared<RenderNodeConfig>();
            render_node_config->init(device, Application::GetRenderer().getResourcesManager(), render_node);
            return render_node_config;
        }
    }

    return nullptr;
}

void SceneDrawable::addRendeNode(std::shared_ptr<MeshNode> model) {
    using namespace std::literals;

    std::shared_ptr<RenderNodeConfig> render_node_config = getMeshRenderNodeConfig(m_device);
    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();

    const MeshNode::MeshList& mesh_list = model->GetMeshes();
    size_t msz = mesh_list.size();
    m_renderables.reserve(m_renderables.size() + msz);
    for (size_t i = 0u; i < msz; ++i) {
        std::shared_ptr<ModelData> model_data = mesh_list.at(i);
        std::shared_ptr<Material> material = model_data->GetMaterial();

        for(int frame = 0; frame < m_max_frames; ++frame) {
            std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>();
            renderable->frame = frame;
            renderable->mesh_node = model;
            
            renderable->texture = material->GetTexture();

            std::shared_ptr<VulkanBuffer> ubo = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, model_data->GetName() + "_uniform_frame_"s + std::to_string(frame), "basic_uniform_resource");
            renderable->uniform_buffer = std::move(ubo);

            renderable->vertex_buffer = model_data->GetVertexBuffer();
            renderable->index_buffer = model_data->GetIndexBuffer();

            renderable->render_node = std::make_shared<RenderNode>();
            renderable->render_node->init(m_device, render_node_config);

            std::shared_ptr<VulkanShader> vertex_shader = renderable->render_node->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
            std::shared_ptr<DescSetLayout> desc_set_layout = Application::GetRenderer().getDescriptorsManager()->getDescSetLayout(vertex_shader->getShaderSignature()->getDescSetNames().at(0));
            renderable->render_node->addReadDependency(renderable->vertex_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getVertexBufferBindingName());
            renderable->render_node->addReadDependency(renderable->index_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getIndexBufferBindingName());
            renderable->render_node->addReadDependency(renderable->uniform_buffer, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
            renderable->render_node->addReadDependency(renderable->texture, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
            renderable->render_node->addWriteDependency(swapchain_images[i], "resolve_attachment");
            renderable->render_node->addWriteDependency(Application::GetRenderer().getOutColorImage(i), "color_attachment");
            renderable->render_node->addWriteDependency(Application::GetRenderer().getOutDepthImage(i), "depth_attachment");
            renderable->render_node->finishRenderNode();

            m_renderables.push_back(renderable);
            Application::GetRenderer().addRenderNode(renderable->render_node, frame);
        }
    }
}