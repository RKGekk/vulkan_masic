#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class DeviceAllocation {
public:
	VkDeviceMemory get_memory() const;
	uint32_t get_offset() const;
	uint32_t get_size() const;

private:
	VkDeviceMemory m_base = VK_NULL_HANDLE;
	uint8_t *m_host_base = nullptr;
	uint32_t m_offset = 0;
	uint32_t m_size = 0;
};