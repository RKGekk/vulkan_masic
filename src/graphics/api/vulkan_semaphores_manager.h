#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <queue>
#include <vector>

class VulkanDevice;
class VulkanSemaphore;

class VulkanSemaphoresManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device);
    void destroy();

    std::shared_ptr<VulkanSemaphore> getSemaphore(std::string name);
    void returnSemaphore(std::shared_ptr<VulkanSemaphore> sem);

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::deque<std::shared_ptr<VulkanSemaphore>> m_semaphores;
};