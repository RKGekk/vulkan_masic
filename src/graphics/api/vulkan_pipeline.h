#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <vector>

#include "vulkan_shaders_manager.h"

class VulkanDevice;

class VulkanPipeline {
public:
    enum class PipelineType {
        GRAPHICS,
        COMPUTE
    };

    bool init(std::shared_ptr<VulkanDevice> device, const PipelineCfg& pipeline_cfg);
    bool init(std::shared_ptr<VulkanDevice> device, const PipelineCfg& pipeline_cfg, VkPipeline pipeline);
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path, std::shared_ptr<VulkanShadersManager> shader_manager);
    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& pipelines_data, std::shared_ptr<VulkanShadersManager> shader_manager);
    void destroy();

    PipelineType getPipelineType() const;
    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
    const std::vector<VkPipelineShaderStageCreateInfo>& getShadersInfo() const;

private:
    VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout>& desc_set_layouts) const;
    VkPipeline createPipeline(const PipelineCfg& pipeline_cfg) const;

    std::shared_ptr<VulkanDevice> m_device;

    PipelineType m_pipeline_type;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders_info;
};