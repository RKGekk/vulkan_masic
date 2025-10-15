#include "application.h"

#include "events/cicadas/evt_data_update_tick.h"
#include "events/cicadas/evt_data_window_close.h"
#include "events/event_manager.h"
#include "vulkan_application.h"

std::unique_ptr<Application> gs_pSingeton;
std::once_flag gs_only_once;

static WindowSurface::WindowNameMap gs_window_by_name;

Application& Application::Get() {
    std::call_once(gs_only_once, [](){gs_pSingeton.reset(new VulkanApplication()); });
    return *gs_pSingeton.get();
}

std::shared_ptr<WindowSurface> Application::GetRenderWindow() {
    return Application::Get().m_window;
}

void Application::run() {
    if (!glfwInit()) return;
    m_is_running = true;
    mainLoop();
}

bool Application::update() {
    m_timer.Tick();
    std::shared_ptr<EvtData_Update_Tick> pEvent = std::make_shared<EvtData_Update_Tick>(m_timer.GetDeltaDuration(), m_timer.GetTotalDuration());
    m_event_manager->VQueueEvent(pEvent);
    bool window_close_evt = m_window->ProcessMessages();
    m_event_manager->VUpdate();

    if (m_game) {
		m_game->VOnUpdate(m_timer);
	}

    return window_close_evt || m_request_quit;
}

void Application::Quit(int exitCode) {
    m_request_quit = true;
    m_window->OnClose(false);
}

Application::Application() : m_is_running(false), m_request_quit(false), m_thread_pool(std::make_shared<ThreadPool>()) {}

Application::~Application() {
    glfwTerminate();
}

void Application::VRegisterEvents() {
    REGISTER_EVENT(EvtData_Update_Tick);
}

void Application::RegisterAllDelegates() {
    IEventManager* pGlobalEventManager = IEventManager::Get();
    pGlobalEventManager->VAddListener({ connect_arg<&Application::CloseWindow>, this }, EvtData_Window_Close::sk_EventType);
}

bool Application::Initialize(ApplicationOptions opt) {
    m_timer.Start();
    m_options = std::move(opt);
    m_event_manager = std::make_unique<EventManager>("MasicApp Event Mgr", true);

    if (!glfwInit()) return false;

    m_window = CreateRenderWindow();
    if (!m_window) {
        glfwTerminate();
        return false;
    }
    
    initGraphics(m_window->GetWindow());

    VRegisterEvents();
    RegisterAllDelegates();

    m_game = VCreateGameAndView();
	if (!m_game) return false;

    return true;
}

std::shared_ptr<WindowSurface> Application::CreateRenderWindow() {
    std::shared_ptr<WindowSurface> pWindow = std::make_shared<WindowSurface>();
    if(!pWindow->Initialize(m_options)) return nullptr;
    gs_window_by_name[pWindow->GetWindowTitle()] = pWindow->GetWindow();
    return pWindow;
}

void Application::ShowWindow() {
	return m_window->Show();
}

const ApplicationOptions& Application::GetApplicationOptions() const {
    return m_options;
}

GameTimer& Application::GetTimer() {
    return m_timer;
}

std::shared_ptr<BaseEngineLogic> GetGameLogic() {
    return nullptr;
};

std::shared_ptr<BaseEngineLogic> VCreateGameAndView() {
    std::shared_ptr pGame = std::make_shared<BaseEngineLogic>();
	pGame->Init();
	
	return pGame;
}

void Application::CloseWindow(IEventDataPtr pEventData) {
    std::shared_ptr<EvtData_Window_Close> pCastEventData = std::static_pointer_cast<EvtData_Window_Close>(pEventData);
}