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

#include "../api/vulkan_device.h"
#include "vulkan_drawable.h"
#include "../pod/render_resource.h"
#include "../api/vulkan_uniform_buffer.h"
#include "../api/vulkan_vertex_buffer.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_shader.h"
#include "../api/vulkan_descriptor.h"
#include "../api/vulkan_texture.h"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

class BasicDrawable : public IVulkanDrawable {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt, int max_frames);
    void reset(const RenderTarget& rt) override;
    void destroy() override;
    void recordCommandBuffer(CommandBatch& command_buffer, const RenderTarget& rt, uint32_t frame_index) override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

    virtual int order() override;

private:
    VulkanPipeline::PipelineCfg createPipelineCfg(const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples);

    std::shared_ptr<VulkanDevice> m_device;

    std::vector<std::shared_ptr<VulkanUniformBuffer>> m_uniform_buffers;
    std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
    float m_rt_aspect = 1.0f;

	struct GraphicsPipeline {
        VulkanPipeline pipeline;
        VulkanPipeline::PipelineCfg pipeline_cfg;
    };
    std::vector<GraphicsPipeline> m_pipelines;

    VulkanDescriptor m_descriptor;
    int m_max_frames;

    VulkanShader m_vert_shader;
    VulkanShader m_frag_shader;

    VulkanTexture m_texture;
};