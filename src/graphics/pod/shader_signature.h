#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include "vertex_format.h"
#include "../api/vulkan_sampler.h"
#include "../api/vulkan_push_constant.h"
#include "push_constant_config.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class VulkanResourcesManager;

class ShaderSignature {
public:
    using SlotNumber = uint32_t;

    bool init(std::shared_ptr<VulkanResourcesManager>& resources_manager, const pugi::xml_node& shader_data);

    const std::string& getName() const;
    const std::string& getFileName() const;
    const std::string& getEntryPointName() const;
    VkFlags getShaderCreateFlags() const; // VkShaderCreateFlagBitsEXT
    VkFlags getPipelineShaderStageCreateFlags() const;
    VkShaderStageFlagBits getStage() const;

    const VertexFormat& getInputAttributes(size_t binding) const;
    VertexFormat::BindingNum getFirstInputAttribute(std::function<bool(const VertexFormat&)> fn) const;
    const std::vector<VertexFormat>& getInputAttributes() const;
    size_t getNumInputAttributeBindings() const;

    VkIndexType getIndexType() const;
    uint32_t getIndexTypeBytesCount() const;
    void setIndexType(VkIndexType idx_type);

    const std::string& getIndexBufferBindingName() const;
    void setIndexBufferBindingName(std::string name);

    uint32_t getIndexBufferOffset() const;
    void setIndexBufferOffset(uint32_t offset);

    const std::string& getIndexBufferResourceType() const;
    void setIndexBufferResourceType(std::string res_type);

    const std::unordered_map<SlotNumber, std::string>& getDescSetNames() const;
    
    std::shared_ptr<VulkanPushConstant>& getPushConstants();
    const std::vector<VkSpecializationMapEntry>& getSpecializationConstantsMap() const;
    const std::vector<char>& getSpecializationConstantsData() const;
    const VkSpecializationInfo& getSpecializationInfo() const;

private:
    std::string m_name;
    std::string m_file_name;
    std::string m_entry_point_name;
    VkFlags m_create_flags; // VkShaderCreateFlagBitsEXT
    VkFlags m_pipeline_shader_stage_create_flags; // VkPipelineShaderStageCreateFlags
    VkShaderStageFlagBits m_stage;
    
    std::vector<VertexFormat> m_input_attributes; // by binding idx

    VkIndexType m_index_type;
    std::string m_index_buffer_binding_name;
    uint32_t m_index_buffer_offset;
    std::string m_index_buffer_resource_type;

    std::unordered_map<SlotNumber, std::string> m_desc_set_names;
    std::shared_ptr<VulkanPushConstant> m_push_constants;
    std::vector<VkSpecializationMapEntry> m_specialization_constants;
    std::vector<char> m_specialization_constants_data;
    VkSpecializationInfo m_specialization_info;
};