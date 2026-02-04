#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <stdexcept>
#include <vector>

class VulkanDevice;

class VulkanRenderPass {
public:

    bool init(std::shared_ptr<VulkanDevice> device);
    void destroy();

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::string m_name;
};