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

class VulkanApplication;

class Application {
public:
    static Application& Get();
    static std::shared_ptr<WindowSurface> GetRenderWindow();

    virtual void run();
    bool update();
    void Quit(int exit_code = 0);

    Application();
    virtual bool Initialize(ApplicationOptions opt);
    virtual ~Application();

    virtual void VRegisterEvents();
    virtual void RegisterAllDelegates();

    virtual std::shared_ptr<WindowSurface> CreateRenderWindow();
    virtual bool initGraphics(WindowSurface::WindowPtr window) = 0;

    void ShowWindow();

    const ApplicationOptions& GetApplicationOptions() const;
    GameTimer& GetTimer();
    std::shared_ptr<BaseEngineLogic> GetGameLogic();;
    
    virtual void mainLoop() = 0;

protected:
    virtual std::shared_ptr<BaseEngineLogic> VCreateGameAndView();
    void CloseWindow(IEventDataPtr pEventData);

    std::shared_ptr<WindowSurface> m_window;
    GameTimer m_timer;

    std::unique_ptr<IEventManager> m_event_manager;
    std::shared_ptr<ThreadPool> m_thread_pool;

    std::atomic_bool m_is_running;
    std::atomic_bool m_request_quit;

    ApplicationOptions m_options;

    std::shared_ptr<BaseEngineLogic> m_game;

private:
    Application(const Application& other) = delete;
    Application& operator=(const Application& other) = delete;
    Application(Application&& other) = delete;
    Application& operator=(Application&& other) = delete;
};