#include "window_surface.h"

#include "application.h"

#include "events/cicadas/evt_data_window_close.h"
#include "events/cicadas/evt_data_dpi_scale.h"
#include "events/cicadas/evt_data_key_pressed_event.h"
#include "events/cicadas/evt_data_key_released_event.h"
#include "events/cicadas/evt_data_maximize_window.h"
#include "events/cicadas/evt_data_minimize_window.h"
#include "events/cicadas/evt_data_mouse_button_pressed.h"
#include "events/cicadas/evt_data_mouse_button_released.h"
#include "events/cicadas/evt_data_mouse_motion.h"
#include "events/cicadas/evt_data_mouse_wheel.h"
#include "events/cicadas/evt_data_resize_window.h"
#include "events/cicadas/evt_data_restore_window.h"
#include "events/cicadas/evt_data_window_close.h"

WindowSurface::WindowPtr WindowSurface::GetWindow() const {
    return m_window;
}

float WindowSurface::GetDPIScaling() const {
    return m_dpi_scaling;
}

const std::string& WindowSurface::GetWindowName() const {
    return m_name;
}

const std::string& WindowSurface::GetWindowTitle() const {
    return m_title;
}

int WindowSurface::GetClientWidth() const {
    return m_client_width_px;
}

int WindowSurface::GetClientHeight() const {
    return m_client_height_px;
}

bool WindowSurface::IsFullscreen() const {
    return m_is_fullscreen;
}

void WindowSurface::SetFullscreen(bool fullscreen) {
}

void WindowSurface::ToggleFullscreen() {
    SetFullscreen(!m_is_fullscreen);
}

void WindowSurface::Show() {
    glfwShowWindow(m_window);
    glfwFocusWindow(m_window);
}

void WindowSurface::Hide() {
    glfwHideWindow(m_window);
}

bool WindowSurface::ProcessMessages() {
    glfwPollEvents();
    return glfwWindowShouldClose(m_window);
}

WindowSurface::WindowSurface() : m_dpi_scaling(1.0f), m_name(), m_title(), m_previous_mouse_x(0), m_previous_mouse_y(0), m_is_fullscreen(false), m_is_minimized(false), m_is_maximized(false) {}

WindowSurface::~WindowSurface() {
    glfwDestroyWindow(m_window);
}

