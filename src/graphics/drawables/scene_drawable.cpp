#include "scene_drawable.h"

#include "../../application.h"
#include "../../engine/base_engine_logic.h"
#include "../../actors/camera_component.h"
#include "../../scene/nodes/basic_camera_node.h"

struct SceneUniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

bool SceneDrawable::init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt, int max_frames) {
    m_device = std::move(device);
    m_max_frames = max_frames;
    m_rt_aspect = rt.getAspect();

    m_clear_render_pass = rt.getRenderPass(RenderTarget::AttachmentLoadOp::CLEAR);
    m_load_render_pass = rt.getRenderPass(RenderTarget::AttachmentLoadOp::LOAD);
    m_viewport_extent = rt.getViewportExtent();

    m_frag_shader = std::make_shared<VulkanShader>();
    m_frag_shader->init(m_device->getDevice(), "shaders/shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    m_vert_shader = std::make_shared<VulkanShader>();
    m_vert_shader->init(m_device->getDevice(), "shaders/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);

    return true;
}

void SceneDrawable::reset(const RenderTarget& rt) {

    m_rt_aspect = rt.getAspect();
    m_clear_render_pass = rt.getRenderPass(RenderTarget::AttachmentLoadOp::CLEAR);
    m_load_render_pass = rt.getRenderPass(RenderTarget::AttachmentLoadOp::LOAD);
    m_viewport_extent = rt.getViewportExtent();

    for (size_t k = 0; k < m_renderables.size(); ++k) {
        std::vector<VulkanDescriptor::Binding> bindings = m_renderables[k]->descriptor.getBingingsForSets();
        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shaders_info = m_renderables[k]->pipelines[0].pipeline.getShadersInfo();
        
        m_renderables[k]->descriptor.destroy();
        m_renderables[k]->pipelines[0].pipeline.destroy();
        m_renderables[k]->pipelines[1].pipeline.destroy();
        
        m_renderables[k]->descriptor.init(m_device->getDevice(), std::move(bindings), m_max_frames);
        
        m_renderables[k]->pipelines[0].pipeline_cfg = createPipelineCfg(
            {m_renderables[k]->descriptor.getDescLayouts()},
            rt.getRenderPass(RenderTarget::AttachmentLoadOp::LOAD),
            rt.getViewportExtent(),
            pipeline_shaders_info,
            m_renderables[k]->vertex_buffer->getVertextInputInfo(),
            m_device->getMsaaSamples()
        );
        m_renderables[k]->pipelines[0].pipeline.init(m_device->getDevice(), m_renderables[k]->pipelines[0].pipeline_cfg);

        m_renderables[k]->pipelines[1].pipeline_cfg = createPipelineCfg(
            {m_renderables[k]->descriptor.getDescLayouts()},
            rt.getRenderPass(RenderTarget::AttachmentLoadOp::CLEAR),
            rt.getViewportExtent(),
            pipeline_shaders_info,
            m_renderables[k]->vertex_buffer->getVertextInputInfo(),
            m_device->getMsaaSamples()
        );
        m_renderables[k]->pipelines[1].pipeline.init(m_device->getDevice(), m_renderables[k]->pipelines[1].pipeline_cfg);
    }
}

void SceneDrawable::destroy() {
    for (size_t k = 0; k < m_renderables.size(); ++k) {
        m_renderables[k]->pipelines[0].pipeline.destroy();
        m_renderables[k]->pipelines[1].pipeline.destroy();
        m_renderables[k]->descriptor.destroy();
        m_renderables[k]->vert_shader->destroy();
        m_renderables[k]->frag_shader->destroy();
        m_renderables[k]->texture->destroy();
        m_renderables[k]->vertex_buffer->destroy();
        for(size_t i = 0u; i < m_max_frames; ++i) {
            m_renderables[k]->uniform_buffers[i]->destroy();
        }
    }
}

void SceneDrawable::recordCommandBuffer(CommandBatch& command_buffer, const RenderTarget& rt, uint32_t frame_index) {
    auto current_pipeline = std::underlying_type<RenderTarget::AttachmentLoadOp>::type(rt.getCurrentLoadOp());

    for (size_t k = 0; k < m_renderables.size(); ++k) {
        VkRenderPassBeginInfo renderpass_info{};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_info.renderPass = m_renderables[k]->pipelines[current_pipeline].pipeline_cfg.render_pass;
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
        
        vkCmdBindDescriptorSets(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderables[k]->pipelines[current_pipeline].pipeline.getPipelineLayout(), 0, 1, &m_renderables[k]->descriptor.getDescriptorSets()[frame_index], 0, nullptr);

        vkCmdBindPipeline(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderables[k]->pipelines[current_pipeline].pipeline.getPipeline());
        
        VkBuffer vertex_buffers[] = {m_renderables[k]->vertex_buffer->getVertexBuffer()->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer.getCommandBufer(), 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(command_buffer.getCommandBufer(), m_renderables[k]->vertex_buffer->getIndexBuffer()->getBuffer(), 0u, m_renderables[k]->vertex_buffer->getIndexType());
        
        vkCmdDrawIndexed(command_buffer.getCommandBufer(), static_cast<uint32_t>(m_renderables[k]->vertex_buffer->getIndicesCount()), 1u, 0u, 0u, 0u);
        
        vkCmdEndRenderPass(command_buffer.getCommandBufer());
        current_pipeline = std::underlying_type<RenderTarget::AttachmentLoadOp>::type(RenderTarget::AttachmentLoadOp::LOAD);
    }
}

void SceneDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    size_t sz = m_render_nodes.size();
    if(!sz) return;

    Application& app = Application::Get();
    std::shared_ptr<BaseEngineLogic> game_logic = app.GetGameLogic();
    std::shared_ptr<CameraComponent> camera_component = game_logic->GetHumanView()->VGetCamera();
    std::shared_ptr<BasicCameraNode> camera_node = camera_component->VGetCameraNode();
    for(size_t i = 0u; i < sz; ++i) {
        std::shared_ptr<MeshNode> mesh_node = m_render_nodes.at(i);
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

void SceneDrawable::addRendeNode(std::shared_ptr<MeshNode> model) {
    const MeshNode::MeshList& mesh_list = model->GetMeshes();
    
    size_t msz = mesh_list.size();
    for (size_t i = 0u; i < msz; ++i) {
        m_render_nodes.push_back(model);
        std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>();
        std::shared_ptr<ModelData> model_data = mesh_list.at(i);
        std::shared_ptr<Material> material = model_data->GetMaterial();

        renderable->texture = material->GetTexture();

        std::vector<VulkanDescriptor::Binding> binding(m_max_frames);
        renderable->uniform_buffers.resize(m_max_frames);
        for(size_t j = 0u; j < m_max_frames; ++j) {
            std::shared_ptr<VulkanUniformBuffer> ubo = std::make_shared<VulkanUniformBuffer>();
            ubo->init<UniformBufferObject>(m_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            renderable->uniform_buffers[j] = std::move(ubo);

            binding[j].resize(2u);
            binding[j][0u].layout_binding.binding = 0u;
            binding[j][0u].layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            binding[j][0u].layout_binding.descriptorCount = 1u;
            binding[j][0u].layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            binding[j][0u].layout_binding.pImmutableSamplers = nullptr;
            binding[j][0u].buffer_info = std::make_shared<VkDescriptorBufferInfo>(renderable->uniform_buffers[i]->getDescBufferInfo());
        
            binding[j][1u].layout_binding.binding = 1u;
            binding[j][1u].layout_binding.descriptorCount = 1u;
            binding[j][1u].layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding[j][1u].layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding[j][1u].layout_binding.pImmutableSamplers = nullptr;
            binding[j][1u].image_info = std::make_shared<VkDescriptorImageInfo>(renderable->texture->getDescImageInfo());
            binding[j][1u].sampler = renderable->texture->getSampler();
        }
        renderable->descriptor.init(m_device->getDevice(), std::move(binding), m_max_frames);
        renderable->vertex_buffer = model_data->GetVertexBuffer();
        renderable->vert_shader = m_vert_shader;
        renderable->frag_shader = m_frag_shader;

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shaders_info;
        pipeline_shaders_info.reserve(2u);
        pipeline_shaders_info.push_back(renderable->vert_shader->getShaderInfo());
        pipeline_shaders_info.push_back(renderable->frag_shader->getShaderInfo());
        
        renderable->pipelines = std::vector<GraphicsPipeline>(2u);
        renderable->pipelines[0].pipeline_cfg = createPipelineCfg(
            {renderable->descriptor.getDescLayouts()},
            m_load_render_pass,
            m_viewport_extent,
            pipeline_shaders_info,
            renderable->vertex_buffer->getVertextInputInfo(),
            m_device->getMsaaSamples()
        );
        renderable->pipelines[0].pipeline.init(m_device->getDevice(), renderable->pipelines[0].pipeline_cfg);

        renderable->pipelines[1].pipeline_cfg = createPipelineCfg(
            {renderable->descriptor.getDescLayouts()},
            m_clear_render_pass,
            m_viewport_extent,
            pipeline_shaders_info,
            renderable->vertex_buffer->getVertextInputInfo(),
            m_device->getMsaaSamples()
        );
        renderable->pipelines[1].pipeline.init(m_device->getDevice(), renderable->pipelines[1].pipeline_cfg);

        m_renderables.push_back(std::move(renderable));
    }
}

VulkanPipeline::PipelineCfg SceneDrawable::createPipelineCfg(const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples) {
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
    //pipeline_cfg.rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
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