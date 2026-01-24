#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <vector>

class VulkanDevice;
class VulkanDescriptorsManager;
class VulkanShadersManager;
class PipelineConfig;

class VulkanPipeline {
public:
    enum class PipelineType {
        GRAPHICS,
        COMPUTE
    };

    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& pipeline_data, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shader_manager);
    void destroy();

    PipelineType getPipelineType() const;
    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
    const std::vector<VkPipelineShaderStageCreateInfo>& getShadersInfo() const;

private:
    std::vector<VkDescriptorSetLayout> getVkDescriptorSetLayouts(const std::vector<std::string>& shader_names, std::shared_ptr<VulkanDescriptorsManager> desc_manager, std::shared_ptr<VulkanShadersManager> shader_manager) const;

    VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout>& desc_set_layouts) const;
    VkPipeline createPipeline(const PipelineCfg& pipeline_cfg) const;

    std::shared_ptr<VulkanDevice> m_device;

    PipelineType m_pipeline_type;
    std::vector<VkDescriptorSetLayout> m_desc_set_layouts;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipelineLayoutCreateInfo m_pipeline_layout_info;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    std::shared_ptr<PipelineConfig> m_pipeline_config;
};