#include "vulkan_pipeline.h"

#include "vulkan_device.h"
#include "../pod/pipeline_config.h"
#include "../pod/descriptor_set_layout.h"
#include "vulkan_shaders_manager.h"
#include "vulkan_shader.h"
#include "vulkan_pipelines_manager.h"
#include "vulkan_descriptors_manager.h"
#include "vulkan_render_pass.h"
#include "../../tools/string_tools.h"
#include "../../application.h"
#include "../vulkan_renderer.h"

#include <array>
#include <filesystem>
#include <stdexcept>
#include <unordered_set>

bool VulkanPipeline::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& pipeline_data, VkExtent2D viewport_extent, std::shared_ptr<VulkanRenderPass> render_pass, uint32_t subpass, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shader_manager) {
    using namespace std::literals;
    
    m_device = std::move(device);
    m_shader_manager = std::move(shader_manager);
    m_render_pass = std::move(render_pass);

    m_pipeline_config = std::make_shared<PipelineConfig>();
    m_pipeline_config->init(m_device, pipeline_data);

    m_pipeline_type = PipelineType::GRAPHICS;
    m_name = m_pipeline_config->getName();
    m_shaders = createShadersMap(m_shader_manager);
    m_desc_slot_to_layout_map = createDescSlotToLayoutMap(desc_manager, m_shader_manager);

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
    VkResult  result = vkCreatePipelineCache(m_device->getDevice(), &pipeline_cache_info, NULL, &m_pipeline_cache);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline cache!");
    }
    
    m_pipeline_layout_info = VkPipelineLayoutCreateInfo{};
    m_pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    m_desc_set_layouts = getVkDescriptorSetLayouts(m_pipeline_config->getShaderNames(), desc_manager, m_shader_manager);
    m_pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(m_desc_set_layouts.size());
    m_pipeline_layout_info.pSetLayouts = m_desc_set_layouts.data();


    m_push_constants = getPushConstantRanges(m_shader_manager);
    m_max_push_constants_size = m_device->getDeviceAbilities().props.limits.maxPushConstantsSize;
    m_current_push_constants_size = getPushConstantsSize(m_push_constants);
    m_pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(m_push_constants.size());
    if(m_push_constants.empty()) {
        m_pipeline_layout_info.pPushConstantRanges = nullptr;
    }
    else {
        m_pipeline_layout_info.pPushConstantRanges = m_push_constants.data();
    }

    result = vkCreatePipelineLayout(m_device->getDevice(), &m_pipeline_layout_info, nullptr, &m_pipeline_layout);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    m_shaders_infos = getPipelineShaderCreateInfo(m_pipeline_config->getShaderNames(), m_shader_manager);
    m_input_info = getVertexInputInfo(m_pipeline_config->getShaderNames(), m_shader_manager);

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
    m_pipeline_info.subpass = subpass;
    m_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    m_pipeline_info.basePipelineIndex = -1;
    
    result = vkCreateGraphicsPipelines(m_device->getDevice(), VK_NULL_HANDLE, 1, &m_pipeline_info, nullptr, &m_pipeline);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

#ifndef NDEBUG
    std::string pipeline_name = "pipeline_"s + m_name;
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    name_info.objectHandle = (uint64_t)m_pipeline;
    name_info.pObjectName = pipeline_name.c_str();

    vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif

    return true;
}

