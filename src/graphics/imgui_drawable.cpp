#include "imgui_drawable.h"

#include <filesystem>

VkSampler createFontTextureSampler(std::shared_ptr<VulkanDevice> device) {
    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(device->getDeviceAbilities().physical_device, &supported_features);

    VkPhysicalDeviceProperties device_props{};
    vkGetPhysicalDeviceProperties(device->getDeviceAbilities().physical_device, &device_props);
    
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 15;
    
    VkSampler texture_sampler;
    VkResult result = vkCreateSampler(device->getDevice(), &sampler_info, nullptr, &texture_sampler);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    return texture_sampler;
}

std::shared_ptr<VulkanTexture> makeFontTexture(std::shared_ptr<VulkanDevice> device, const char* TTF_font_file_name, float fontSizePixels) {
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig cfg = ImFontConfig();
    cfg.FontDataOwnedByAtlas = false;
    cfg.RasterizerMultiply = 1.0f;
    cfg.SizePixels = ceilf(fontSizePixels);
    cfg.PixelSnapH = true;
    cfg.OversampleH = 1;
    cfg.OversampleV = 1;
    ImFont* font = nullptr;

    if (TTF_font_file_name) {
        font = io.Fonts->AddFontFromFileTTF(TTF_font_file_name, cfg.SizePixels, &cfg);
    }
    else {
        font = io.Fonts->AddFontDefault(&cfg);
    }

    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;

    // init fonts
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    std::shared_ptr<VulkanTexture> font_texture = std::make_shared<VulkanTexture>();
    VkSampler fonts_sampler = createFontTextureSampler(device);
    font_texture->init(device, pixels, width, height, fonts_sampler, VK_FORMAT_R8G8B8A8_UNORM);

    io.Fonts->TexID = 0u;
    io.FontDefault = font;

    return font_texture;
}

VkPipelineVertexInputStateCreateInfo getImVertextInputInfo() {
    static std::vector<VkVertexInputBindingDescription> binding_desc(1);
    static std::once_flag binding_exe_flag;
    std::call_once(binding_exe_flag, [](){
        binding_desc[0].binding = 0u;
        binding_desc[0].stride = 20u;
        binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    });

    static std::vector<VkVertexInputAttributeDescription> attribute_desc(3);
    static std::once_flag attribute_exe_flag;
    std::call_once(attribute_exe_flag, [](){
        attribute_desc[0].binding = 0u;
        attribute_desc[0].location = 0u;
        attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[0].offset = offsetof(ImDrawVert, pos);
        
        attribute_desc[1].binding = 0u;
        attribute_desc[1].location = 1u;
        attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[1].offset = offsetof(ImDrawVert, uv);
        
        attribute_desc[2].binding = 0u;
        attribute_desc[2].location = 2u;
        attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attribute_desc[2].offset = offsetof(ImDrawVert, col);
    });

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_desc.size());
    vertex_input_info.pVertexBindingDescriptions = binding_desc.data();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();

    return vertex_input_info;
}

VulkanPipeline::PipelineCfg ImGUIDrawable::createPipelineCfg(const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples) {
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
    pipeline_cfg.depth_stencil_info.depthTestEnable = VK_FALSE;
    pipeline_cfg.depth_stencil_info.depthWriteEnable = VK_FALSE;
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
    pipeline_cfg.rasterizer_info.cullMode = VK_CULL_MODE_NONE;
    //m_pipeline_cfg.rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipeline_cfg.rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipeline_cfg.rasterizer_info.depthBiasEnable = VK_FALSE;
    pipeline_cfg.rasterizer_info.depthBiasConstantFactor = 0.0f;
    pipeline_cfg.rasterizer_info.depthBiasClamp = 0.0f;
    pipeline_cfg.rasterizer_info.depthBiasSlopeFactor = 0.0f;
    pipeline_cfg.rasterizer_info.lineWidth = 1.0f;

    pipeline_cfg.color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipeline_cfg.color_blend_state.blendEnable = VK_TRUE;
    pipeline_cfg.color_blend_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipeline_cfg.color_blend_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipeline_cfg.color_blend_state.colorBlendOp = VK_BLEND_OP_ADD;
    pipeline_cfg.color_blend_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipeline_cfg.color_blend_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipeline_cfg.color_blend_state.alphaBlendOp = VK_BLEND_OP_ADD;

    return pipeline_cfg;
}

