#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

class VulkanDevice;


class VulkanFence {
public:
	bool init(std::shared_ptr<VulkanDevice> device, VkFence fence);
    bool init(std::shared_ptr<VulkanDevice> device, uint64_t value, VkSemaphore timeline_semaphore);
	void destroy();

	void wait();
	bool waitTimeout(uint64_t nsec);
    VkFence getFence() const;
    void setInternalSyncObject();

private:
	std::shared_ptr<VulkanDevice> m_device;
	VkFence m_fence;
	VkSemaphore m_timeline_semaphore;
	uint64_t m_timeline_value;
	bool m_observed_wait = false;
	std::mutex m_lock;
    bool m_internal_sync = false;
};