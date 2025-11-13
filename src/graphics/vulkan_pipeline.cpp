#include "vulkan_pipeline.h"

#include <array>
#include <stdexcept>

//bool VulkanPipeline::init(VkDevice device, const std::vector<VkDescriptorSetLayout>& desc_set_layouts, VkRenderPass render_pass, VkExtent2D viewport_extent, std::vector<VkPipelineShaderStageCreateInfo> shaders_info, const VkPipelineVertexInputStateCreateInfo& vertex_input_info, VkSampleCountFlagBits msaa_samples) {
bool VulkanPipeline::init(VkDevice device, const PipelineCfg& pipeline_cfg) {
    m_device = device;

    VkPipelineCacheCreateInfo pipeline_cache_info{};
    pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipeline_cache_info.pNext = nullptr;
    pipeline_cache_info.initialDataSize = 0u;
    pipeline_cache_info.pInitialData = nullptr;
    pipeline_cache_info.flags = 0u;
    VkResult  result = vkCreatePipelineCache(device, &pipeline_cache_info, NULL, &m_pipeline_cache);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline cache!");
    }

    m_pipeline_layout = createPipelineLayout(pipeline_cfg.desc_set_layouts);
    m_graphics_pipeline = createPipeline(pipeline_cfg);
    m_shaders_info = pipeline_cfg.shaders_info;

    return true;
}

void VulkanPipeline::destroy() {
    vkDestroyPipelineCache(m_device, m_pipeline_cache, NULL);
    vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
}

VkPipeline VulkanPipeline::getPipeline() const {
    return m_graphics_pipeline;
}

VkPipelineLayout VulkanPipeline::getPipelineLayout() const {
    return m_pipeline_layout;
}

const std::vector<VkPipelineShaderStageCreateInfo>& VulkanPipeline::getShadersInfo() const {
    return m_shaders_info;
}

VkPipelineLayout VulkanPipeline::createPipelineLayout(const std::vector<VkDescriptorSetLayout>& desc_set_layouts) const {
    VkPipelineLayout pipeline_layout;

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(desc_set_layouts.size());
    pipeline_layout_info.pSetLayouts = desc_set_layouts.data();
    pipeline_layout_info.pushConstantRangeCount = 0u;
    pipeline_layout_info.pPushConstantRanges = nullptr;
    
    VkResult result = vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &pipeline_layout);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    return pipeline_layout;
}

VkPipeline VulkanPipeline::createPipeline(const PipelineCfg& pipeline_cfg) const {
    
    VkPipelineDynamicStateCreateInfo dynamic_state_info{};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(pipeline_cfg.dynamic_states.size());
    dynamic_state_info.pDynamicStates = pipeline_cfg.dynamic_states.data();
    
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = pipeline_cfg.viewport_extent;
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)pipeline_cfg.viewport_extent.width;
    viewport.height = (float)pipeline_cfg.viewport_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkPipelineViewportStateCreateInfo viewport_state_info{};
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.viewportCount = 1u;
    viewport_state_info.pViewports = &viewport;
    viewport_state_info.scissorCount = 1u;
    viewport_state_info.pScissors = &scissor;
    
    VkPipelineMultisampleStateCreateInfo multisample_info{};
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.sampleShadingEnable = VK_FALSE;
    multisample_info.rasterizationSamples = pipeline_cfg.msaa_samples;
    //multisample_info.minSampleShading = 1.0f;
    //multisample_info.pSampleMask = nullptr;
    //multisample_info.alphaToCoverageEnable = VK_FALSE;
    //multisample_info.alphaToOneEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo color_blend_info{};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount = 1u;
    color_blend_info.pAttachments = &pipeline_cfg.color_blend_state;
    color_blend_info.blendConstants[0] = 0.0f;
    color_blend_info.blendConstants[1] = 0.0f;
    color_blend_info.blendConstants[2] = 0.0f;
    color_blend_info.blendConstants[3] = 0.0f;
    
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = static_cast<uint32_t>(pipeline_cfg.shaders_info.size());
    pipeline_info.pStages = pipeline_cfg.shaders_info.data();
    pipeline_info.pVertexInputState = &pipeline_cfg.vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &pipeline_cfg.rasterizer_info;
    pipeline_info.pMultisampleState = &multisample_info;
    pipeline_info.pDepthStencilState = &pipeline_cfg.depth_stencil_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDynamicState = &dynamic_state_info;
    pipeline_info.layout = m_pipeline_layout;
    pipeline_info.renderPass = pipeline_cfg.render_pass;
    pipeline_info.subpass = 0u;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    
    VkPipeline graphics_pipeline;
    VkResult result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return graphics_pipeline;
}