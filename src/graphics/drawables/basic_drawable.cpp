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

    m_device = std::move(device);
    m_rt_aspect = Application::GetRenderer().getSwapchain()->getFormatConfig()->getAspect();
    m_max_frames = max_frames;

    m_texture = Application::GetRenderer().getResourcesManager()->create_image("textures/texture.jpg");
    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();

    m_render_nodes.resize(max_frames);
    m_vertex_buffers.resize(max_frames);
    m_uniform_buffers.resize(max_frames);
    for(size_t i = 0u; i < max_frames; ++i) {

        m_vertex_buffers[i] = Application::GetRenderer().getResourcesManager()->create_buffer(g_vertices.data(), g_vertices.size() * sizeof(Vertex), "basic_vertex_resource");
        m_index_buffers[i] = Application::GetRenderer().getResourcesManager()->create_buffer(g_indices.data(), g_indices.size() * sizeof(uint16_t), "basic_index_resource");
        m_uniform_buffers[i] = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, "basic_uniform_resource");


        m_render_nodes[i] = std::make_shared<GraphicsRenderNode>();
        m_render_nodes[i]->init(device, "mesh_render"s, Application::GetRenderer().getFrameData(i)->render_graph);

        std::shared_ptr<VulkanShader> vertex_shader = m_render_nodes[i]->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
        std::shared_ptr<DescSetLayout> desc_set_layout = Application::GetRenderer().getDescriptorsManager()->getDescSetLayout(vertex_shader->getShaderSignature()->getDescSetNames().at(0));

        m_render_nodes[i]->addReadDependency(m_vertex_buffers[i], vertex_shader->getShaderSignature()->getVertexFormat().getVertexBufferBindingName());
        m_render_nodes[i]->addReadDependency(m_index_buffers[i], vertex_shader->getShaderSignature()->getVertexFormat().getIndexBufferBindingName());
        m_render_nodes[i]->addReadDependency(m_uniform_buffers[i], desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
        m_render_nodes[i]->addReadDependency(m_texture, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
        m_render_nodes[i]->addWriteDependency(swapchain_images[i], "resolve_attachment");
        m_render_nodes[i]->addWriteDependency(Application::GetRenderer().getOutColorImage(i), "color_attachment");
        m_render_nodes[i]->addWriteDependency(Application::GetRenderer().getOutDepthImage(i), "depth_attachment");

        m_render_nodes[i]->finishRenderNode();
        Application::GetRenderer().addRenderNode(m_render_nodes[i], i);
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
    float angle = delta.fGetTotalSeconds() * glm::radians(90.f);
    glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), angle, rotation_axis);
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), m_rt_aspect, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1.0f;

    m_uniform_buffers[image_index]->update(&ubo, sizeof(UniformBufferObject));

    //const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();
    //m_render_nodes[image_index]->changeWriteDependency(swapchain_images[image_index], "resolve_attachment");
}

int BasicDrawable::order() {
    return 0;
}