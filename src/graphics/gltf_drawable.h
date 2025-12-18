#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "tiny_gltf.h"

#include "vulkan_device.h"
#include "vulkan_drawable.h"
#include "render_resource.h"
#include "vulkan_uniform_buffer.h"
#include "vulkan_vertex_buffer.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "vulkan_descriptor.h"
#include "vulkan_texture.h"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

class GLTFDrawable : public IVulkanDrawable {
public:
    struct GraphicsPipeline {
        VulkanPipeline pipeline;
        VulkanPipeline::PipelineCfg pipeline_cfg;
    };

    bool init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt, int max_frames, std::filesystem::path model_path);

    void reset(const RenderTarget& rt) override;
    void destroy() override;
    void recordCommandBuffer(CommandBatch& command_buffer, const RenderTarget& rt, uint32_t frame_index) override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

    virtual int order() override;

private:
    int32_t getNumVertices(size_t mesh_idx, size_t primitive_idx, const tinygltf::Model& gltf_model);
    bool ValidateVertexAttribute(const std::string& semantic_name);
    int32_t getVertexStride(size_t mesh_idx, size_t primitive_idx, const tinygltf::Model& gltf_model);
    float GetAttributeFloat(const unsigned char* raw_data_ptr, uint32_t component_type);
    VkIndexType getIndexType(int accessor_component_type);
    size_t getShaderFloatOffset(const std::string& semantic_name);
    size_t getShaderStride();
    uint32_t getShaderNumElementsToCopy(const std::string& semantic_name);
    std::vector<float> getVertices(const tinygltf::Model& gltf_model);
    VkPipelineVertexInputStateCreateInfo getVertextInputInfo();

    VulkanPipeline::PipelineCfg createPipelineCfg(const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples);

    std::shared_ptr<VulkanDevice> m_device;

    std::vector<std::shared_ptr<VulkanUniformBuffer>> m_uniform_buffers;
    std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
    float m_rt_aspect = 1.0f;

    std::vector<GraphicsPipeline> m_pipelines;
    
    VulkanDescriptor m_descriptor;
    int m_max_frames;

    VulkanShader m_vert_shader;
    VulkanShader m_frag_shader;

    VulkanTexture m_texture;

    tinygltf::Model m_gltf_model;
    tinygltf::TinyGLTF m_gltf_ctx;
};