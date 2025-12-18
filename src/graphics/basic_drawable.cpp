#include "basic_drawable.h"

#include "basic_vertex.h"
#include "basic_uniform.h"

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

VulkanPipeline::PipelineCfg BasicDrawable::createPipelineCfg(const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples) {
    VulkanPipeline::PipelineCfg pipeline_cfg = {};

    pipeline_cfg.dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    pipeline_cfg.desc_set_layouts = desc_set_layouts;
    pipeline_cfg.render_pass = render_pass;
    pipeline_cfg.viewport_extent = viewport_extent;
    pipeline_cfg.shaders_info = std::move(shaders_info);
    pipeline_cfg.vertex_input_info = vertex_input_info;
    pipeline_cfg.msaa_samples = msaa_samples;

    pipeline_cfg.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipeline_cfg.depth_stencil_info.depthTestEnable = VK_TRUE;
    pipeline_cfg.depth_stencil_info.depthWriteEnable = VK_TRUE;
    pipeline_cfg.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
    pipeline_cfg.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    pipeline_cfg.depth_stencil_info.minDepthBounds = 0.0f;
    pipeline_cfg.depth_stencil_info.maxDepthBounds = 1.0f;
    pipeline_cfg.depth_stencil_info.stencilTestEnable = VK_FALSE;
    pipeline_cfg.depth_stencil_info.front = {};
    pipeline_cfg.depth_stencil_info.back = {};

    pipeline_cfg.rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipeline_cfg.rasterizer_info.depthBiasClamp = VK_FALSE;
    pipeline_cfg.rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
    pipeline_cfg.rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
    pipeline_cfg.rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
    //m_pipeline_cfg.rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipeline_cfg.rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipeline_cfg.rasterizer_info.depthBiasEnable = VK_FALSE;
    pipeline_cfg.rasterizer_info.depthBiasConstantFactor = 0.0f;
    pipeline_cfg.rasterizer_info.depthBiasClamp = 0.0f;
    pipeline_cfg.rasterizer_info.depthBiasSlopeFactor = 0.0f;
    pipeline_cfg.rasterizer_info.lineWidth = 1.0f;

    pipeline_cfg.color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipeline_cfg.color_blend_state.blendEnable = VK_FALSE;
    pipeline_cfg.color_blend_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    pipeline_cfg.color_blend_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipeline_cfg.color_blend_state.colorBlendOp = VK_BLEND_OP_ADD;
    pipeline_cfg.color_blend_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipeline_cfg.color_blend_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipeline_cfg.color_blend_state.alphaBlendOp = VK_BLEND_OP_ADD;

    return pipeline_cfg;
}

