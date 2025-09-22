#include "gltf_drawable.h"

#include <utility>

struct GLTFUniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct GLTFVertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
    
    static const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() {
        static std::vector<VkVertexInputBindingDescription> binding_desc(1);
        static std::once_flag exe_flag;
        std::call_once(exe_flag, [](){
            binding_desc[0].binding = 0u;
            binding_desc[0].stride = sizeof(GLTFVertex);
            binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        });
        
        return binding_desc;
    }
    
    static const std::vector<VkVertexInputAttributeDescription>& getAttributeDescritpions() {
        static std::vector<VkVertexInputAttributeDescription> attribute_desc(3);
        static std::once_flag exe_flag;
        std::call_once(exe_flag, [](){
            attribute_desc[0].binding = 0;
            attribute_desc[0].location = 0;
            attribute_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_desc[0].offset = offsetof(GLTFVertex, pos);
            
            attribute_desc[1].binding = 0;
            attribute_desc[1].location = 1;
            attribute_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_desc[1].offset = offsetof(GLTFVertex, color);
            
            attribute_desc[2].binding = 0;
            attribute_desc[2].location = 2;
            attribute_desc[2].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_desc[2].offset = offsetof(GLTFVertex, tex_coord);;
        });
        
        return attribute_desc;
    }

    static VkPipelineVertexInputStateCreateInfo getVertextInputInfo() {
        const std::vector<VkVertexInputBindingDescription>& binding_desc = getBindingDescriptions();
        const std::vector<VkVertexInputAttributeDescription>& attribute_desc = getAttributeDescritpions();
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_desc.size());
        vertex_input_info.pVertexBindingDescriptions = binding_desc.data();
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();

        return vertex_input_info;
    }
};

const std::vector<GLTFVertex> g_vertices = {
    {{-0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  1.0f}},
    {{-0.5f,  0.5f,  0.5f}, { 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f}}
};

const std::vector<uint16_t> g_indices = {
    0, 1, 2,
    2, 3, 0,
    4, 5, 6,
    6, 7, 4
};

