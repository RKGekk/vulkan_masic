#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class VulkanPipeline {
public:
    bool init(VkDevice device, VkDescriptorSetLayout desc_set_layout, VkRenderPass render_pass, VkExtent2D viewport_extent, const std::vector<VkPipelineShaderStageCreateInfo>& shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples);
    void destroy();

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

private:
    VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout desc_set_layout) const;
    VkPipeline createPipeline(VkRenderPass render_pass, VkExtent2D viewport_extent, const std::vector<VkPipelineShaderStageCreateInfo>& shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples) const;

    VkDevice m_device;

    VkPipelineCache m_pipeline_cache = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;
};