bool WindowSurface::Initialize(const ApplicationOptions& cfg) {
    m_name = cfg.AppName;
    m_title = cfg.AppName;
    m_client_width_px = cfg.ScreenWidth;
    m_client_height_px = cfg.ScreenHeight;
    m_is_fullscreen = cfg.FullScreen;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(m_client_width_px, m_client_height_px, m_title.c_str(), nullptr, nullptr);
    if(!m_window) {
        return false;
    }

    float xscale;
    float yscale;
    glfwGetWindowContentScale(m_window, &xscale, &yscale);
    m_dpi_scaling = xscale;

    glfwSetInputMode(m_window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

    VRegisterEvents();

    return true;
}

WindowKey decodeFlfwKey(int key){
    switch (key) {
        case GLFW_KEY_SPACE: return WindowKey::Space; // 32 /*  */
        case GLFW_KEY_APOSTROPHE: return WindowKey::OemQuotes; // 39 /* ' */
        case GLFW_KEY_COMMA: return WindowKey::OemComma; // 44 /* , */
        case GLFW_KEY_MINUS: return WindowKey::OemMinus; // 45 /* - */
        case GLFW_KEY_PERIOD: return WindowKey::OemPeriod; // 46 /* . */
        case GLFW_KEY_SLASH: return WindowKey::OemSlash; // 47 /* / */
        case GLFW_KEY_0: return WindowKey::D0; // 48
        case GLFW_KEY_1: return WindowKey::D1; // 49
        case GLFW_KEY_2: return WindowKey::D2; // 50
        case GLFW_KEY_3: return WindowKey::D3; // 51
        case GLFW_KEY_4: return WindowKey::D4; // 52
        case GLFW_KEY_5: return WindowKey::D5; // 53
        case GLFW_KEY_6: return WindowKey::D6; // 54
        case GLFW_KEY_7: return WindowKey::D7; // 55
        case GLFW_KEY_8: return WindowKey::D8; // 56
        case GLFW_KEY_9: return WindowKey::D9; // 57
        case GLFW_KEY_SEMICOLON: return WindowKey::OemSemicolon; // 59 /* ; */
        case GLFW_KEY_EQUAL: return WindowKey::OemEq; // 61 /* = */
        case GLFW_KEY_A: return WindowKey::A; // 65
        case GLFW_KEY_B: return WindowKey::B; // 66
        case GLFW_KEY_C: return WindowKey::C; // 67
        case GLFW_KEY_D: return WindowKey::D; // 68
        case GLFW_KEY_E: return WindowKey::E; // 69
        case GLFW_KEY_F: return WindowKey::F; // 70
        case GLFW_KEY_G: return WindowKey::G; // 71
        case GLFW_KEY_H: return WindowKey::H; // 72
        case GLFW_KEY_I: return WindowKey::I; // 73
        case GLFW_KEY_J: return WindowKey::J; // 74
        case GLFW_KEY_K: return WindowKey::K; // 75
        case GLFW_KEY_L: return WindowKey::L; // 76
        case GLFW_KEY_M: return WindowKey::M; // 77
        case GLFW_KEY_N: return WindowKey::N; // 78
        case GLFW_KEY_O: return WindowKey::O; // 79
        case GLFW_KEY_P: return WindowKey::P; // 80
        case GLFW_KEY_Q: return WindowKey::Q; // 81
        case GLFW_KEY_R: return WindowKey::R; // 82
        case GLFW_KEY_S: return WindowKey::S; // 83
        case GLFW_KEY_T: return WindowKey::T; // 84
        case GLFW_KEY_U: return WindowKey::U; // 85
        case GLFW_KEY_V: return WindowKey::V; // 86
        case GLFW_KEY_W: return WindowKey::W; // 87
        case GLFW_KEY_X: return WindowKey::X; // 88
        case GLFW_KEY_Y: return WindowKey::Y; // 89
        case GLFW_KEY_Z: return WindowKey::Z; // 90
        case GLFW_KEY_LEFT_BRACKET: return WindowKey::OemOpenBrackets; // 91 /* [ */
        case GLFW_KEY_BACKSLASH: return WindowKey::OemBackslash; // 92 /* \ */
        case GLFW_KEY_RIGHT_BRACKET: return WindowKey::OemCloseBrackets; // 93 /* ] */
        case GLFW_KEY_GRAVE_ACCENT: return WindowKey::OemGraveAccent; // 96 /* ` */
        case GLFW_KEY_WORLD_1: return WindowKey::Oem1; // 161 /* non-US #1 */
        case GLFW_KEY_WORLD_2: return WindowKey::Oem2; // 162 /* non-US #2 */
        case GLFW_KEY_ESCAPE: return WindowKey::Escape; // 256
        case GLFW_KEY_ENTER: return WindowKey::Enter; // 257
        case GLFW_KEY_TAB: return WindowKey::Tab; // 258
        case GLFW_KEY_BACKSPACE: return WindowKey::Back; // 259
        case GLFW_KEY_INSERT: return WindowKey::Insert; // 260
        case GLFW_KEY_DELETE: return WindowKey::Delete; // 261
        case GLFW_KEY_RIGHT: return WindowKey::Right; // 262
        case GLFW_KEY_LEFT: return WindowKey::Left; // 263
        case GLFW_KEY_DOWN: return WindowKey::Down; // 264
        case GLFW_KEY_UP: return WindowKey::Up; // 265
        case GLFW_KEY_PAGE_UP: return WindowKey::PageUp; // 266
        case GLFW_KEY_PAGE_DOWN: return WindowKey::PageDown; // 267
        case GLFW_KEY_HOME: return WindowKey::Home; // 268
        case GLFW_KEY_END: return WindowKey::End; // 269
        case GLFW_KEY_CAPS_LOCK: return WindowKey::CapsLock; // 280
        case GLFW_KEY_SCROLL_LOCK: return WindowKey::ScrollLock; // 281
        case GLFW_KEY_NUM_LOCK: return WindowKey::NumLock; // 282
        case GLFW_KEY_PRINT_SCREEN: return WindowKey::PrintScreen; // 283
        case GLFW_KEY_PAUSE: return WindowKey::Pause; // 284
        case GLFW_KEY_F1: return WindowKey::F1; // 290
        case GLFW_KEY_F2: return WindowKey::F2; // 291
        case GLFW_KEY_F3: return WindowKey::F2; // 292
        case GLFW_KEY_F4: return WindowKey::F4; // 293
        case GLFW_KEY_F5: return WindowKey::F5; // 294
        case GLFW_KEY_F6: return WindowKey::F6; // 295
        case GLFW_KEY_F7: return WindowKey::F7; // 296
        case GLFW_KEY_F8: return WindowKey::F8; // 297
        case GLFW_KEY_F9: return WindowKey::F9; // 298
        case GLFW_KEY_F10: return WindowKey::F10; // 299
        case GLFW_KEY_F11: return WindowKey::F11; // 300
        case GLFW_KEY_F12: return WindowKey::F12; // 301
        case GLFW_KEY_F13: return WindowKey::F13; // 302
        case GLFW_KEY_F14: return WindowKey::F14; // 303
        case GLFW_KEY_F15: return WindowKey::F15; // 304
        case GLFW_KEY_F16: return WindowKey::F16; // 305
        case GLFW_KEY_F17: return WindowKey::F17; // 306
        case GLFW_KEY_F18: return WindowKey::F18; // 307
        case GLFW_KEY_F19: return WindowKey::F19; // 308
        case GLFW_KEY_F20: return WindowKey::F20; // 309
        case GLFW_KEY_F21: return WindowKey::F21; // 310
        case GLFW_KEY_F22: return WindowKey::F22; // 311
        case GLFW_KEY_F23: return WindowKey::F23; // 312
        case GLFW_KEY_F24: return WindowKey::F24; // 313
        case GLFW_KEY_F25: return WindowKey::F25; // 314
        case GLFW_KEY_KP_0: return WindowKey::NumPad0; // 320
        case GLFW_KEY_KP_1: return WindowKey::NumPad1; // 321
        case GLFW_KEY_KP_2: return WindowKey::NumPad2; // 322
        case GLFW_KEY_KP_3: return WindowKey::NumPad3; // 323
        case GLFW_KEY_KP_4: return WindowKey::NumPad4; // 324
        case GLFW_KEY_KP_5: return WindowKey::NumPad5; // 325
        case GLFW_KEY_KP_6: return WindowKey::NumPad6; // 326
        case GLFW_KEY_KP_7: return WindowKey::NumPad7; // 327
        case GLFW_KEY_KP_8: return WindowKey::NumPad8; // 328
        case GLFW_KEY_KP_9: return WindowKey::NumPad9; // 329
        case GLFW_KEY_KP_DECIMAL: return WindowKey::OemPeriod; // 330
        case GLFW_KEY_KP_DIVIDE: return WindowKey::Divide; // 331
        case GLFW_KEY_KP_MULTIPLY: return WindowKey::Multiply; // 332
        case GLFW_KEY_KP_SUBTRACT: return WindowKey::Subtract; // 333
        case GLFW_KEY_KP_ADD: return WindowKey::Add; // 334
        case GLFW_KEY_KP_ENTER: return WindowKey::Enter; // 335
        case GLFW_KEY_KP_EQUAL: return WindowKey::OemEq; // 336
        case GLFW_KEY_LEFT_SHIFT: return WindowKey::LShiftKey; // 340
        case GLFW_KEY_LEFT_CONTROL: return WindowKey::LControlKey; // 341
        case GLFW_KEY_LEFT_ALT: return WindowKey::LMenu; // 342
        case GLFW_KEY_LEFT_SUPER: return WindowKey::LWin; // 343
        case GLFW_KEY_RIGHT_SHIFT: return WindowKey::RShiftKey; // 344
        case GLFW_KEY_RIGHT_CONTROL: return WindowKey::RControlKey; // 345
        case GLFW_KEY_RIGHT_ALT: return WindowKey::RMenu; // 346
        case GLFW_KEY_RIGHT_SUPER: return WindowKey::RWin; // 347
        case GLFW_KEY_MENU: return WindowKey::LMenu; // 348
        default: return WindowKey::Escape;
    }
}

MouseButtonSide DecodeMouseButton(int button) {
    switch (button) {
        case GLFW_MOUSE_BUTTON_RIGHT: return MouseButtonSide::Right;
        case GLFW_MOUSE_BUTTON_LEFT: return MouseButtonSide::Left;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButtonSide::Middle;
        default: return MouseButtonSide::None;
    }
}

void WindowSurface::VRegisterEvents() {
    REGISTER_EVENT(EvtData_Window_Close);
    REGISTER_EVENT(EvtData_Resize_Window);
    REGISTER_EVENT(EvtData_Minimize_Window);
    REGISTER_EVENT(EvtData_Maximize_Window);
    REGISTER_EVENT(EvtData_Restore_Window);
    REGISTER_EVENT(EvtData_DPI_Scale);
    REGISTER_EVENT(EvtData_Key_Pressed_Event);
    REGISTER_EVENT(EvtData_Key_Released_Event);
    REGISTER_EVENT(EvtData_Mouse_Motion);
    REGISTER_EVENT(EvtData_Mouse_Button_Pressed);
    REGISTER_EVENT(EvtData_Mouse_Button_Released);
    REGISTER_EVENT(EvtData_Mouse_Wheel);

    glfwSetWindowCloseCallback(
        m_window,
        [](GLFWwindow* window) {
            Application::GetRenderWindow()->OnClose(false);
        }
    );

    glfwSetKeyCallback(
        m_window,
        [](GLFWwindow* glfw_window, int glfw_key, int glfw_scancode, int glfw_action, int glfw_mods) {
            if (glfw_action == GLFW_REPEAT) return;
            WindowKey key = decodeFlfwKey(glfw_key);
            unsigned int c = static_cast<unsigned int>(glfw_scancode);
            bool shift = glfw_mods & GLFW_MOD_SHIFT != 0;
            bool control = glfw_mods & GLFW_MOD_CONTROL != 0;
            bool alt = glfw_mods & GLFW_MOD_ALT != 0;
            if (glfw_action == GLFW_PRESS) {
                KeyEventArgs key_event_args(key, c, KeyState::Pressed, control, shift, alt);
                Application::GetRenderWindow()->OnKeyPressed(key_event_args);
            }
            else if (glfw_action == GLFW_RELEASE) {
                KeyEventArgs key_event_args(key, c, KeyState::Released, control, shift, alt);
                Application::GetRenderWindow()->OnKeyPressed(key_event_args);
            }
        }
    );

    glfwSetWindowSizeCallback(
        m_window,
        [](GLFWwindow* window, int width, int height) {
            WindowState window_state = WindowState::Restored;

            ResizeEventArgs resize_event_args(width, height, window_state);
            Application::GetRenderWindow()->OnResize(resize_event_args);
        }
    );

    glfwSetCursorPosCallback(
        m_window,
        [](GLFWwindow* window, double xpos, double ypos) {
            int lButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            int rButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
            int mButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
            int lshift_state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
            int rshift_state = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT);
            int lcontrol_state = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
            int rcontrol_state = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
            
            bool lButton = lButton_state == GLFW_PRESS;
            bool rButton = rButton_state == GLFW_PRESS;
            bool mButton = mButton_state == GLFW_PRESS;
            bool shift = lshift_state == GLFW_PRESS || rshift_state == GLFW_PRESS;
            bool control = lcontrol_state == GLFW_PRESS || rcontrol_state == GLFW_PRESS;

            int x = ((int)xpos);
            int y = ((int)ypos);

            MouseMotionEventArgs mouse_motion_event_args(lButton, mButton, rButton, control, shift, x, y, 0, 0);
            Application::GetRenderWindow()->OnMouseMoved(mouse_motion_event_args);
        }
    );

    glfwSetMouseButtonCallback(
        m_window,
        [](GLFWwindow* window, int button, int action, int mods) {
            int lButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            int rButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
            int mButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
            int lshift_state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
            int rshift_state = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT);
            int lcontrol_state = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
            int rcontrol_state = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
            
            bool lButton = lButton_state == GLFW_PRESS;
            bool rButton = rButton_state == GLFW_PRESS;
            bool mButton = mButton_state == GLFW_PRESS;
            bool shift = lshift_state == GLFW_PRESS || rshift_state == GLFW_PRESS;
            bool control = lcontrol_state == GLFW_PRESS || rcontrol_state == GLFW_PRESS;

            double xpos;
            double ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int x = ((int)xpos);
            int y = ((int)ypos);

            if(action == GLFW_PRESS) {
                MBEventArgs mouse_button_event_args(DecodeMouseButton(button), MKState::Pressed, lButton, mButton, rButton, control, shift, x, y);
                Application::GetRenderWindow()->OnMouseButtonPressed(mouse_button_event_args);
            }
            else if(action == GLFW_RELEASE) {
                MBEventArgs mouse_button_event_args(DecodeMouseButton(button), MKState::Released, lButton, mButton, rButton, control, shift, x, y);
                Application::GetRenderWindow()->OnMouseButtonPressed(mouse_button_event_args);
            }
        }
    );

    glfwSetScrollCallback(
        m_window,
        [](GLFWwindow* window, double xoffset, double yoffset) {
            int lButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            int rButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
            int mButton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
            int lshift_state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
            int rshift_state = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT);
            int lcontrol_state = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
            int rcontrol_state = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
            
            bool lButton = lButton_state == GLFW_PRESS;
            bool rButton = rButton_state == GLFW_PRESS;
            bool mButton = mButton_state == GLFW_PRESS;
            bool shift = lshift_state == GLFW_PRESS || rshift_state == GLFW_PRESS;
            bool control = lcontrol_state == GLFW_PRESS || rcontrol_state == GLFW_PRESS;

            double xpos;
            double ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int x = ((int)xpos);
            int y = ((int)ypos);
            int z = ((int)yoffset);

            MouseWheelEventArgs mouse_wheel_event_args(z, lButton, mButton, rButton, control, shift, x, y);
            Application::GetRenderWindow()->OnMouseWheel(mouse_wheel_event_args);
        }
    );
}

