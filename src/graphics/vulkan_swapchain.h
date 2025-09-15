#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_device.h"
#include "vulkan_command_manager.h"

struct SwapChainBuffer{
	VkImage image;
	VkImageView view;
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
    bool is_native_swapchain_BGR;
};

struct SwapchainParams {
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    VkSharingMode images_sharing_mode;
};

class VulkanSwapChain {
public:
    static const uint32_t CURRENT_SYNC = -1;

    bool init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window);
    void destroy();
    void recreate();

    const SwapchainSupportDetails& getSwapchainSupportDetails() const;
    const SwapchainParams& getSwapchainParams() const;
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
    VkSurfaceKHR getSurface() const;
    GLFWwindow* getWindow() const;
    VkSwapchainKHR getSwapchain() const;

    const std::vector<SwapChainBuffer>& getSwapchainImages() const;

    static VkSurfaceKHR createSurface(VkInstance vk_instance, GLFWwindow* glfw_window_ptr);
    static SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

private:
    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
    static bool isNativeSwapChainBGR(const std::vector<VkSurfaceFormatKHR>& formats);

    VkSwapchainKHR createSwapchain(VkSurfaceKHR surface, const SwapchainParams& swapchain_params, const SwapchainSupportDetails& swapchain_support_details, const QueueFamilyIndices& queue_family_indices) const;
    std::vector<VkImage> retriveSwapchainImages() const;
    std::vector<SwapChainBuffer> retriveSwapchainBuffers(VkFormat format) const;

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    SwapchainSupportDetails m_swapchain_support_details;
    SwapchainParams m_swapchain_params;
    std::shared_ptr<VulkanDevice> m_device;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    GLFWwindow* m_window = nullptr;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

    std::vector<SwapChainBuffer> m_swapchain_images;
    std::vector<VkSemaphore> m_image_available_sem; // signaled when the presentation engine is finished using the image.
    std::vector<VkFence> m_image_available_fen; // signaled when the presentation engine is finished using the image.

    uint32_t m_current_frame = 0u;
    uint32_t m_current_sync = 0u;
    uint32_t m_max_frames = 0u;
};