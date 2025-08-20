#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <vector>

class VulkanPipeline {
public:
    bool init(VkDevice device, const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples);
    void destroy();

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
    const std::vector<VkPipelineShaderStageCreateInfo>& getShadersInfo() const;

private:
    VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout>& desc_set_layouts) const;
    VkPipeline createPipeline(VkRenderPass render_pass, VkExtent2D viewport_extent, const std::vector<VkPipelineShaderStageCreateInfo>& shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples) const;

    VkDevice m_device;

    VkPipelineCache m_pipeline_cache = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders_info;
};