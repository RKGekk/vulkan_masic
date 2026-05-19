#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "vertex_format.h"

class PushConstantConfig {
public:
    using ShaderConstantName = std::string;
    using ShaderConstantMetadataId = size_t;
    using ShaderConstantOffset = uint32_t;
    struct ShaderConstant {
        VkFormat vk_format;
        VertexAttributeGLSLFormat glsl_format;
        uint32_t size;
        uint32_t allignment;
        ShaderConstantOffset offset;
        std::string name;
    };

    bool init(std::string name, const pugi::xml_node& const_data);

    const std::string& getName() const;

    const VkPushConstantRange& getPushConstantRange() const;
    void setPushConstantRangeOffset(uint32_t offset);
    const std::vector<ShaderConstant>& getAllPushConstantsMetadata() const;
    const ShaderConstant& getPushConstantsMetadata(const ShaderConstantName& name) const;
    const std::unordered_map<ShaderConstantName, ShaderConstantMetadataId>& getPushConstantsNamesMap() const;
    uint32_t getLargestMemberAlignment() const; // The total size of a push constant block struct must be rounded up to a multiple of its largest member's alignment. For calculation of trailing padding.
    uint32_t getTotalSize() const;
    uint32_t getRawSize() const;

    static uint32_t getGLSLAlignment(VertexAttributeGLSLFormat glsl_format);

    std::shared_ptr<PushConstantConfig> makeInstance(std::string name, VkShaderStageFlags shader_stages) const;

private:
    std::string m_name;
    VkPushConstantRange m_push_constant_range;
    std::vector<ShaderConstant> m_push_constants_metadata;
    std::unordered_map<ShaderConstantName, ShaderConstantMetadataId> m_push_constants_names_map;
    uint32_t m_largest_member_alignment; // The total size of a push constant block struct must be rounded up to a multiple of its largest member's alignment. For calculation of trailing padding.
    uint32_t m_total_size;
    uint32_t m_raw_size;
};