#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "render_resource.h"

class VulkanDevice;
class VulkanSampler;

class DescSetLayout {
public:

    struct UpdateMetadata {
        enum class SamplerType { NONE, INLINE, DEFAULT, FROM_IMAGEBUFFER };

        RenderResource::Type resource_type;
        std::string buffer_resource_type_name;

        SamplerType sampler_type;
        std::shared_ptr<VulkanSampler> image_sampler;
        std::string image_resource_type_name;
        std::string image_view_type_name;
        VkImageLayout read_image_layout;
    };

    using DescSetBindings = std::vector<VkDescriptorSetLayoutBinding>;
    using DescSetBindingsMetadata = std::vector<UpdateMetadata>;

    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path);
    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& descriptor_sets_node);
    void destroy();
    
    const std::string& getName() const;
    const std::string& getAllocatorName() const;
    const DescSetBindings& getBindings() const;
    VkDescriptorSetLayoutBinding getBinding(VkDescriptorType desc_type) const;
    VkDescriptorSetLayoutBinding getBinding(uint32_t binding_num) const;
    bool haveBindingType(VkDescriptorType desc_type) const;
    bool haveBindingNum(uint32_t binding_num) const;
    const std::vector<std::shared_ptr<VulkanSampler>>& getImmutableSamplers() const;
    const std::vector<VkSampler>& getImmutableSamplersPtr() const;
    VkDescriptorSetLayoutCreateInfo getDescriptorSetLayoutInfo() const;
    VkDescriptorSetLayout getDescriptorSetLayout() const;

private:
    std::shared_ptr<VulkanDevice> m_device;

    std::string m_name;
    std::string m_allocator_name;
    DescSetBindings m_bindings;
    DescSetBindingsMetadata m_bindings_metadata;
    std::vector<std::shared_ptr<VulkanSampler>> m_immutable_samplers;
    std::vector<VkSampler> m_immutable_samplers_ptr;
    VkDescriptorSetLayoutCreateInfo m_desc_layout_info;
    VkDescriptorSetLayout m_desc_layout;
};