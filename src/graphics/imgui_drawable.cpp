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
    cfg.RasterizerMultiply = 1.5f;
    cfg.SizePixels = ceilf(fontSizePixels);
    cfg.PixelSnapH = true;
    cfg.OversampleH = 4;
    cfg.OversampleV = 4;
    ImFont* font = nullptr;

    if (TTF_font_file_name) {
        font = io.Fonts->AddFontFromFileTTF(TTF_font_file_name, cfg.SizePixels, &cfg);
    } else {
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

VkRenderPass createRenderPass(std::shared_ptr<VulkanDevice> device, VkFormat color_format, VkFormat depth_format, VkSubpassDependency subpass_dependency) {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = color_format;
    color_attachment.samples = device->getMsaaSamples();
    //color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0u;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 
    
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = depth_format;
    depth_attachment.samples = device->getMsaaSamples();
    //depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1u;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format = color_format;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    //color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 2u;
    color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass_desc{};
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1u;
    subpass_desc.pColorAttachments = &color_attachment_ref;
    subpass_desc.pDepthStencilAttachment = &depth_attachment_ref;
    subpass_desc.pResolveAttachments = &color_attachment_resolve_ref;
    
    std::array<VkAttachmentDescription, 3> attachments = {color_attachment, depth_attachment, color_attachment_resolve};
    std::array<VkSubpassDescription, 1> subpases = {subpass_desc};
    std::array<VkSubpassDependency, 1> subdependencies = {subpass_dependency};
    
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = static_cast<uint32_t>(subpases.size());
    render_pass_info.pSubpasses = subpases.data();
    render_pass_info.dependencyCount = static_cast<uint32_t>(subdependencies.size());
    render_pass_info.pDependencies = subdependencies.data();
    
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkResult result = vkCreateRenderPass(device->getDevice(), &render_pass_info, nullptr, &render_pass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    
    return render_pass;
}

VkRenderPass createRenderPass(std::shared_ptr<VulkanDevice> device, VkFormat color_format, VkFormat depth_format) {
    VkRenderPass result;
    VkSubpassDependency pass_dependency{};
    pass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    pass_dependency.dstSubpass = 0;
    pass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    pass_dependency.srcAccessMask = 0u;
    pass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    pass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    result = createRenderPass(device, color_format, depth_format, pass_dependency);

    return result;
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
        attribute_desc[0].offset = 0u;
        
        attribute_desc[1].binding = 0u;
        attribute_desc[1].location = 1u;
        attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[1].offset = 8u;
        
        attribute_desc[2].binding = 0u;
        attribute_desc[2].location = 2u;
        attribute_desc[2].format = VK_FORMAT_R32_UINT;
        attribute_desc[2].offset = 12u;
    });

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_desc.size());
    vertex_input_info.pVertexBindingDescriptions = binding_desc.data();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();

    return vertex_input_info;
}

