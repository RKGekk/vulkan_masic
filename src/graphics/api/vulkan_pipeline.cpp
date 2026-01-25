#include "vulkan_pipeline.h"

#include "vulkan_device.h"
#include "../pod/pipeline_config.h"
#include "vulkan_shaders_manager.h"
#include "vulkan_pipelines_manager.h"
#include "vulkan_descriptors_manager.h"
#include "../../tools/string_tools.h"

#include <array>
#include <unordered_set>
#include <stdexcept>

bool VulkanPipeline::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& pipeline_data, VkExtent2D viewport_extent, VkRenderPass render_pass, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shader_manager) {
    m_device = device;

    VkPipelineCacheCreateInfo pipeline_cache_info{};
    pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipeline_cache_info.pNext = nullptr;
    pipeline_cache_info.initialDataSize = 0u;
    pipeline_cache_info.pInitialData = nullptr;
    pipeline_cache_info.flags = 0u;
    VkResult  result = vkCreatePipelineCache(device->getDevice(), &pipeline_cache_info, NULL, &m_pipeline_cache);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline cache!");
    }

    m_pipeline_config = std::make_shared<PipelineConfig>();
    m_pipeline_config->init(pipeline_data);

    m_pipeline_type = PipelineType::GRAPHICS;
    
    m_pipeline_layout_info = VkPipelineLayoutCreateInfo{};
    m_pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    m_desc_set_layouts = getVkDescriptorSetLayouts(m_pipeline_config->getShaderNames(), desc_manager, shader_manager);
    m_pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(m_desc_set_layouts.size());
    m_pipeline_layout_info.pSetLayouts = m_desc_set_layouts.data();

    m_push_constants = getPushConstantRanges(m_pipeline_config->getShaderNames(), shader_manager);
    m_pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(m_push_constants.size());
    m_pipeline_layout_info.pPushConstantRanges = m_push_constants.data();

    m_shaders_infos = getPipelineShaderCreateInfo(m_pipeline_config->getShaderNames(), shader_manager);
    m_input_info = getVertexInputInfo(m_pipeline_config->getShaderNames(), shader_manager);

    m_scissor = VkRect2D{};
    m_scissor.offset = {0, 0};
    m_scissor.extent = viewport_extent;
    
    m_viewport = VkViewport{};
    m_viewport.x = 0.0f;
    m_viewport.y = 0.0f;
    m_viewport.width = (float)viewport_extent.width;
    m_viewport.height = (float)viewport_extent.height;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;
    
    m_viewport_state_info = VkPipelineViewportStateCreateInfo{};
    m_viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_viewport_state_info.viewportCount = 1u;
    m_viewport_state_info.pViewports = &m_viewport;
    m_viewport_state_info.scissorCount = 1u;
    m_viewport_state_info.pScissors = &m_scissor;

    m_assembly_info = VkPipelineInputAssemblyStateCreateInfo{};
    m_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_assembly_info.topology = m_pipeline_config->getTopology();
    m_assembly_info.primitiveRestartEnable = m_pipeline_config->getPrimitiveRestartEnable();

    m_tessellation_info = m_pipeline_config->getTessellationInfo();
    
    m_pipeline_info = VkGraphicsPipelineCreateInfo{};
    m_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_pipeline_info.flags = m_pipeline_config->getPipelineCreateFlags();
    m_pipeline_info.stageCount = static_cast<uint32_t>(m_shaders_infos.size());
    m_pipeline_info.pStages = m_shaders_infos.data();
    m_pipeline_info.pVertexInputState = &m_input_info;
    m_pipeline_info.pInputAssemblyState = &m_assembly_info;
    m_pipeline_info.pTessellationState = &m_tessellation_info;
    m_pipeline_info.pViewportState = &m_viewport_state_info;
    m_pipeline_info.pRasterizationState = &m_pipeline_config->getRasterizerInfo();
    m_pipeline_info.pMultisampleState = &m_pipeline_config->getMultisampleInfo();
    m_pipeline_info.pDepthStencilState = &m_pipeline_config->getDepthStencilInfo();
    m_pipeline_info.pColorBlendState = &m_pipeline_config->getColorBlendInfo();
    m_pipeline_info.pDynamicState = &m_pipeline_config->getDynamicInfo();
    m_pipeline_info.layout = m_pipeline_layout;
    m_pipeline_info.renderPass = render_pass;
    m_pipeline_info.subpass = 0u;
    m_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    m_pipeline_info.basePipelineIndex = -1;
    
    VkResult result = vkCreateGraphicsPipelines(m_device->getDevice(), VK_NULL_HANDLE, 1, &m_pipeline_info, nullptr, &m_pipeline);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return true;
}


std::vector<VkDescriptorSetLayout> VulkanPipeline::getVkDescriptorSetLayouts(const std::vector<std::string>& shader_names, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shader_manager) const {
    std::unordered_set<VkDescriptorSetLayout> temp;
    for(const std::string& shader_name : shader_names) {
        for(const std::string& desc_name : shader_manager->getShader(shader_name)->getShaderSignature()->getDescSetNames()) {
            temp.insert(desc_manager->getDescSetLayout(desc_name)->getDescriptorSetLayout());
        }
    }

    return {temp.begin(), temp.end()};
}