void VulkanPipeline::destroy() {
    //saveCacheToFile(m_pipeline_cache, m_name);

    vkDestroyPipelineCache(m_device->getDevice(), m_pipeline_cache, nullptr);
    vkDestroyPipeline(m_device->getDevice(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device->getDevice(), m_pipeline_layout, nullptr);
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

const VkGraphicsPipelineCreateInfo& VulkanPipeline::getPipelineInfo() const {
    return m_pipeline_info;
}

const std::shared_ptr<VulkanRenderPass>& VulkanPipeline::getRenderPass() {
    return m_render_pass;
}

VkPipelineVertexInputStateCreateInfo VulkanPipeline::getInputInfo() const {
    return m_input_info;
}

std::shared_ptr<VulkanShader> VulkanPipeline::getShader(VkShaderStageFlagBits stage) {
    if(!m_shaders.contains(stage)) return nullptr;
    return m_shaders.at(stage);
}

const std::unordered_map<VkShaderStageFlagBits, std::shared_ptr<VulkanShader>>& VulkanPipeline::getShaders() const {
    return m_shaders;
}

const std::unordered_map<uint32_t, std::shared_ptr<DescSetLayout>>& VulkanPipeline::getDescLayouts() const {
    return m_desc_slot_to_layout_map;
}

bool VulkanPipeline::has_push_constants() const {
    return m_push_constants.size() > 0u;
}

void VulkanPipeline::build_push_constants() {
    if (!has_push_constants()) return;

    std::unordered_set<PushConstantConfig::ShaderConstantName> push_const_names;
    uint32_t offset = 0u;
    m_push_constants_data.resize(m_current_push_constants_size);
    for(const std::string& shader_name : m_pipeline_config->getShaderNames()) {
        if(!m_shader_manager->getShader(shader_name)->getShaderSignature()->getPushConstants()) continue;

        std::shared_ptr<VulkanPushConstant>& push_constant = m_shader_manager->getShader(shader_name)->getShaderSignature()->getPushConstants();
        if(push_const_names.contains(push_constant->getName())) continue;
        push_const_names.insert(push_constant->getName());
        uint32_t push_const_size = push_constant->getConstConfig()->getTotalSize();
        push_constant->getConstConfig()->setPushConstantRangeOffset(offset);
        memcpy(
            m_push_constants_data.data() + offset,
            push_constant->getData().data(),
            push_const_size
        );
        
        offset += push_const_size;
    }
}

void VulkanPipeline::attach_push_constants(VkCommandBuffer command_buffer) {
    if (!has_push_constants()) return;

    std::unordered_set<PushConstantConfig::ShaderConstantName> push_const_names;
    uint32_t offset = 0u;
    for(const std::string& shader_name : m_pipeline_config->getShaderNames()) {
        if(!m_shader_manager->getShader(shader_name)->getShaderSignature()->getPushConstants()) continue;
        
        std::shared_ptr<VulkanPushConstant>& push_constant = m_shader_manager->getShader(shader_name)->getShaderSignature()->getPushConstants();
        if(push_const_names.contains(push_constant->getName())) continue;
        push_const_names.insert(push_constant->getName());
        uint32_t push_const_size = push_constant->getConstConfig()->getTotalSize();
        
        vkCmdPushConstants(
            command_buffer,
            m_pipeline_layout,
            push_constant->getConstConfig()->getPushConstantRange().stageFlags,
            push_constant->getConstConfig()->getPushConstantRange().offset,
            push_constant->getConstConfig()->getPushConstantRange().size,
            m_push_constants_data.data()
        );
        
        offset += push_const_size;
    }
}

std::vector<VkDescriptorSetLayout> VulkanPipeline::getVkDescriptorSetLayouts(const std::vector<std::string>& shader_names, const std::shared_ptr<VulkanDescriptorsManager>& desc_manager, const std::shared_ptr<VulkanShadersManager>& shader_manager) const {
    std::unordered_set<VkDescriptorSetLayout> temp;
    for(const std::string& shader_name : shader_names) {
        for(const auto&[slot, desc_name] : shader_manager->getShader(shader_name)->getShaderSignature()->getDescSetNames()) {
            temp.insert(desc_manager->getDescSetLayout(desc_name)->getDescriptorSetLayout());
        }
    }

    return {temp.begin(), temp.end()};
}

std::vector<VkPushConstantRange> VulkanPipeline::getPushConstantRanges(const std::shared_ptr<VulkanShadersManager>& shader_manager) {
    std::vector<VkPushConstantRange> push_constants;
    std::unordered_set<PushConstantConfig::ShaderConstantName> push_const_names;
    uint32_t offset = 0u;
    for(const std::string& shader_name : m_pipeline_config->getShaderNames()) {
        std::shared_ptr<VulkanPushConstant>& push_constant = shader_manager->getShader(shader_name)->getShaderSignature()->getPushConstants();
        if(!push_constant) continue;
        if(push_const_names.contains(push_constant->getName())) continue;
        push_const_names.insert(push_constant->getName());
        VkPushConstantRange range = push_constant->getConstConfig()->getPushConstantRange();
        range.offset = offset;
        push_constants.push_back(range);

        offset += range.size;
    }
    return push_constants;
}

uint32_t VulkanPipeline::getPushConstantsSize(const std::vector<VkPushConstantRange>& const_ranges) const {
    return const_ranges.size() > 0u ? const_ranges.back().offset + const_ranges.back().size : 0u;
}

std::vector<VkPipelineShaderStageCreateInfo> VulkanPipeline::getPipelineShaderCreateInfo(const std::vector<std::string>& shader_names, const std::shared_ptr<VulkanShadersManager>& shader_manager) {
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_create_info;
    for(const std::string& shader_name : shader_names) {
        pipeline_shader_create_info.push_back(shader_manager->getShader(shader_name)->getShaderInfo());
    }

    return pipeline_shader_create_info;
}

VkPipelineVertexInputStateCreateInfo VulkanPipeline::getVertexInputInfo(const std::vector<std::string>& shader_names, const std::shared_ptr<VulkanShadersManager>& shader_manager) {
    m_input_binding_descs.clear();
    m_input_attribute_descs.clear();

    if(!m_shaders.contains(VK_SHADER_STAGE_VERTEX_BIT)) return VkPipelineVertexInputStateCreateInfo{};

    const std::shared_ptr<VulkanShader>& vertex_shader = m_shaders.at(VK_SHADER_STAGE_VERTEX_BIT);
    for(size_t b = 0u; b < vertex_shader->getShaderSignature()->getNumInputAttributeBindings(); ++b) {
        const VertexFormat& vertex_format = vertex_shader->getShaderSignature()->getInputAttributes(b);
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
            attribute_desc.format = vertex_format.getAttribInternalFormat(i);
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

std::unordered_map<VkShaderStageFlagBits, std::shared_ptr<VulkanShader>> VulkanPipeline::createShadersMap(const std::shared_ptr<VulkanShadersManager>& shader_manager) const {
    std::unordered_map<VkShaderStageFlagBits, std::shared_ptr<VulkanShader>> result;

    for(const std::string& shader_name : m_pipeline_config->getShaderNames()) {
        result[shader_manager->getShader(shader_name)->getShaderSignature()->getStage()] = shader_manager->getShader(shader_name);
    }

    return result;
}

std::unordered_map<uint32_t, std::shared_ptr<DescSetLayout>> VulkanPipeline::createDescSlotToLayoutMap(const std::shared_ptr<VulkanDescriptorsManager>& desc_manager, const std::shared_ptr<VulkanShadersManager>& shader_manager) const {
    std::unordered_map<uint32_t, std::shared_ptr<DescSetLayout>> result;

    for (const auto&[stage, shader] : m_shaders) {
        for(const auto&[slot, desc_set_name] : shader->getShaderSignature()->getDescSetNames()) {
            result[slot] = desc_manager->getDescSetLayout(desc_set_name);
        }
    }

    return result;
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