#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "gltf_drawable.h"

#include <filesystem>
#include <utility>

struct GLTFUniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

VkIndexType getIndexType(int accessor_component_type) {
    switch (accessor_component_type) {
        case TINYGLTF_COMPONENT_TYPE_BYTE : return VK_INDEX_TYPE_UINT8_EXT;
        case TINYGLTF_COMPONENT_TYPE_SHORT : return VK_INDEX_TYPE_UINT16;
        case TINYGLTF_COMPONENT_TYPE_INT : return VK_INDEX_TYPE_UINT32;

        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE : return VK_INDEX_TYPE_UINT8_EXT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : return VK_INDEX_TYPE_UINT16;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT : return VK_INDEX_TYPE_UINT32;
        
        default: return VK_INDEX_TYPE_UINT32;
    }
}

float GetAttributeFloat(const unsigned char* raw_data_ptr, uint32_t component_type) {
	float result = 0.0f;
	switch (component_type) {
		case TINYGLTF_COMPONENT_TYPE_BYTE: {
			result = static_cast<float>(*((int8_t*)raw_data_ptr))/127.0f;
		}
		break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            result = static_cast<float>(*((uint8_t*)raw_data_ptr))/255.0f;
        }
        break;
		case TINYGLTF_COMPONENT_TYPE_SHORT: {
            result = static_cast<float>(*((int16_t*)raw_data_ptr))/32767.0f;
        }
        break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            result = static_cast<float>(*((uint16_t*)raw_data_ptr))/65535.0f;
        } 
        break;
		case TINYGLTF_COMPONENT_TYPE_INT: {
            result = static_cast<float>(*((int32_t*)raw_data_ptr))/2147483647.0f;
        }
        break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            result = static_cast<float>(*((uint32_t*)raw_data_ptr))/4294967295.0f;
        } 
        break;
		case TINYGLTF_COMPONENT_TYPE_FLOAT: {
			result = *((float*)raw_data_ptr);
		}
		break;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE: {
			double value = *((double*)raw_data_ptr);
			result = value;
		}
		break;
		default: break;
	}
	return result;
}

int32_t getVertexStride(size_t mesh_idx, size_t primitive_idx, const tinygltf::Model& gltf_model) {
    int32_t vertex_stride = 0u;
    const tinygltf::Mesh& mesh = gltf_model.meshes[mesh_idx];
    const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];
    for (const auto& [semantic_name, semantic_accessor_idx] : primitive.attributes) {
        const tinygltf::Accessor& vertex_attrib_accessor = gltf_model.accessors[semantic_accessor_idx];
        int32_t element_size = tinygltf::GetComponentSizeInBytes(vertex_attrib_accessor.componentType);
        int32_t num_elements_in_type = tinygltf::GetNumComponentsInType(vertex_attrib_accessor.type);
        vertex_stride += element_size * num_elements_in_type;
    }

    return vertex_stride;
}

int32_t getNumVertices(size_t mesh_idx, size_t primitive_idx, const tinygltf::Model& gltf_model) {
    const tinygltf::Mesh& mesh = gltf_model.meshes[mesh_idx];
    const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];
	auto pos_semantic_accessor_idx = primitive.attributes.at("POSITION");
	const tinygltf::Accessor& pos_vertex_attrib_accessor = gltf_model.accessors[pos_semantic_accessor_idx];
	int32_t num_vertices = pos_vertex_attrib_accessor.count;
	return num_vertices;
}

bool ValidateVertexAttribute(const std::string& semantic_name) {
	if (semantic_name == "POSITION") {
		return true;
	}
	else if (semantic_name == "NORMAL") {
		return true;
	}
	else if (semantic_name == "TANGENT") {
		return true;
	}
	else if (semantic_name == "BINORMAL") {
		return true;
	}
    else if (semantic_name == "TEXCOORD") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_0") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_1") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_2") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_3") {
		return true;
	}
	else if (semantic_name == "COLOR") {
		return true;
	}
    else if (semantic_name == "COLOR_0") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_0") {
		return true;
	}
	else if (semantic_name == "JOINTS_0") {
		return true;
	}
	return false;
}

size_t getShaderFloatOffset(const std::string& semantic_name) {
    if (semantic_name == "POSITION") {
		return 0u;
	}
	else if (semantic_name == "TEXCOORD_0") {
		return 6u;
	}
    else if (semantic_name == "COLOR_0") {
		return 3u;
	}
	return -1;
}