bool ImGUIDrawable::init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt, int max_frames) {
    using namespace std::literals;

    m_device = std::move(device);
    m_max_frames = max_frames;

    m_imgui_vtx.resize(max_frames);
    m_imgui_idx.resize(max_frames);

    static const std::string TTF_font_file_name = std::filesystem::current_path().append("fonts").append("OpenSans-Light.ttf").string();
    static const float font_size_pixels = 15.0f;

    m_pImgui_ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_pImgui_ctx);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui-lvk";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.FontAllowUserScaling = true;
    io.FontGlobalScale = 1.0f;
    ImGui::StyleColorsClassic();

    m_font_texture = makeFontTexture(m_device, TTF_font_file_name.c_str(), font_size_pixels);

    m_frag_shader.init(m_device->getDevice(), "shaders/imgui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_vert_shader.init(m_device->getDevice(), "shaders/imgui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);

    std::vector<VulkanDescriptor::Binding> binding(max_frames);
    m_vertex_buffers.resize(max_frames);
    m_uniform_buffers.resize(max_frames);
    for(size_t i = 0u; i < max_frames; ++i) {
        m_vertex_buffers[i] = std::make_shared<VertexBuffer>();
        m_vertex_buffers[i]->init(m_device, nullptr, 0u, nullptr, 0u, VK_INDEX_TYPE_UINT16, getImVertextInputInfo(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        m_uniform_buffers[i] = std::make_shared<VulkanUniformBuffer>();
        m_uniform_buffers[i]->init<ImGuiUniformBufferObject>(m_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
        binding[i][1u].image_info = std::make_shared<VkDescriptorImageInfo>(m_font_texture->getDescImageInfo());
        binding[i][1u].sampler = m_font_texture->getSampler();
    }

    m_descriptor.init(m_device->getDevice(), std::move(binding), max_frames);

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

void ImGUIDrawable::reset(const RenderTarget& rt) {
    std::vector<VulkanDescriptor::Binding> bindings = m_descriptor.getBingingsForSets();
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shaders_info = m_pipelines[0].pipeline.getShadersInfo();

    m_descriptor.destroy();
    m_pipelines[0].pipeline.destroy();
    m_pipelines[1].pipeline.destroy();

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

void ImGUIDrawable::destroy() {
    ImGui::DestroyContext(m_pImgui_ctx);
    m_pImgui_ctx = nullptr;
    m_pipelines[0].pipeline.destroy();
    m_pipelines[1].pipeline.destroy();
    m_descriptor.destroy();
    m_vert_shader.destroy();
    m_frag_shader.destroy();

    m_font_texture->destroy();

    for(size_t i = 0u; i < m_max_frames; ++i) {
        m_vertex_buffers[i]->destroy();
        m_uniform_buffers[i]->destroy();
    }
}

void ImGUIDrawable::recordCommandBuffer(CommandBatch& command_buffer, const RenderTarget& rt, uint32_t image_index) {

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

    vkCmdBeginRenderPass(command_buffer.getCommandBufer(), &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
        
    vkCmdBindDescriptorSets(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[current_pipeline].pipeline.getPipelineLayout(), 0, 1, &m_descriptor.getDescriptorSets()[image_index], 0, nullptr);
        
    vkCmdBindPipeline(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[current_pipeline].pipeline.getPipeline());

    ImDrawData* dd = ImGui::GetDrawData();

    const float fb_width = dd->DisplaySize.x * dd->FramebufferScale.x;
    const float fb_height = dd->DisplaySize.y * dd->FramebufferScale.y;
    VkViewport view_port{};
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = fb_width;
    view_port.height = fb_height;
    view_port.minDepth = 0.0f;
    view_port.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer.getCommandBufer(), 0u, 1u, &view_port);

    const float L = dd->DisplayPos.x;
    const float R = dd->DisplayPos.x + dd->DisplaySize.x;
    const float T = dd->DisplayPos.y;
    const float B = dd->DisplayPos.y + dd->DisplaySize.y;
    const ImVec2 clip_off = dd->DisplayPos;
    const ImVec2 clip_scale = dd->FramebufferScale;

    if(m_vertex_buffers[image_index]->getVertexCount() < dd->TotalVtxCount || m_vertex_buffers[image_index]->getIndicesCount() < dd->TotalIdxCount) {
        m_imgui_vtx[image_index].resize(dd->TotalVtxCount);
        m_imgui_idx[image_index].resize(dd->TotalIdxCount);
        m_vertex_buffers[image_index]->destroy();
        m_vertex_buffers[image_index]->init(m_device, nullptr, dd->TotalVtxCount, nullptr, dd->TotalIdxCount, VK_INDEX_TYPE_UINT16, getImVertextInputInfo());
    }

    uint32_t idxOffset = 0;
    uint32_t vtxOffset = 0;
    ImDrawVert* vtx = (ImDrawVert*)m_imgui_vtx[image_index].data();
    uint16_t* idx = (uint16_t*)m_imgui_idx[image_index].data();
    for (const ImDrawList* cmdList : dd->CmdLists) {
        memcpy(vtx, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx += cmdList->VtxBuffer.Size;
        idx += cmdList->IdxBuffer.Size;
    }
    m_vertex_buffers[image_index]->update(m_imgui_vtx[image_index], m_imgui_idx[image_index]);

    VkBuffer vertex_buffers[] = {m_vertex_buffers[image_index]->getVertexBuffer()->getBuffer()};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(command_buffer.getCommandBufer(), 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer.getCommandBufer(), m_vertex_buffers[image_index]->getIndexBuffer()->getBuffer(), 0u, VK_INDEX_TYPE_UINT16);

    for (const ImDrawList* cmdList : dd->CmdLists) {
        for (int cmd_i = 0; cmd_i < cmdList->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd cmd = cmdList->CmdBuffer[cmd_i];

            ImVec2 clipMin((cmd.ClipRect.x - clip_off.x) * clip_scale.x, (cmd.ClipRect.y - clip_off.y) * clip_scale.y);
            ImVec2 clipMax((cmd.ClipRect.z - clip_off.x) * clip_scale.x, (cmd.ClipRect.w - clip_off.y) * clip_scale.y);
            
            if (clipMin.x < 0.0f) clipMin.x = 0.0f;
            if (clipMin.y < 0.0f) clipMin.y = 0.0f;
            if (clipMax.x > fb_width ) clipMax.x = fb_width;
            if (clipMax.y > fb_height) clipMax.y = fb_height;
            if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                continue;
            
            ImGuiUniformBufferObject bindData;
            bindData.LRTB = {L, R, T, B};
            m_uniform_buffers[image_index]->update(command_buffer, &bindData, sizeof(ImGuiUniformBufferObject), VK_ACCESS_UNIFORM_READ_BIT);
      
            VkRect2D scissor{};
            scissor.offset = VkOffset2D{(int32_t)clipMin.x, (int32_t)clipMin.y};
            scissor.extent = VkExtent2D{uint32_t(clipMax.x - clipMin.x), uint32_t(clipMax.y - clipMin.y)};
            vkCmdSetScissor(command_buffer.getCommandBufer(), 0u, 1u, &scissor);
            vkCmdDrawIndexed(command_buffer.getCommandBufer(), cmd.ElemCount, 1u, idxOffset + cmd.IdxOffset, int32_t(vtxOffset + cmd.VtxOffset), 0u);
        }
        idxOffset += cmdList->IdxBuffer.Size;
        vtxOffset += cmdList->VtxBuffer.Size;
    }

    vkCmdEndRenderPass(command_buffer.getCommandBufer());
}

void ImGUIDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    float angle = delta.fGetTotalSeconds() * glm::radians(90.f);
    glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
}

int ImGUIDrawable::order() {
    return 999;
}

void ImGUIDrawable::beginFrame(const RenderTarget& rt) {
    ImGui::SetCurrentContext(m_pImgui_ctx);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(rt.getViewportExtent().width, rt.getViewportExtent().height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.IniFilename = nullptr;

    ImGui::NewFrame();
}

void ImGUIDrawable::endFrame() {
    ImGui::SetCurrentContext(m_pImgui_ctx);
    ImGui::EndFrame();
    ImGui::Render();
}