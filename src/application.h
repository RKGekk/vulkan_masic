#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <mutex>
#include <unordered_map>

#include "application_options.h"
#include "engine/base_engine_logic.h"
#include "window_surface.h"
#include "tools/game_timer.h"
#include "tools/thread_pool.h"
#include "events/ievent_manager.h"

#include "graphics/vulkan_instance.h"
#include "graphics/vulkan_device.h"
#include "graphics/vulkan_renderer.h"

class VulkanApplication;

class Application {
public:
    static Application& Get();
    static std::shared_ptr<WindowSurface> GetRenderWindow();
    static VulkanRenderer& GetRenderer();

    void run();
    bool update();
    void Quit(int exit_code = 0);

    Application();
    bool Initialize(ApplicationOptions opt);
    ~Application();

    void VRegisterEvents();
    void RegisterAllDelegates();

    std::shared_ptr<WindowSurface> CreateRenderWindow();

    void ShowWindow();

    const ApplicationOptions& GetApplicationOptions() const;
    GameTimer& GetTimer();
    const std::shared_ptr<BaseEngineLogic>& GetGameLogic() const;
    
    void mainLoop();

protected:
    void CloseWindow(IEventDataPtr pEventData);

    std::shared_ptr<WindowSurface> m_window;
    GameTimer m_timer;

    std::unique_ptr<IEventManager> m_event_manager;
    std::shared_ptr<ThreadPool> m_thread_pool;

    std::atomic_bool m_is_running;
    std::atomic_bool m_request_quit;

    ApplicationOptions m_options;

    std::shared_ptr<BaseEngineLogic> m_game;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VulkanInstance m_vulkan_instance;
    std::shared_ptr<VulkanDevice> m_vulkan_device;
    
    VulkanRenderer m_renderer;

private:
    Application(const Application& other) = delete;
    Application& operator=(const Application& other) = delete;
    Application(Application&& other) = delete;
    Application& operator=(Application&& other) = delete;
};