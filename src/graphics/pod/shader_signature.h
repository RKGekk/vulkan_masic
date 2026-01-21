#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vertex_format.h"
#include "../api/vulkan_sampler.h"

#include <pugixml.hpp>

class VulkanDevice;

class ShaderSignature {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path);
    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data);
    void destroy();

    const VertexFormat& getVertexFormat() const;

private:
    bool add_shader(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& shader_data);

    VertexFormat m_vertex_format;
    std::string m_name;
    std::string m_file_name;
    VkFlags m_create_flags; // VkPipelineCreateFlagBits
    VkShaderStageFlagBits m_stage;
    VertexFormat m_input_attributes;
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
    std::vector<std::shared_ptr<VulkanSampler>> m_immutable_samplers;
    std::vector<VkSampler> m_immutable_samplers_ptr;
    std::vector<VkPushConstantRange> m_push_constants;
    std::vector<VkSpecializationMapEntry> m_specialization_constants;
    std::vector<char> m_specialization_constants_data;
};