bool GLTFDrawable::init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt) {
    m_device = std::move(device);
    m_render_target_fmt = rt.render_target_fmt;
    m_rt_aspect = (float)rt.render_target_fmt.viewportExtent.width / (float)rt.render_target_fmt.viewportExtent.height;

    std::shared_ptr<VertexBuffer<GLTFVertex, uint16_t>> vertex_buffer = std::make_shared<VertexBuffer<GLTFVertex, uint16_t>>();
    vertex_buffer->init(m_device, g_vertices, g_indices, GLTFVertex::getVertextInputInfo());
    m_vertex_buffer = std::move(vertex_buffer);

    std::shared_ptr<VulkanUniformBuffer<UniformBufferObject>> uniform_buffer = std::make_shared<VulkanUniformBuffer<UniformBufferObject>>();
    uniform_buffer->init(m_device, rt.frame_count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_uniform_buffers = std::move(uniform_buffer);

    m_texture.init(m_device, "textures/texture.jpg");

    std::vector<VulkanDescriptor::Binding> binding(rt.frame_count);
    for(size_t i = 0u; i < rt.frame_count; ++i) {
        binding[i].resize(2u);
        binding[i][0u].layout_binding.binding = 0u;
        binding[i][0u].layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding[i][0u].layout_binding.descriptorCount = 1u;
        binding[i][0u].layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        binding[i][0u].layout_binding.pImmutableSamplers = nullptr;
        binding[i][0u].buffer_info = std::make_shared<VkDescriptorBufferInfo>(m_uniform_buffers->getDescBufferInfo(i));
        
        binding[i][1u].layout_binding.binding = 1u;
        binding[i][1u].layout_binding.descriptorCount = 1u;
        binding[i][1u].layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding[i][1u].layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding[i][1u].layout_binding.pImmutableSamplers = nullptr;
        binding[i][1u].image_info = std::make_shared<VkDescriptorImageInfo>(m_texture.getDescImageInfo());
        binding[i][1u].sampler = m_texture.getSampler();
    }

    m_descriptor.init(m_device->getDevice(), std::move(binding), rt.frame_count);


    m_frag_shader.init(m_device->getDevice(), "shaders/shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_vert_shader.init(m_device->getDevice(), "shaders/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shaders_info;
    pipeline_shaders_info.reserve(2u);
    pipeline_shaders_info.push_back(m_frag_shader.getShaderInfo());
    pipeline_shaders_info.push_back(m_vert_shader.getShaderInfo());

    m_render_pass = createRenderPass(rt.render_target_fmt.colorAttachmentFormat.format, rt.render_target_fmt.depthAttachmentFormat);
    m_pipeline.init(m_device->getDevice(), {m_descriptor.getDescLayouts()}, m_render_pass, rt.render_target_fmt.viewportExtent, std::move(pipeline_shaders_info), m_vertex_buffer->getVertextInputInfo(), m_device->getMsaaSamples());
    m_out_framebuffers = createFramebuffers(rt);

    return true;
}

void GLTFDrawable::reset(const RenderTarget& rt) {
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
    m_render_pass = createRenderPass(rt.render_target_fmt.colorAttachmentFormat.format, rt.render_target_fmt.depthAttachmentFormat);
    m_pipeline.init(m_device->getDevice(), {m_descriptor.getDescLayouts()}, m_render_pass, rt.render_target_fmt.viewportExtent, std::move(pipeline_shaders), m_vertex_buffer->getVertextInputInfo(), m_device->getMsaaSamples());
    m_out_framebuffers = createFramebuffers(rt);
}

void GLTFDrawable::destroy() {
    m_vertex_buffer->destroy();
    m_uniform_buffers->destroy();
    m_pipeline.destroy();
    m_descriptor.destroy();
    m_vert_shader.destroy();
    m_frag_shader.destroy();
    vkDestroyRenderPass(m_device->getDevice(), m_render_pass, nullptr);

    m_texture.destroy();

    for(size_t i = 0u; i < m_out_framebuffers.size(); ++i) {
        vkDestroyFramebuffer(m_device->getDevice(), m_out_framebuffers[i], nullptr);
    }
}

void GLTFDrawable::recordCommandBuffer(const CommandBatch& command_buffer, uint32_t image_index) {

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

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_render_target_fmt.viewportExtent;

    vkCmdBeginRenderPass(command_buffer.getCommandBufer(), &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
        
    vkCmdSetViewport(command_buffer.getCommandBufer(), 0u, 1u, &view_port);
    vkCmdSetScissor(command_buffer.getCommandBufer(), 0u, 1u, &scissor);

    vkCmdBindDescriptorSets(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getPipelineLayout(), 0, 1, &m_descriptor.getDescriptorSets()[image_index], 0, nullptr);
        
    vkCmdBindPipeline(command_buffer.getCommandBufer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getPipeline());

    VkBuffer vertex_buffers[] = {m_vertex_buffer->getVertexBuffer().buf};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer.getCommandBufer(), 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer.getCommandBufer(), m_vertex_buffer->getIndexBuffer().buf, 0u, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(command_buffer.getCommandBufer(), static_cast<uint32_t>(m_vertex_buffer->getIndicesCount()), 1u, 0u, 0u, 0u);

    vkCmdEndRenderPass(command_buffer.getCommandBufer());
}

void GLTFDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    float angle = delta.fGetTotalSeconds() * glm::radians(90.f);
    glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    
    GLTFUniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), -1.0f * angle, rotation_axis);
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), m_rt_aspect, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1.0f;

    m_uniform_buffers->update(VK_NULL_HANDLE, &ubo, image_index);
}

VkRenderPass GLTFDrawable::createRenderPass(VkFormat color_format, VkFormat depth_format) {
    VkRenderPass result;
    VkSubpassDependency pass_dependency{};
    pass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    pass_dependency.dstSubpass = 0;
    pass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    pass_dependency.srcAccessMask = 0u;
    pass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    pass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    result = createRenderPass(color_format, depth_format, pass_dependency);

    return result;
}

VkRenderPass GLTFDrawable::createRenderPass(VkFormat color_format, VkFormat depth_format, VkSubpassDependency subpass_dependency) {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = color_format;
    color_attachment.samples = m_device->getMsaaSamples();
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0u;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 
    
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = depth_format;
    depth_attachment.samples = m_device->getMsaaSamples();
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1u;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format = color_format;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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
    VkResult result = vkCreateRenderPass(m_device->getDevice(), &render_pass_info, nullptr, &render_pass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    
    return render_pass;
}

std::vector<VkFramebuffer> GLTFDrawable::createFramebuffers(const RenderTarget& rt) {
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