#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include "vertex_format.h"
#include "../api/vulkan_sampler.h"

#include <vector>
#include <string>
#include <map>

class VulkanDevice;

class ShaderSignature {
public:
    using DescSetSlot = uint32_t;
    using DescSetBindings = std::map<DescSetSlot, std::vector<VkDescriptorSetLayoutBinding>>;

    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path);
    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data);
    void destroy();

    const VertexFormat& getVertexFormat() const;

    const std::string& getName() const;
    const std::string& getFileName() const;
    const std::string& getEntryPointName() const;
    VkFlags getShaderCreateFlags() const; // VkShaderCreateFlagBitsEXT
    VkFlags getPipelineShaderStageCreateFlags() const;
    VkShaderStageFlagBits getStage() const;
    const VertexFormat& getInputAttributes() const;
    const DescSetBindings& getBindings() const;
    const std::vector<std::shared_ptr<VulkanSampler>>& getImmutableSamplers() const;
    const std::vector<VkSampler>& getImmutableSamplersPtr() const;
    const std::vector<VkPushConstantRange>& getPushConstantsRanges() const;
    const std::vector<VkSpecializationMapEntry>& getSpecializationConstantsMap() const;
    const std::vector<char>& getSpecializationConstantsData() const;
    const VkSpecializationInfo& getSpecializationInfo() const;

private:
    VertexFormat m_vertex_format;
    std::string m_name;
    std::string m_file_name;
    std::string m_entry_point_name;
    VkFlags m_create_flags; // VkShaderCreateFlagBitsEXT
    VkFlags m_pipeline_shader_stage_create_flags; // VkPipelineShaderStageCreateFlags
    VkShaderStageFlagBits m_stage;
    VertexFormat m_input_attributes;
    DescSetBindings m_bindings;
    std::vector<std::shared_ptr<VulkanSampler>> m_immutable_samplers;
    std::vector<VkSampler> m_immutable_samplers_ptr;
    std::vector<VkPushConstantRange> m_push_constants;
    std::vector<VkSpecializationMapEntry> m_specialization_constants;
    std::vector<char> m_specialization_constants_data;
    VkSpecializationInfo m_specialization_info;
};