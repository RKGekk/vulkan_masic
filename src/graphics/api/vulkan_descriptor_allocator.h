#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <vector>

class DescriptorAllocatorPage;
class DescriptorAllocation;
class Device;

class DescriptorAllocator {
public:
	DescriptorAllocation Allocate(uint32_t num_descriptors = 1u);

	void ReleaseStaleDescriptors();

	DescriptorAllocator(Device& device, VkDescriptorType type, uint32_t num_descriptors_per_heap = 256u);
	virtual ~DescriptorAllocator();

private:
	using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;

	std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();

	Device& m_device;
	VkDescriptorType m_heap_type;
	uint32_t m_num_descriptors_per_heap;

	DescriptorHeapPool m_heap_pool;
	std::unordered_set<size_t> m_available_heaps;

	std::mutex m_allocation_mutex;
};