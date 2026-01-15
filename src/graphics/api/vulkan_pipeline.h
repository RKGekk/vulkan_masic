#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <vector>

class VulkanPipeline {
public:
    enum class PipelineType {
        GRAPHICS,
        COMPUTE
    };

    struct PipelineCfg {
        std::string name;
        std::vector<VkDescriptorSetLayout> desc_set_layouts;
        VkRenderPass render_pass;
        VkExtent2D viewport_extent;
        std::vector<VkPipelineShaderStageCreateInfo> shaders_info;
        VkPipelineVertexInputStateCreateInfo vertex_input_info;
        VkSampleCountFlagBits msaa_samples;
        std::vector<VkDynamicState> dynamic_states;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        VkPipelineRasterizationStateCreateInfo rasterizer_info;
        VkPipelineColorBlendAttachmentState color_blend_state;
    };

    bool init(VkDevice device, const PipelineCfg& pipeline_cfg);
    bool init(VkDevice device, const PipelineCfg& pipeline_cfg, VkPipeline pipeline);
    void destroy();

    PipelineType getPipelineType() const;
    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
    const std::vector<VkPipelineShaderStageCreateInfo>& getShadersInfo() const;

private:
    VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout>& desc_set_layouts) const;
    VkPipeline createPipeline(const PipelineCfg& pipeline_cfg) const;

    VkDevice m_device;

    PipelineType m_pipeline_type;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders_info;
};