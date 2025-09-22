#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application_options.h"
#include "tools/game_timer.h"
#include "events/ievent_manager.h"
#include "events/cicadas/resize_event_args.h"
#include "events/cicadas/key_event_args.h"
#include "events/cicadas/mouse_button_event_args.h"
#include "events/cicadas/mouse_motion_event_args.h"
#include "events/cicadas/mouse_wheel_event_args.h"

#include <memory>
#include <string>

class WindowSurface {
public:
    using WindowPtr = GLFWwindow*;
    using WindowNameMap = std::unordered_map<std::string, WindowPtr>;

    WindowPtr GetWindow() const;

    float GetDPIScaling() const;

    const std::string& GetWindowName() const;
    const std::string& GetWindowTitle() const;

    int GetClientWidth() const;
    int GetClientHeight() const;

    bool IsFullscreen() const;
    void SetFullscreen(bool fullscreen);
    void ToggleFullscreen();

    void Show();
    void Hide();
    bool ProcessMessages();
    bool SkipDraw();

    WindowSurface();
    ~WindowSurface();

    bool Initialize(const ApplicationOptions& cfg);
    void VRegisterEvents();

    WindowSurface(const WindowSurface&) = delete;
    WindowSurface& operator=(const WindowSurface& right) = delete;
    WindowSurface(WindowSurface&&) = delete;
    WindowSurface&& operator=(WindowSurface&& right) = delete;

    void OnDPIScaleChanged(float dpi_scale);

    void OnClose(bool confirm_close);
    void OnResize(ResizeEventArgs& e);
    void OnMinimized(ResizeEventArgs& e);
    void OnMaximized(ResizeEventArgs& e);
    void OnRestored(ResizeEventArgs& e);

    void OnKeyPressed(KeyEventArgs& e);
    void OnKeyReleased(KeyEventArgs& e);

    void OnMouseMoved(MouseMotionEventArgs& e);
    void OnMouseButtonPressed(MBEventArgs& e);
    void OnMouseButtonReleased(MBEventArgs& e);
    void OnMouseWheel(MouseWheelEventArgs& e);

private:
    std::string m_name;
    std::string m_title;

    uint32_t m_client_width_px;
    uint32_t m_client_height_px;

    int32_t m_previous_mouse_x;
    int32_t m_previous_mouse_y;

    float m_dpi_scaling;

    bool m_is_fullscreen;
    bool m_is_minimized;
    bool m_is_maximized;

    WindowPtr m_window;
};