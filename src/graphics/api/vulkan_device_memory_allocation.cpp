#include "vulkan_device_memory_allocation.h"

VkDeviceMemory DeviceAllocation::get_memory() const {
	return m_base;
}

uint32_t DeviceAllocation::get_offset() const {
	return m_offset;
}

uint32_t DeviceAllocation::get_size() const {
	return m_size;
}