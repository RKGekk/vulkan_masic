#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <string>
#include <vector>

class PipelineConfig {
public:
    bool init(const std::string& rg_file_path);
    bool init(const pugi::xml_node& pipeline_data);
    void destroy();

private:
    std::string m_name;
    std::vector<std::string> m_shaders;
    VkPrimitiveTopology m_topology;
    VkPipelineTessellationStateCreateInfo m_tessellation_state_Info;
    bool m_primitive_restart_enable;
    VkPipelineRasterizationStateCreateInfo m_rasterizer_info;
    VkPipelineMultisampleStateCreateInfo m_multisample_info;
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil_info;
    VkPipelineColorBlendStateCreateInfo m_color_blend_info;
    std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachments;

    std::vector<VkDescriptorSetLayout> desc_set_layouts;
    VkRenderPass render_pass;
    VkExtent2D viewport_extent;
    std::vector<VkPipelineShaderStageCreateInfo> shaders_info;
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    std::vector<VkDynamicState> dynamic_states;
    
};