#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

struct VulkanDescriptorBinding {
    VkDescriptorSetLayoutBinding layout_binding;
    std::shared_ptr<VkDescriptorImageInfo> image_info;
    std::shared_ptr<VkDescriptorBufferInfo> buffer_info;
    VkSampler sampler;
};

class VulkanDescriptor {
public:

    using Binding = std::vector<VulkanDescriptorBinding>;
    bool init(VkDevice device, std::vector<Binding> sets, uint32_t set_copy_ct);
    void destroy();

    VkDescriptorSetLayout getDescLayouts() const;
    const std::vector<VkDescriptorSet>& getDescriptorSets() const;
    const std::vector<Binding>& getBingingsForSets() const;

private:
    VkDescriptorSetLayout createDescSetLayout();
    VkDescriptorPool createDescPool();
    std::vector<VkDescriptorSet> createDescSets();

    std::unordered_map<VkDescriptorType, size_t> getTypesCount();
    std::vector<VkDescriptorSetLayoutBinding> getDescSetLayoutBindings();

    VkDevice m_device;

    uint32_t m_frame_count;
    std::vector<Binding> m_bingings_for_sets;
	VkDescriptorSetLayout m_desc_layout;
	VkDescriptorPool m_descriptor_pool;
	std::vector<VkDescriptorSet> m_descriptor_sets;
};