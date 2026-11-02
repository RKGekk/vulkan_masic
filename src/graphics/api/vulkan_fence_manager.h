#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

class VulkanDevice;
class VulkanFence;

class VulkanFenceManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device);
    void destroy();

    std::shared_ptr<VulkanFence> getFence(const std::string& new_name);

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::unordered_map<std::string, std::shared_ptr<VulkanFence>> m_fences;
};