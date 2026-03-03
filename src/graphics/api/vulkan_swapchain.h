#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include "vulkan_device.h"
#include "vulkan_image_buffer.h"
#include "../pod/render_resource.h"

class WindowSurface;
class Managers;
class FormatConfig;

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
    bool is_native_swapchain_BGR;
};

class VulkanSwapChain {
public:
    static const uint32_t CURRENT_SYNC = -1;

    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<WindowSurface> window, std::shared_ptr<Managers> managers, const std::string& rg_file_path);
    void destroy();
    void recreate();

    const SwapchainSupportDetails& getSwapchainSupportDetails() const;
    const VkSwapchainCreateInfoKHR& getSwapchainParams() const;
    int getMaxFrames() const;
    uint32_t getCurrentFrame() const;
    const uint32_t* getCurrentFramePtr() const;
    uint32_t getCurrentSync() const;
    uint32_t fetchNextSync() const;
    bool setNextFrame(VkFence wait_to);
    VkSemaphore getImageAvailableSemaphore(uint32_t image_index = CURRENT_SYNC); // signaled when the presentation engine is finished using the image.
    VkSemaphore* getImageAvailableSemaphorePtr(uint32_t image_index = CURRENT_SYNC); // signaled when the presentation engine is finished using the image.
    VkFence getImageAvailableFence(uint32_t image_index = CURRENT_SYNC); // signaled when the presentation engine is finished using the image.
    VkFence* getImageAvailableFencePtr(uint32_t image_index = CURRENT_SYNC); // signaled when the presentation engine is finished using the image.
    const std::shared_ptr<WindowSurface>& getWindow() const;
    VkSwapchainKHR getSwapchain() const;

    const std::vector<std::shared_ptr<VulkanImageBuffer>>& getSwapchainImages() const;

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
    std::vector<VkSemaphore> m_image_available_sem; // signaled when the presentation engine is finished using the image.
    std::vector<VkFence> m_image_available_fen; // signaled when the presentation engine is finished using the image.

    uint32_t m_current_frame = 0u;
    uint32_t m_current_sync = 0u;
    uint32_t m_max_frames = 0u;
};