bool BasicDrawable::init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt, int max_frames) {
    m_device = std::move(device);
    m_rt_aspect = rt.getAspect();
    m_max_frames = max_frames;

    m_texture.init(m_device, "textures/texture.jpg");

    std::vector<VulkanDescriptor::Binding> binding(max_frames);
    m_vertex_buffers.resize(max_frames);
    m_uniform_buffers.resize(max_frames);
    for(size_t i = 0u; i < max_frames; ++i) {
        std::shared_ptr<VertexBuffer> vertex_buffer = std::make_shared<VertexBuffer>();
        vertex_buffer->init<Vertex, uint16_t>(m_device, g_vertices, g_indices, Vertex::getVertextInputInfo());
        m_vertex_buffers[i] = std::move(vertex_buffer);

        std::shared_ptr<VulkanUniformBuffer> uniform_buffer = std::make_shared<VulkanUniformBuffer>();
        uniform_buffer->init<UniformBufferObject>(m_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        m_uniform_buffers[i] = std::move(uniform_buffer);

        binding[i].resize(2u);
        binding[i][0u].layout_binding.binding = 0u;
        binding[i][0u].layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding[i][0u].layout_binding.descriptorCount = 1u;
        binding[i][0u].layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        binding[i][0u].layout_binding.pImmutableSamplers = nullptr;
        binding[i][0u].buffer_info = std::make_shared<VkDescriptorBufferInfo>(m_uniform_buffers[i]->getDescBufferInfo());
        
        binding[i][1u].layout_binding.binding = 1u;
        binding[i][1u].layout_binding.descriptorCount = 1u;
        binding[i][1u].layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding[i][1u].layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding[i][1u].layout_binding.pImmutableSamplers = nullptr;
        binding[i][1u].image_info = std::make_shared<VkDescriptorImageInfo>(m_texture.getDescImageInfo());
        binding[i][1u].sampler = m_texture.getSampler();
    }

    m_descriptor.init(m_device->getDevice(), std::move(binding), max_frames);

    m_frag_shader.init(m_device->getDevice(), "shaders/shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_vert_shader.init(m_device->getDevice(), "shaders/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shaders_info;
    pipeline_shaders_info.reserve(2u);
    pipeline_shaders_info.push_back(m_frag_shader.getShaderInfo());
    pipeline_shaders_info.push_back(m_vert_shader.getShaderInfo());

    m_pipelines = std::vector<GraphicsPipeline>(2u);
    m_pipelines[0].pipeline_cfg = createPipelineCfg(
        {m_descriptor.getDescLayouts()},
        rt.getRenderPass(RenderTarget::AttachmentLoadOp::LOAD),
        rt.getViewportExtent(),
        pipeline_shaders_info,
        m_vertex_buffers[0]->getVertextInputInfo(),
        m_device->getMsaaSamples()
    );
    m_pipelines[0].pipeline.init(m_device->getDevice(), m_pipelines[0].pipeline_cfg);

    m_pipelines[1].pipeline_cfg = createPipelineCfg(
        {m_descriptor.getDescLayouts()},
        rt.getRenderPass(RenderTarget::AttachmentLoadOp::CLEAR),
        rt.getViewportExtent(),
        pipeline_shaders_info,
        m_vertex_buffers[0]->getVertextInputInfo(),
        m_device->getMsaaSamples()
    );
    m_pipelines[1].pipeline.init(m_device->getDevice(), m_pipelines[1].pipeline_cfg);

    return true;
}

void BasicDrawable::reset(const RenderTarget& rt) {
    std::vector<VulkanDescriptor::Binding> bindings = m_descriptor.getBingingsForSets();
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shaders_info = m_pipelines[0].pipeline.getShadersInfo();

    m_descriptor.destroy();
    m_pipelines[0].pipeline.destroy();
    m_pipelines[1].pipeline.destroy();

    m_rt_aspect = rt.getAspect();

    m_descriptor.init(m_device->getDevice(), std::move(bindings), m_max_frames);

    m_pipelines[0].pipeline_cfg = createPipelineCfg(
        {m_descriptor.getDescLayouts()},
        rt.getRenderPass(RenderTarget::AttachmentLoadOp::LOAD),
        rt.getViewportExtent(),
        pipeline_shaders_info,
        m_vertex_buffers[0]->getVertextInputInfo(),
        m_device->getMsaaSamples()
    );
    m_pipelines[0].pipeline.init(m_device->getDevice(), m_pipelines[0].pipeline_cfg);

    m_pipelines[1].pipeline_cfg = createPipelineCfg(
        {m_descriptor.getDescLayouts()},
        rt.getRenderPass(RenderTarget::AttachmentLoadOp::CLEAR),
        rt.getViewportExtent(),
        pipeline_shaders_info,
        m_vertex_buffers[0]->getVertextInputInfo(),
        m_device->getMsaaSamples()
    );
    m_pipelines[1].pipeline.init(m_device->getDevice(), m_pipelines[1].pipeline_cfg);
}

void BasicDrawable::destroy() {
    m_pipelines[0].pipeline.destroy();
    m_pipelines[1].pipeline.destroy();
    m_descriptor.destroy();
    m_vert_shader.destroy();
    m_frag_shader.destroy();

    m_texture.destroy();

    for(size_t i = 0u; i < m_max_frames; ++i) {
        m_vertex_buffers[i]->destroy();
        m_uniform_buffers[i]->destroy();
    }
}

void BasicDrawable::recordCommandBuffer(CommandBatch& command_buffer, const RenderTarget& rt, uint32_t image_index) {

    auto current_pipeline = std::underlying_type<RenderTarget::AttachmentLoadOp>::type(rt.getCurrentLoadOp());

    VkRenderPassBeginInfo renderpass_info{};
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_info.renderPass = m_pipelines[current_pipeline].pipeline_cfg.render_pass;
    renderpass_info.framebuffer = rt.getFrameBuffer();
    renderpass_info.renderArea.offset = {0, 0};
    renderpass_info.renderArea.extent = rt.getViewportExtent();
    
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};
    renderpass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    renderpass_info.pClearValues = clear_values.data();

    VkViewport view_port{};
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = static_cast<float>(rt.getViewportExtent().width);
    view_port.height = static_cast<float>(rt.getViewportExtent().height);
    view_port.minDepth = 0.0f;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = rt.getViewportExtent();

    vkCmdBeginRenderPass(command_buffer.getCommandBufer(), &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
        
    vkCmdSetViewport(command_buffer.getCommandBufer(), 0u, 1u, &view_port);
    vkCmdSetScissor(command_buffer.getCommandBufer(), 0u, 1u, &scissor);

    vkCmdBindDescriptorSets(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[current_pipeline].pipeline.getPipelineLayout(), 0, 1, &m_descriptor.getDescriptorSets()[image_index], 0, nullptr);
        
    vkCmdBindPipeline(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[current_pipeline].pipeline.getPipeline());

    VkBuffer vertex_buffers[] = {m_vertex_buffers[image_index]->getVertexBuffer()->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer.getCommandBufer(), 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer.getCommandBufer(), m_vertex_buffers[image_index]->getIndexBuffer()->getBuffer(), 0u, m_vertex_buffers[image_index]->getIndexType());

    vkCmdDrawIndexed(command_buffer.getCommandBufer(), static_cast<uint32_t>(m_vertex_buffers[image_index]->getIndicesCount()), 1u, 0u, 0u, 0u);

    vkCmdEndRenderPass(command_buffer.getCommandBufer());
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
}

int BasicDrawable::order() {
    return 0;
}