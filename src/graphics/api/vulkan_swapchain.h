#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include "../pod/render_resource.h"

class WindowSurface;
class Managers;
class FormatConfig;
class VulkanImageBuffer;
class VulkanDevice;

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
    bool is_native_swapchain_BGR;
};

class VulkanSwapChain {
public:
    static const uint32_t CURRENT_SYNC = -1;

    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<WindowSurface> window, const std::string& rg_file_path);
    void destroy();
    void recreate();

    const SwapchainSupportDetails& getSwapchainSupportDetails() const;
    const VkSwapchainCreateInfoKHR& getSwapchainParams() const;
    int getMaxFrames() const;
    
    const std::shared_ptr<WindowSurface>& getWindow() const;
    VkSwapchainKHR getSwapchain() const;

    const std::vector<std::shared_ptr<VulkanImageBuffer>>& getSwapchainImages() const;
    const std::shared_ptr<FormatConfig>& getFormatConfig() const;

    static VkSurfaceKHR createSurface(VkInstance vk_instance, GLFWwindow* glfw_window_ptr);
    static SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    static VkExtent2D chooseSwapExtent(GLFWwindow* m_window, const VkSurfaceCapabilitiesKHR& capabilities);

private:
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
    static bool isNativeSwapChainBGR(const std::vector<VkSurfaceFormatKHR>& formats);

    SwapchainSupportDetails m_swapchain_support_details;
    std::shared_ptr<FormatConfig> m_format_config;
    VkSwapchainCreateInfoKHR m_swapchain_create_info;
    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<WindowSurface> m_window = nullptr;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

    std::vector<std::shared_ptr<VulkanImageBuffer>> m_swapchain_images;
    uint32_t m_max_frames = 0u;
};