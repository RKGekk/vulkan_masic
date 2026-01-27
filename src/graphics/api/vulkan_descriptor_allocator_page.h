#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../pod/descriptor_set_layout.h"

class VulkanDevice;

class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage> {
public:
    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<DescSetLayout> layout, VkDescriptorPoolCreateFlags flags, uint32_t num_descriptors_per_heap = 256u);
    void destroy();
    void reset();

	std::shared_ptr<DescSetLayout> GetDescSetLayout() const;
	bool HasSpace(uint32_t num_descriptors) const;
	uint32_t NumFreeHandles() const;

	std::vector<VkDescriptorSet> Allocate(uint32_t num_descriptors);
    VkDescriptorSet Allocate();
	void Free(std::vector<VkDescriptorSet> desc);
    void Free(VkDescriptorSet desc);
	void ReleaseStaleDescriptors();

    std::shared_ptr<DescriptorAllocatorPage> InitNewPage() const;

private:
    std::unordered_map<VkDescriptorType, size_t> getTypesCount() const;

	std::shared_ptr<VulkanDevice> m_device;
	std::shared_ptr<DescSetLayout> m_layout;
	uint32_t m_num_descriptors_per_heap;

	VkDescriptorPool m_descriptor_pool;
    VkDescriptorPoolCreateInfo m_pool_info;
    std::vector<VkDescriptorPoolSize> m_pool_sizes;

    std::vector<VkDescriptorSet> m_desc_sets;
    std::vector<VkDescriptorSet> m_stale_desc_sets;
};