size_t getShaderStride() {
    return 12 + 12 + 8;
}

uint32_t getShaderNumElementsToCopy(const std::string& semantic_name) {
    if (semantic_name == "POSITION") {
		return 3u;
	}
	else if (semantic_name == "TEXCOORD_0") {
		return 2u;
	}
    else if (semantic_name == "COLOR_0") {
		return 3u;
	}
	return -1;
}

std::vector<float> getVertices(const tinygltf::Model& gltf_model) {
    int32_t vertex_stride = getVertexStride(0u, 0u, gltf_model);
    int32_t num_vertices = getNumVertices(0u, 0u, gltf_model);
    int32_t shader_stride = getShaderStride();
    std::vector<float> result(num_vertices * (shader_stride / 4), 0.0f);

    const tinygltf::Mesh& mesh = gltf_model.meshes[0];
    const tinygltf::Primitive& primitive = mesh.primitives[0];
    for (const auto& [semantic_name, semantic_accessor_idx] : primitive.attributes) {
        if(!ValidateVertexAttribute(semantic_name)) continue;
        size_t shader_current_offset = getShaderFloatOffset(semantic_name);

        const tinygltf::Accessor& vertex_attrib_accessor = gltf_model.accessors[semantic_accessor_idx];
        int32_t element_size = tinygltf::GetComponentSizeInBytes(vertex_attrib_accessor.componentType);
        int32_t num_elements_in_type = tinygltf::GetNumComponentsInType(vertex_attrib_accessor.type);
        int32_t num_elements_to_copy = getShaderNumElementsToCopy(semantic_name);
        const tinygltf::BufferView& vertex_attrib_view = gltf_model.bufferViews[vertex_attrib_accessor.bufferView];
        const tinygltf::Buffer& vertex_attrib_buffer = gltf_model.buffers[vertex_attrib_view.buffer];
        const unsigned char* begin_ptr = vertex_attrib_buffer.data.data();
        size_t buffer_offset_bytes = vertex_attrib_accessor.byteOffset + vertex_attrib_view.byteOffset;
        size_t buffer_stride_bytes = vertex_attrib_view.byteStride ? vertex_attrib_view.byteStride : element_size * num_elements_in_type;
        size_t elements_count = vertex_attrib_accessor.count;
        for (size_t current_element = 0u; current_element < elements_count; ++current_element) {
        	for (int32_t current_component_num = 0; current_component_num < num_elements_to_copy; ++current_component_num) {
        		const unsigned char* raw_data_ptr = begin_ptr + buffer_offset_bytes + current_element * buffer_stride_bytes + current_component_num * element_size;
        		float vertex_attrib_element = GetAttributeFloat(raw_data_ptr, vertex_attrib_accessor.componentType);
        		result[current_element * (shader_stride / 4) + shader_current_offset + current_component_num] = vertex_attrib_element;
        	}
        }
    }

    return result;
}

VkPipelineVertexInputStateCreateInfo getVertextInputInfo() {
    static std::vector<VkVertexInputBindingDescription> binding_desc(1);
    static std::once_flag binding_exe_flag;
    std::call_once(binding_exe_flag, [](){
        binding_desc[0].binding = 0u;
        binding_desc[0].stride = 32u;
        binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    });

    static std::vector<VkVertexInputAttributeDescription> attribute_desc(3);
    static std::once_flag attribute_exe_flag;
    std::call_once(attribute_exe_flag, [](){
        attribute_desc[0].binding = 0u;
        attribute_desc[0].location = 0u;
        attribute_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc[0].offset = 0u;
        
        attribute_desc[1].binding = 0u;
        attribute_desc[1].location = 1u;
        attribute_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc[1].offset = 12u;
        
        attribute_desc[2].binding = 0u;
        attribute_desc[2].location = 2u;
        attribute_desc[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[2].offset = 24u;
    });

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_desc.size());
    vertex_input_info.pVertexBindingDescriptions = binding_desc.data();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();

    return vertex_input_info;
}

