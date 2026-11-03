#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <mutex>

class VulkanDevice;

class VulkanFence {
public:
	bool init(std::shared_ptr<VulkanDevice> device, bool signaled, std::string name);
	void destroy();

	void wait() const;
	void reset() const;
    VkFence getFence() const;
	const VkFence* getFencePtr() const;

private:
	std::shared_ptr<VulkanDevice> m_device;
	std::string m_name;
	VkFence m_fence;
};