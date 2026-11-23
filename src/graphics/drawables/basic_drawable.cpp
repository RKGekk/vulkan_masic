#include "basic_drawable.h"

#include "../pod/basic_vertex.h"
#include "../pod/basic_uniform.h"
#include "../pod/graphics_render_node_config.h"
#include "../pod/format_config.h"
#include "../pod/graphics_render_node.h"
#include "../vulkan_renderer.h"
#include "../api/vulkan_swapchain.h"
#include "../api/vulkan_resources_manager.h"
#include "../api/vulkan_descriptors_manager.h"
#include "../../application.h"

#include <utility>

const std::vector<Vertex> g_vertices = {
    {{-0.5f, -0.5f,  0.0f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.0f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.0f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  1.0f}},
    {{-0.5f,  0.5f,  0.0f}, { 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f}},

    {{-0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  1.0f}},
    {{-0.5f,  0.5f, -0.5f}, { 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f}}
};

const std::vector<uint16_t> g_indices = {
    0, 1, 2,
    2, 3, 0,
    4, 5, 6,
    6, 7, 4
};

bool BasicDrawable::init(std::shared_ptr<VulkanDevice> device, int max_frames) {
    using namespace std::literals;

    Application& app = Application::Get();
    VulkanRenderer& renderer = Application::GetRenderer();

    m_device = std::move(device);
    m_rt_aspect = renderer.getSwapchain()->getFormatConfig()->getAspect();
    m_max_frames = max_frames;

    m_texture = renderer.makeTransferImgCommand("textures/texture.jpg");
    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();

    m_render_nodes.resize(max_frames);
    m_vertex_buffers.resize(max_frames);
    m_uniform_buffers.resize(max_frames);
    for(size_t i = 0u; i < max_frames; ++i) {
        std::shared_ptr<PerFrame>& global_frame_data = renderer.getFrameData(i);
        std::shared_ptr<RenderGraph>& frame_render_graph = global_frame_data->render_graph;

        std::string basic_render_node_name = "BasicGraphicsRenderNode"s + std::to_string(i);
        std::shared_ptr<GraphicsRenderNodeConfig> render_node_cfg = frame_render_graph->makeGraphicsRenderNodeCfgInstance("mesh_render"s, basic_render_node_name);

        std::shared_ptr<GraphicsRenderNode> render_node = std::make_shared<GraphicsRenderNode>();
        render_node->init("BasicGraphicsRenderNode"s + std::to_string(i), render_node_cfg, frame_render_graph);

        const std::shared_ptr<VulkanShader>& vertex_shader = render_node->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
        const std::shared_ptr<ShaderSignature>& shader_signature = vertex_shader->getShaderSignature();
        VertexFormat::BindingNum vertex_binding = shader_signature->getFirstInputAttribute([](const VertexFormat& vf){ return vf.getInputRate() == VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX; });
        std::shared_ptr<DescSetLayout> desc_set_layout = Application::GetRenderer().getDescriptorsManager()->getDescSetLayout(vertex_shader->getShaderSignature()->getDescSetNames().at(0));

        std::shared_ptr<VulkanBuffer> vertex_buffer = renderer.makeTransferCommand("basic_graphics_render_vertex_buffer_frame"s + std::to_string(i), "basic_vertex_resource", g_vertices.data(), g_vertices.size() * sizeof(Vertex), i);
        render_node->addReadDependency(vertex_buffer, shader_signature->getInputAttributes(vertex_binding).getVertexBufferBindingName());
        m_vertex_buffers[i] = std::move(vertex_buffer);

        std::shared_ptr<VulkanBuffer> index_buffer = renderer.makeTransferCommand("basic_graphics_render_index_buffer_frame"s + std::to_string(i), "basic_index_resource", g_indices.data(), g_indices.size() * sizeof(uint16_t), i);
        render_node->addReadDependency(m_index_buffers[i], shader_signature->getIndexBufferBindingName());
        m_index_buffers[i] = std::move(index_buffer);

        std::shared_ptr<VulkanBuffer> other_buffer = renderer.getResourcesManager()->create_buffer("basic_graphics_render_uniform_buffer_frame"s + std::to_string(i), "basic_uniform_resource", 0u);
        render_node->addReadDependency(m_uniform_buffers[i], desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
        other_buffer->AddUpdateMetadataFn(
            [render_node, &desc_set_layout_bind_name = desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)]
            (const std::shared_ptr<RenderResource>& bo){
                render_node->changeReadDependency(bo, desc_set_layout_bind_name);
                render_node->updateDescriptor(desc_set_layout_bind_name);
            }
        );
        m_uniform_buffers[i] = std::move(other_buffer);

        render_node->addReadDependency(m_texture, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
        render_node->addWriteDependency(swapchain_images[i], "resolve_attachment");
        render_node->addWriteDependency(Application::GetRenderer().getOutColorImage(i), "color_attachment");
        render_node->addWriteDependency(Application::GetRenderer().getOutDepthImage(i), "depth_attachment");

        render_node->finishRenderNode();
        Application::GetRenderer().addRenderNode(render_node, i);
        m_render_nodes[i] = std::move(render_node);
    }

    return true;
}

void BasicDrawable::destroy() {
    // size_t sz = m_uniform_buffers.size();
    // m_texture->destroy();
    // for(size_t i = 0u; i < sz; ++i) {
    //     m_uniform_buffers[i]->destroy();
    //     m_vertex_buffers[i]->destroy();
    //     m_index_buffers[i]->destroy();
    //     //m_render_nodes[i]->destroy();
    // }
}

void BasicDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    Application& app = Application::Get();
    VulkanRenderer& renderer = app.GetRenderer();

    float angle = delta.fGetTotalSeconds() * glm::radians(90.f);
    glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), angle, rotation_axis);
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), m_rt_aspect, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1.0f;

    renderer.updateCommand(m_uniform_buffers[image_index], &ubo, sizeof(UniformBufferObject), image_index);
}

int BasicDrawable::order() {
    return 0;
}