VulkanPipeline::PipelineCfg GLTFDrawable::createPipelineCfg(const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples) {
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

bool GLTFDrawable::init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt, int max_frames, std::filesystem::path model_path) {
    using namespace std::literals;
    
    m_device = std::move(device);
    m_max_frames = max_frames;
    m_rt_aspect = rt.getAspect();

    bool store_original_json_for_extras_and_extensions = true;
    m_gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(store_original_json_for_extras_and_extensions);

    bool load_result = false;
    std::string ext = model_path.extension().string().c_str();
    std::string load_error;
    std::string load_warning;
    if (ext.compare(".glb") == 0) {
    	load_result = m_gltf_ctx.LoadBinaryFromFile(&m_gltf_model, &load_error, &load_warning, model_path.string().c_str());
    }
    else {
    	load_result = m_gltf_ctx.LoadASCIIFromFile(&m_gltf_model, &load_error, &load_warning, model_path.string().c_str());
    }

    const tinygltf::Mesh& mesh = m_gltf_model.meshes[0];
    const tinygltf::Primitive& primitive = mesh.primitives[0];
    const tinygltf::Accessor& indices_accessor = m_gltf_model.accessors[primitive.indices];
    const tinygltf::BufferView& indices_buffer_view = m_gltf_model.bufferViews[indices_accessor.bufferView];
    const tinygltf::Buffer& indices_buffer = m_gltf_model.buffers[indices_buffer_view.buffer];

    size_t index_count = indices_accessor.count;
    const void* indices_data = indices_buffer.data.data() + indices_buffer_view.byteOffset;

    int pos_semantic_accessor_idx = primitive.attributes.at("POSITION");
    const tinygltf::Accessor& vertices_pos_accessor = m_gltf_model.accessors[pos_semantic_accessor_idx];

    size_t vertex_count = vertices_pos_accessor.count;
    std::vector<float> vertices = getVertices(m_gltf_model);
    const void* vertices_data = vertices.data();

    const tinygltf::Material& gltf_material = m_gltf_model.materials[primitive.material];
    const tinygltf::Texture& gltf_texture = m_gltf_model.textures[gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index];
    const tinygltf::Image& gltf_image = m_gltf_model.images[gltf_texture.source];
    const std::string texture_name = "textures/"s + gltf_image.uri;
    m_texture.init(m_device, texture_name);

    std::vector<VulkanDescriptor::Binding> binding(max_frames);
    m_vertex_buffers.resize(max_frames);
    m_uniform_buffers.resize(max_frames);
    for(size_t i = 0u; i < max_frames; ++i) {
        std::shared_ptr<VertexBuffer> vertex_buffer = std::make_shared<VertexBuffer>();
        vertex_buffer->init(m_device, vertices_data, vertex_count, indices_data, index_count, getIndexType(indices_accessor.componentType), getVertextInputInfo());
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

void GLTFDrawable::reset(const RenderTarget& rt) {
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

void GLTFDrawable::destroy() {
    m_pipelines[0].pipeline.destroy();
    m_pipelines[0].pipeline.destroy();
    m_descriptor.destroy();
    m_vert_shader.destroy();
    m_frag_shader.destroy();

    m_texture.destroy();

    for(size_t i = 0u; i < m_max_frames; ++i) {
        m_vertex_buffers[i]->destroy();
        m_uniform_buffers[i]->destroy();
    }
}

void GLTFDrawable::recordCommandBuffer(CommandBatch& command_buffer, const RenderTarget& rt, uint32_t image_index) {

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
    vkCmdBindIndexBuffer(command_buffer.getCommandBufer(), m_vertex_buffers[image_index]->getIndexBuffer()->getBuffer(), 0u, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(command_buffer.getCommandBufer(), static_cast<uint32_t>(m_vertex_buffers[image_index]->getIndicesCount()), 1u, 0u, 0u, 0u);

    vkCmdEndRenderPass(command_buffer.getCommandBufer());
}

void GLTFDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    float angle = delta.fGetTotalSeconds() * glm::radians(90.f);
    glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    
    GLTFUniformBufferObject ubo{};
    ubo.model = glm::translate(glm::scale(glm::rotate(glm::mat4(1.0f), -1.0f * angle, rotation_axis), glm::vec3(0.25f, 0.25f, 0.25f)), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), m_rt_aspect, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1.0f;

    m_uniform_buffers[image_index]->update(&ubo, sizeof(GLTFUniformBufferObject));
}