void WindowSurface::OnDPIScaleChanged(float dpi_scale) {
    std::shared_ptr<EvtData_DPI_Scale> pEvent(new EvtData_DPI_Scale(dpi_scale));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnClose(bool confirm_close) {
    std::shared_ptr<EvtData_Window_Close> pEvent = std::make_shared<EvtData_Window_Close>(m_name, confirm_close);
    IEventManager::Get()->VQueueEvent(pEvent);
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void WindowSurface::OnResize(ResizeEventArgs& e) {
    m_client_width_px = e.Width;
    m_client_height_px = e.Height;

    if (e.State == WindowState::Restored) {
        m_is_maximized = false;
        m_is_minimized = false;
        std::shared_ptr<EvtData_Resize_Window> pEvent(new EvtData_Resize_Window(e));
        IEventManager::Get()->VQueueEvent(pEvent);
    }
    else if (!m_is_minimized && e.State == WindowState::Minimized) {
        m_is_minimized = true;
        m_is_maximized = false;
        std::shared_ptr<EvtData_Resize_Window> pEvent(new EvtData_Resize_Window(e));
        IEventManager::Get()->VQueueEvent(pEvent);
    }
    else if (!m_is_maximized && e.State == WindowState::Maximized) {
        m_is_maximized = true;
        m_is_minimized = false;
        std::shared_ptr<EvtData_Resize_Window> pEvent(new EvtData_Resize_Window(e));
        IEventManager::Get()->VTriggerEvent(pEvent);
    }
}

void WindowSurface::OnMinimized(ResizeEventArgs& e) {
    std::shared_ptr<EvtData_Minimize_Window> pEvent(new EvtData_Minimize_Window(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnMaximized(ResizeEventArgs& e) {
    std::shared_ptr<EvtData_Maximize_Window> pEvent(new EvtData_Maximize_Window(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnRestored(ResizeEventArgs& e) {
    std::shared_ptr<EvtData_Restore_Window> pEvent(new EvtData_Restore_Window(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnKeyPressed(KeyEventArgs& e) {
    std::shared_ptr<EvtData_Key_Pressed_Event> pEvent(new EvtData_Key_Pressed_Event(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnKeyReleased(KeyEventArgs& e) {
    std::shared_ptr<EvtData_Key_Released_Event> pEvent(new EvtData_Key_Released_Event(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnMouseMoved(MouseMotionEventArgs& e) {
    e.RelX = e.X - m_previous_mouse_x;
    e.RelY = e.Y - m_previous_mouse_y;

    m_previous_mouse_x = e.X;
    m_previous_mouse_y = e.Y;

    std::shared_ptr<EvtData_Mouse_Motion> pEvent(new EvtData_Mouse_Motion(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnMouseButtonPressed(MBEventArgs& e) {
    std::shared_ptr<EvtData_Mouse_Button_Pressed> pEvent(new EvtData_Mouse_Button_Pressed(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnMouseButtonReleased(MBEventArgs& e) {
    std::shared_ptr<EvtData_Mouse_Button_Released> pEvent(new EvtData_Mouse_Button_Released(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}

void WindowSurface::OnMouseWheel(MouseWheelEventArgs& e) {
    std::shared_ptr<EvtData_Mouse_Wheel> pEvent(new EvtData_Mouse_Wheel(e));
    IEventManager::Get()->VQueueEvent(pEvent);
}