std::vector<VkPushConstantRange> VulkanPipeline::getPushConstantRanges(const std::vector<std::string>& shader_names, std::shared_ptr<VulkanShadersManager> shader_manager) {
    std::vector<VkPushConstantRange> push_constants;
    uint32_t offset = 0u;
    for(const std::string& shader_name : shader_names) {
        for(VkPushConstantRange constant : shader_manager->getShader(shader_name)->getShaderSignature()->getPushConstantsRanges()) {
            VkPushConstantRange push_constant = constant;
            push_constant.offset = offset;

            push_constants.push_back(push_constant);
            offset += constant.size;
        }
    }
    return push_constants;
}

std::vector<VkPipelineShaderStageCreateInfo> VulkanPipeline::getPipelineShaderCreateInfo(const std::vector<std::string>& shader_names, std::shared_ptr<VulkanShadersManager> shader_manager) {
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_create_info;
    for(const std::string& shader_name : shader_names) {
        pipeline_shader_create_info.push_back(shader_manager->getShader(shader_name)->getShaderInfo());
    }

    return pipeline_shader_create_info;
}

VkPipelineVertexInputStateCreateInfo VulkanPipeline::getVertexInputInfo(const std::vector<std::string>& shader_names, std::shared_ptr<VulkanShadersManager> shader_manager) {
    m_input_binding_descs.clear();
    m_input_attribute_descs.clear();
    for(const std::string& shader_name : shader_names) {
        if(shader_manager->getShader(shader_name)->getShaderSignature()->getStage() == VK_SHADER_STAGE_VERTEX_BIT) {
            for(size_t b = 0u; b < shader_manager->getShader(shader_name)->getShaderSignature()->getNumInputAttributeBindings(); ++b) {
                const VertexFormat& vertex_format = shader_manager->getShader(shader_name)->getShaderSignature()->getInputAttributes(b);
                VkVertexInputBindingDescription binding_desc{};
                binding_desc.binding = b;
                binding_desc.stride = vertex_format.getVertexSize();
                binding_desc.inputRate = vertex_format.getInputRate();
                m_input_binding_descs.push_back(binding_desc);

                size_t sz = vertex_format.getVertexAttribCount();
                for (size_t i = 0; i < sz; ++i) {
                    VkVertexInputAttributeDescription attribute_desc{};
                    attribute_desc.binding = b;
                    attribute_desc.location = i;
                    attribute_desc.format = getVkFormat(vertex_format.getAttribFormat(i));
                    attribute_desc.offset = vertex_format.getOffset(i);

                    m_input_attribute_descs.push_back(attribute_desc);
                }
            }

            VkPipelineVertexInputStateCreateInfo vertex_input_info{};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(m_input_binding_descs.size());
            vertex_input_info.pVertexBindingDescriptions = m_input_binding_descs.data();
            vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_input_attribute_descs.size());
            vertex_input_info.pVertexAttributeDescriptions = m_input_attribute_descs.data();

            return vertex_input_info;
        }
    }
    return VkPipelineVertexInputStateCreateInfo{};
}

void VulkanPipeline::destroy() {
    vkDestroyPipeline(m_device->getDevice(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device->getDevice(), m_pipeline_layout, nullptr);
}

void VulkanPipeline::saveCacheToFile(VkPipelineCache cache, const std::string& file_name) {
    size_t cache_data_size;
    // Determine the size of the cache data.
    VkResult result = vkGetPipelineCacheData(m_device->getDevice(), cache, &cache_data_size, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to read pipeline cache size!");
    }

    if (cache_data_size == 0u) return;

    // Allocate a temporary store for the cache data.
    std::vector<char> data(cache_data_size);

    // Retrieve the actual data from the cache.
    result = vkGetPipelineCacheData(m_device->getDevice(), cache, &cache_data_size, data.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to write pipeline cache to file!");
    }
    // Open the file and write the data to it.
    writeFile(file_name, data.size(), data.data());
}

VulkanPipeline::PipelineType VulkanPipeline::getPipelineType() const {
    return m_pipeline_type;
}

VkPipeline VulkanPipeline::getPipeline() const {
    return m_pipeline;
}

VkPipelineLayout VulkanPipeline::getPipelineLayout() const {
    return m_pipeline_layout;
}

const std::vector<VkPipelineShaderStageCreateInfo>& VulkanPipeline::getShadersInfo() const {
    return m_shaders_info;
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
    //multisample_info.sampleShadingEnable = VK_TRUE;
    multisample_info.rasterizationSamples = pipeline_cfg.msaa_samples;
    //multisample_info.minSampleShading = 1.0f;
    //multisample_info.pSampleMask = nullptr;
    //multisample_info.alphaToCoverageEnable = VK_TRUE;
    //multisample_info.alphaToOneEnable = VK_TRUE;
    
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
    VkResult result = vkCreateGraphicsPipelines(m_device->getDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return graphics_pipeline;
}