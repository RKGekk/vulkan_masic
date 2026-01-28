#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../pod/descriptor_set_layout.h"

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

class VulkanDescriptor {
public:

    bool init(VkDevice device, std::shared_ptr<DescSetLayout> layout, VkDescriptorSet descriptor_set);
    void destroy();

    const std::string& getName() const;
    VkDescriptorSetLayout getDescLayout() const;
    VkDescriptorSet getDescriptorSet() const;
    
    void updateDescSampler(VkSampler sampler);
    void updateDescImage(VkImageView image_view, VkImageLayout image_layout);
    void updateDescCombinedImage(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
    void updateDescImageInfo(uint32_t binding, VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);

    void updateDescBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0u);
    void updateDescTexel(VkBufferView buffer_view);

private:

    VkDevice m_device;

	VkDescriptorSet m_descriptor_set;
    std::shared_ptr<DescSetLayout> m_layout;
};