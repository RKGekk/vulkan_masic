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

#include <imgui.h>

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
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

class ImGUIDrawable : public IVulkanDrawable {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt, int max_frames);

    void reset(const RenderTarget& rt) override;
    void destroy() override;
    void recordCommandBuffer(CommandBatch& command_buffer, const RenderTarget& rt, uint32_t frame_index) override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

    virtual int order() override;

    void beginFrame(const RenderTarget& rt);
    void endFrame();

private:
    VulkanPipeline::PipelineCfg createPipelineCfg(const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples);
    std::shared_ptr<VulkanDevice> m_device;

    struct GraphicsPipeline {
        VulkanPipeline pipeline;
        VulkanPipeline::PipelineCfg pipeline_cfg;
    };
    std::vector<GraphicsPipeline> m_pipelines;

    VulkanDescriptor m_descriptor;
    int m_max_frames;
    

    VulkanShader m_vert_shader;
    VulkanShader m_frag_shader;

    std::shared_ptr<VulkanTexture> m_font_texture;
    std::vector<std::shared_ptr<VulkanUniformBuffer>> m_uniform_buffers;

    std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
    std::vector<std::vector<ImDrawVert>> m_imgui_vtx;
    std::vector<std::vector<ImDrawIdx>> m_imgui_idx;

    ImGuiContext* m_pImgui_ctx;
};