bool ImGUIDrawable::init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt) {
    m_device = std::move(device);

    m_imgui_vtx.resize(rt.frame_count);
    m_imgui_idx.resize(rt.frame_count);

    static const std::string TTF_font_file_name = std::filesystem::current_path().append("fonts").append("OpenSans-Light.ttf").string();
    static const float font_size_pixels = 30.0f;

    ImGui::CreateContext();

    m_render_target_fmt = rt.render_target_fmt;
    m_rt_aspect = (float)rt.render_target_fmt.viewportExtent.width / (float)rt.render_target_fmt.viewportExtent.height;

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui-lvk";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    m_font_texture = makeFontTexture(m_device, TTF_font_file_name.c_str(), font_size_pixels);

    m_frag_shader.init(m_device->getDevice(), "shaders/imgui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_vert_shader.init(m_device->getDevice(), "shaders/imgui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);

    std::vector<VulkanDescriptor::Binding> binding(rt.frame_count);
    m_vertex_buffers.resize(rt.frame_count);
    m_uniform_buffers.resize(rt.frame_count);
    for(size_t i = 0u; i < rt.frame_count; ++i) {
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

    m_descriptor.init(m_device->getDevice(), std::move(binding), rt.frame_count);

    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shaders_info;
    pipeline_shaders_info.reserve(2u);
    pipeline_shaders_info.push_back(m_frag_shader.getShaderInfo());
    pipeline_shaders_info.push_back(m_vert_shader.getShaderInfo());

    m_render_pass = createRenderPass(m_device, rt.render_target_fmt.colorAttachmentFormat.format, rt.render_target_fmt.depthAttachmentFormat);
    m_pipeline.init(m_device->getDevice(), {m_descriptor.getDescLayouts()}, m_render_pass, rt.render_target_fmt.viewportExtent, std::move(pipeline_shaders_info), m_vertex_buffers[0]->getVertextInputInfo(), m_device->getMsaaSamples());
    
    m_out_framebuffers = createFramebuffers(rt);

    return true;
}

std::vector<VkFramebuffer> ImGUIDrawable::createFramebuffers(const RenderTarget& rt) {
    std::vector<VkFramebuffer> result_framebuffers(rt.frame_count);
    for(size_t i = 0u; i < rt.frame_count; ++i) {
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_render_pass;
        framebuffer_info.attachmentCount = static_cast<uint32_t>(rt.frames[i].size());
        framebuffer_info.pAttachments = rt.frames[i].data();
        framebuffer_info.width = rt.render_target_fmt.viewportExtent.width;
        framebuffer_info.height = rt.render_target_fmt.viewportExtent.height;
        framebuffer_info.layers = 1u;
        
        VkResult result = vkCreateFramebuffer(m_device->getDevice(), &framebuffer_info, nullptr, &result_framebuffers[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
    
    return result_framebuffers;
}

void ImGUIDrawable::reset(const RenderTarget& rt) {
    std::vector<VulkanDescriptor::Binding> bindings = m_descriptor.getBingingsForSets();
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shaders = m_pipeline.getShadersInfo();

    m_descriptor.destroy();
    vkDestroyRenderPass(m_device->getDevice(), m_render_pass, nullptr);
    m_pipeline.destroy();
    for(size_t i = 0u; i < m_out_framebuffers.size(); ++i) {
        vkDestroyFramebuffer(m_device->getDevice(), m_out_framebuffers[i], nullptr);
    }

    m_render_target_fmt = rt.render_target_fmt;
    m_rt_aspect = (float)rt.render_target_fmt.viewportExtent.width / (float)rt.render_target_fmt.viewportExtent.height;

    m_descriptor.init(m_device->getDevice(), std::move(bindings), rt.frame_count);
    m_render_pass = createRenderPass(m_device, rt.render_target_fmt.colorAttachmentFormat.format, rt.render_target_fmt.depthAttachmentFormat);
    m_pipeline.init(m_device->getDevice(), {m_descriptor.getDescLayouts()}, m_render_pass, rt.render_target_fmt.viewportExtent, std::move(pipeline_shaders), m_vertex_buffers[0]->getVertextInputInfo(), m_device->getMsaaSamples());
    m_out_framebuffers = createFramebuffers(rt);
}

void ImGUIDrawable::destroy() {
    m_pipeline.destroy();
    m_descriptor.destroy();
    m_vert_shader.destroy();
    m_frag_shader.destroy();
    vkDestroyRenderPass(m_device->getDevice(), m_render_pass, nullptr);

    m_font_texture->destroy();

    for(size_t i = 0u; i < m_out_framebuffers.size(); ++i) {
        m_vertex_buffers[i]->destroy();
        m_uniform_buffers[i]->destroy();
        vkDestroyFramebuffer(m_device->getDevice(), m_out_framebuffers[i], nullptr);
    }
}

void ImGUIDrawable::recordCommandBuffer(CommandBatch& command_buffer, uint32_t image_index) {

    VkRenderPassBeginInfo renderpass_info{};
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_info.renderPass = m_render_pass;
    renderpass_info.framebuffer = m_out_framebuffers[image_index];
    renderpass_info.renderArea.offset = {0, 0};
    renderpass_info.renderArea.extent = m_render_target_fmt.viewportExtent;
    
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};
    renderpass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    renderpass_info.pClearValues = clear_values.data();

    VkViewport view_port{};
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = static_cast<float>(m_render_target_fmt.viewportExtent.width);
    view_port.height = static_cast<float>(m_render_target_fmt.viewportExtent.height);
    view_port.minDepth = 0.0f;
    view_port.maxDepth = 1.0f;

    vkCmdBeginRenderPass(command_buffer.getCommandBufer(), &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
        
    vkCmdSetViewport(command_buffer.getCommandBufer(), 0u, 1u, &view_port);

    vkCmdBindDescriptorSets(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getPipelineLayout(), 0, 1, &m_descriptor.getDescriptorSets()[image_index], 0, nullptr);
        
    vkCmdBindPipeline(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getPipeline());

    ImDrawData* dd = ImGui::GetDrawData();

    ImGuiUniformBufferObject ubo{};
    const float L = dd->DisplayPos.x;
    const float R = dd->DisplayPos.x + dd->DisplaySize.x;
    const float T = dd->DisplayPos.y;
    const float B = dd->DisplayPos.y + dd->DisplaySize.y;
    const float fb_width = dd->DisplaySize.x * dd->FramebufferScale.x;
    const float fb_height = dd->DisplaySize.y * dd->FramebufferScale.y;
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
            m_uniform_buffers[image_index]->update(command_buffer, &ubo, sizeof(ImGuiUniformBufferObject), VK_ACCESS_UNIFORM_READ_BIT);
      
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

void ImGUIDrawable::beginFrame(const RenderTarget& rt) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(rt.render_target_fmt.viewportExtent.width, rt.render_target_fmt.viewportExtent.height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.IniFilename = nullptr;

    ImGui::NewFrame();
}

void ImGUIDrawable::endFrame() {
    ImGui::EndFrame();
    ImGui::Render();
}