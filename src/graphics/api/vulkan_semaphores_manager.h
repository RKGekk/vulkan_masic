#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <vector>

class VulkanDevice;

class VulkanSemaphoresManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device);
    void destroy();

    VkSemaphore getSemaphore();
    void returnSemaphore(VkSemaphore sem);

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::vector<VkSemaphore> m_semaphores;
};