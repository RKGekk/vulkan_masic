#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

class VulkanDevice;
class VulkanDescriptor;
class VulkanDescriptorsManager;
class VulkanShadersManager;
class PipelineConfig;
class VulkanRenderPass;
class VulkanShader;
class DescSetLayout;

class VulkanPipeline {
public:
    enum class PipelineType {
        GRAPHICS,
        COMPUTE
    };

    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& pipeline_data, VkExtent2D viewport_extent, std::shared_ptr<VulkanRenderPass> render_pass, uint32_t subpass, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shader_manager);
    void destroy();

    PipelineType getPipelineType() const;
    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
    const std::vector<VkPipelineShaderStageCreateInfo>& getShadersInfo() const;
    const std::shared_ptr<PipelineConfig>& getPipelineConfig() const;
    const VkGraphicsPipelineCreateInfo& getPipelineInfo() const;
    const std::shared_ptr<VulkanRenderPass>& getRenderPass();
    VkPipelineVertexInputStateCreateInfo getInputInfo() const;
    std::shared_ptr<VulkanShader> getShader(VkShaderStageFlagBits stage);
    const std::unordered_map<VkShaderStageFlagBits, std::shared_ptr<VulkanShader>>& getShaders() const;
    const std::unordered_map<uint32_t, std::shared_ptr<DescSetLayout>>& getDescLayouts() const;

private:
    std::vector<VkDescriptorSetLayout> getVkDescriptorSetLayouts(const std::vector<std::string>& shader_names, const std::shared_ptr<VulkanDescriptorsManager>& desc_manager, const std::shared_ptr<VulkanShadersManager>& shader_manager) const;
    std::vector<VkPushConstantRange> getPushConstantRanges(const std::vector<std::string>& shader_names, const std::shared_ptr<VulkanShadersManager>& shader_manager);
    std::vector<VkPipelineShaderStageCreateInfo> getPipelineShaderCreateInfo(const std::vector<std::string>& shader_names, const std::shared_ptr<VulkanShadersManager>& shader_manager);
    std::unordered_map<VkShaderStageFlagBits, std::shared_ptr<VulkanShader>> createShadersMap(const std::shared_ptr<VulkanShadersManager>& shader_manager) const;
    VkPipelineVertexInputStateCreateInfo getVertexInputInfo(const std::vector<std::string>& shader_names, const std::shared_ptr<VulkanShadersManager>& shader_manager);
    std::unordered_map<uint32_t, std::shared_ptr<DescSetLayout>> createDescSlotToLayoutMap(const std::shared_ptr<VulkanDescriptorsManager>& desc_manager, const std::shared_ptr<VulkanShadersManager>& shader_manager) const;
    void saveCacheToFile(VkPipelineCache cache, const std::string& file_name);
    
    std::shared_ptr<VulkanDevice> m_device;
    std::string m_name;
    PipelineType m_pipeline_type;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders_infos;
    std::unordered_map<VkShaderStageFlagBits, std::shared_ptr<VulkanShader>> m_shaders;
    VkPipelineVertexInputStateCreateInfo m_input_info;
    std::vector<VkVertexInputBindingDescription> m_input_binding_descs;
    std::vector<VkVertexInputAttributeDescription> m_input_attribute_descs;
    std::vector<VkDescriptorSetLayout> m_desc_set_layouts;
    std::unordered_map<uint32_t, std::shared_ptr<DescSetLayout>> m_desc_slot_to_layout_map;
    std::vector<VkPushConstantRange> m_push_constants;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipelineLayoutCreateInfo m_pipeline_layout_info;
    VkRect2D m_scissor;
    VkViewport m_viewport;
    VkPipelineViewportStateCreateInfo m_viewport_state_info;
    VkGraphicsPipelineCreateInfo m_pipeline_info;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    std::shared_ptr<PipelineConfig> m_pipeline_config;
    VkPipelineCache m_pipeline_cache = VK_NULL_HANDLE;
    std::shared_ptr<VulkanRenderPass> m_render_pass;
};