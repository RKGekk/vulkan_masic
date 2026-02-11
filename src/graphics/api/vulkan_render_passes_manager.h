#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_pipeline.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "vulkan_render_pass.h"

class VulkanDevice;
class VulkanSwapChain;

class VulkanRenderPassesManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path, const std::shared_ptr<VulkanSwapChain>& swapchain);
    void destroy();

    std::shared_ptr<VulkanRenderPass> getRenderPass(std::string pass_name);

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::unordered_map<std::string, std::shared_ptr<VulkanRenderPass>> m_render_pass_name_map;
};