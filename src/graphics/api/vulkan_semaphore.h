#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <string>

class VulkanDevice;

class VulkanSemaphore {
public:
    bool init(std::shared_ptr<VulkanDevice> device, bool signaled, std::string name);
    void destroy();
	
	void setName(std::string new_name);
	VkSemaphore getSemaphore() const;
	
private:
	std::shared_ptr<VulkanDevice> m_device;
	bool m_signaled;
	std::string m_name;
	VkSemaphore m_semaphore = VK_NULL_HANDLE;
};