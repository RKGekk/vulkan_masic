#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../pod/descriptor_set_layout.h"

class DescriptorAllocatorPage;
class VulkanDevice;

class DescriptorAllocator {
public:
    bool init(std::shared_ptr<VulkanDevice> device, std::string name, std::vector<std::shared_ptr<DescSetLayout>> layouts, VkDescriptorPoolCreateFlags flags, uint32_t num_descriptors_per_heap = 256u);
    void destroy();

	VkDescriptorSet Allocate(const std::string& desc_layout_name);
    std::vector<VkDescriptorSet> Allocate(const std::string& desc_layout_name, uint32_t num_descriptors);
	void ReleaseStaleDescriptors();

    const std::string& getName() const;

private:
    using HeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;

	std::shared_ptr<VulkanDevice> m_device;
    std::string m_name;

	uint32_t m_num_descriptors_per_heap;
	std::unordered_map<std::string, HeapPool> m_heap_pool;
};
