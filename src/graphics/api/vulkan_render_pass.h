#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <stdexcept>
#include <vector>

class VulkanDevice;
class RenderPassConfig;

class VulkanRenderPass {
public:

    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<RenderPassConfig> render_pass_cfg);
    void destroy();

    VkRenderPass getRenderPass() const;
    const std::shared_ptr<RenderPassConfig>& getRenderPassConfig() const;

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::string m_name;
    std::shared_ptr<RenderPassConfig> m_render_pass_cfg;
    VkRenderPass m_render_pass = VK_NULL_HANDLE;
};