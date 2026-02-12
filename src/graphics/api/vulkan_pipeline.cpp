#include "vulkan_pipeline.h"

#include "vulkan_device.h"
#include "../pod/pipeline_config.h"
#include "vulkan_shaders_manager.h"
#include "vulkan_pipelines_manager.h"
#include "vulkan_descriptors_manager.h"
#include "vulkan_render_pass.h"
#include "../../tools/string_tools.h"

#include <array>
#include <filesystem>
#include <stdexcept>
#include <unordered_set>

bool VulkanPipeline::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& pipeline_data, VkExtent2D viewport_extent, std::shared_ptr<VulkanRenderPass> render_pass, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shader_manager) {
    m_device = device;
    m_render_pass = render_pass;

    m_pipeline_config = std::make_shared<PipelineConfig>();
    m_pipeline_config->init(pipeline_data);

    m_pipeline_type = PipelineType::GRAPHICS;
    m_name = m_pipeline_config->getName();

    std::filesystem::path pipeline_cache_file_path(m_name);
    std::vector<char> pipeline_cache_data;
    bool pipeline_cache_exists = std::filesystem::exists(pipeline_cache_file_path);
	if(pipeline_cache_exists) {
        pipeline_cache_data = readFile(pipeline_cache_file_path.string());
    }

    VkPipelineCacheCreateInfo pipeline_cache_info{};
    pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipeline_cache_info.pNext = nullptr;
    pipeline_cache_info.initialDataSize = pipeline_cache_data.size();
    pipeline_cache_info.pInitialData = pipeline_cache_data.data();
    pipeline_cache_info.flags = 0u;
    VkResult  result = vkCreatePipelineCache(device->getDevice(), &pipeline_cache_info, NULL, &m_pipeline_cache);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline cache!");
    }
    
    m_pipeline_layout_info = VkPipelineLayoutCreateInfo{};
    m_pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    m_desc_set_layouts = getVkDescriptorSetLayouts(m_pipeline_config->getShaderNames(), desc_manager, shader_manager);
    m_pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(m_desc_set_layouts.size());
    m_pipeline_layout_info.pSetLayouts = m_desc_set_layouts.data();

    m_push_constants = getPushConstantRanges(m_pipeline_config->getShaderNames(), shader_manager);
    m_pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(m_push_constants.size());
    m_pipeline_layout_info.pPushConstantRanges = m_push_constants.data();

    VkResult result = vkCreatePipelineLayout(m_device->getDevice(), &m_pipeline_layout_info, nullptr, &m_pipeline_layout);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

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

    m_pipeline_info = VkGraphicsPipelineCreateInfo{};
    m_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_pipeline_info.flags = m_pipeline_config->getPipelineCreateFlags();
    m_pipeline_info.stageCount = static_cast<uint32_t>(m_shaders_infos.size());
    m_pipeline_info.pStages = m_shaders_infos.data();
    m_pipeline_info.pVertexInputState = &m_input_info;
    m_pipeline_info.pInputAssemblyState = &m_pipeline_config->getAssemblyInfo();
    m_pipeline_info.pTessellationState = &m_pipeline_config->getTessellationInfo();
    m_pipeline_info.pViewportState = &m_viewport_state_info;
    m_pipeline_info.pRasterizationState = &m_pipeline_config->getRasterizerInfo();
    m_pipeline_info.pMultisampleState = &m_pipeline_config->getMultisampleInfo();
    m_pipeline_info.pDepthStencilState = &m_pipeline_config->getDepthStencilInfo();
    m_pipeline_info.pColorBlendState = &m_pipeline_config->getColorBlendInfo();
    m_pipeline_info.pDynamicState = &m_pipeline_config->getDynamicInfo();
    m_pipeline_info.layout = m_pipeline_layout;
    m_pipeline_info.renderPass = m_render_pass->getRenderPass();
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
                    attribute_desc.format = getAttributeFormat(vertex_format.getAttribFormat(i));
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
    //saveCacheToFile(m_pipeline_cache, m_name);

    vkDestroyPipelineCache(m_device->getDevice(), m_pipeline_cache, nullptr);
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
    return m_shaders_infos;
}

const std::shared_ptr<PipelineConfig>& VulkanPipeline::getPipelineConfig() const {
    return m_pipeline_config;
}

std::shared_ptr<VulkanRenderPass> VulkanPipeline::getRenderPass() {
    return m_render_pass;
}

VkPipelineVertexInputStateCreateInfo VulkanPipeline::getInputInfo() const {
    return m_input_info;
}