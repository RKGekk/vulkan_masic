#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application_options.h"
#include "application.h"
#include "graphics/vulkan_instance.h"
#include "graphics/vulkan_device.h"
#include "graphics/vulkan_renderer.h"

#include <memory>

class VulkanApplication : public Application {
public:
    VulkanApplication();
    ~VulkanApplication() override;

    void VRegisterEvents() override;
    void RegisterAllDelegates() override;

    bool initGraphics(WindowSurface::WindowPtr window) override;
    
    void update_frame(uint32_t current_image);
    void mainLoop() override;

protected:
    void CloseWindow(IEventDataPtr pEventData);

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VulkanInstance m_vulkan_instance;
    std::shared_ptr<VulkanDevice> m_vulkan_device;
    
    VulkanRenderer m_renderer;

private:
    VulkanApplication(const VulkanApplication& other) = delete;
    VulkanApplication& operator=(const VulkanApplication& other) = delete;
    VulkanApplication(VulkanApplication&& other) = delete;
    VulkanApplication& operator=(VulkanApplication&& other) = delete;
};