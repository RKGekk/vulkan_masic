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

    const std::string& getName() const;
    VkFlags getPipelineCreateFlags() const;
    const std::vector<std::string>& getShaderNames() const;
    const VkPipelineInputAssemblyStateCreateInfo& getAssemblyInfo() const;
    const VkPipelineTessellationStateCreateInfo& getTessellationInfo() const;
    const VkPipelineRasterizationStateCreateInfo& getRasterizerInfo() const;
    const VkPipelineMultisampleStateCreateInfo& getMultisampleInfo() const;
    const VkPipelineDepthStencilStateCreateInfo& getDepthStencilInfo() const;
    const VkPipelineColorBlendStateCreateInfo& getColorBlendInfo() const;
    const std::vector<VkPipelineColorBlendAttachmentState>& getColorBlendAttachments() const;
    const VkPipelineDynamicStateCreateInfo& getDynamicInfo() const;
    const std::vector<VkDynamicState>& getDynamicStates() const;

private:
    std::string m_name;
    VkFlags m_pipeline_create_flags; // VkPipelineCreateFlagBits
    std::vector<std::string> m_shaders;
    VkPipelineInputAssemblyStateCreateInfo m_assembly_info;
    VkPipelineTessellationStateCreateInfo m_tessellation_info;
    VkPipelineRasterizationStateCreateInfo m_rasterizer_info;
    VkPipelineMultisampleStateCreateInfo m_multisample_info;
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil_info;
    VkPipelineColorBlendStateCreateInfo m_color_blend_info;
    std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachments;
    VkPipelineDynamicStateCreateInfo m_dynamic_info;
    std::vector<VkDynamicState> m_dynamic_states;
};