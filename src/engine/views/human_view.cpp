#include "human_view.h"

#include "../../application.h"
#include "../../graphics/scene_config.h"
#include "../../graphics/vulkan_renderer.h"
#include "../../graphics/vulkan_device.h"
#include "../../tools/memory_utility.h"
#include "../../events/cicadas/evt_data_mouse_motion.h"
#include "../../events/cicadas/evt_data_mouse_button_pressed.h"
#include "../../events/cicadas/evt_data_mouse_button_released.h"
#include "../../events/cicadas/evt_data_key_pressed_event.h"
#include "../../events/cicadas/evt_data_key_released_event.h"

#include "../../events/cicadas/evt_data_mouse_wheel.h"

const std::string HumanView::g_name = "Level"s;

HumanView::HumanView(std::shared_ptr<ProcessManager> process_manager) {
	m_process_manager = std::move(process_manager);

	m_pointer_radius = 1.0f;
	m_view_id = 0xffffffff;

	VulkanRenderer& renderer = Application::GetRenderer();
	std::shared_ptr<VulkanDevice> device = renderer.GetDevice();

	m_bShow_ui = false;
	m_bShow_debug_ui = Application::Get().GetApplicationOptions().DebugUI;
	if (m_bShow_debug_ui) {
		m_test_menu_ui = std::make_shared<TestMenuUI>();
		VPushElement(m_test_menu_ui);

		m_gui = std::make_shared<ImGUIDrawable>();
    	m_gui->init(device, renderer.getRenderTarget(), renderer.getSwapchain()->getMaxFrames());
		renderer.addDrawable(m_gui);
	}

	RegisterAllDelegates();
	m_base_game_state = BaseEngineState::BGS_Initializing;

	m_scene = std::make_shared<ScreenElementScene>();
	
	m_current_tick = {};
	m_last_draw = {};

	m_pFree_camera_controller = std::make_shared<MovementController>(nullptr);
	m_controllers.push_back(m_pFree_camera_controller);
}

HumanView::~HumanView() {
	RemoveAllDelegates();
}

bool HumanView::LoadGame(const pugi::xml_node& pLevelData) {
	return VLoadGameDelegate(pLevelData);
}

bool HumanView::VOnRestore() {
	bool hr = true;
	for (ScreenElementList::iterator i = m_screen_elements.begin(); i != m_screen_elements.end(); ++i) {
		hr &= (*i)->VOnRestore();
	}

	return hr;
}

bool HumanView::VOnLostDevice() {
	bool hr = true;
	for (ScreenElementList::iterator i = m_screen_elements.begin(); i != m_screen_elements.end(); ++i) {
		hr &= (*i)->VOnLostDevice();
	}

	return hr;
}

void HumanView::VOnRender(const GameTimerDelta& delta) {
	m_current_tick = delta.GetTotalDuration();
	if (m_current_tick == m_last_draw) { return; }
	if (!m_can_draw) { return; }

	const auto one_frame_time = 0.016ms;
	if (m_run_full_speed || ((m_current_tick - m_last_draw) > one_frame_time)) {
		m_gui->beginFrame(Application::GetRenderer().getRenderTarget());
		
		m_screen_elements.sort(SortBy_SharedPtr_Content<IScreenElement>());
		for (ScreenElementList::iterator i = m_screen_elements.begin(); i != m_screen_elements.end(); ++i) {
			if ((*i)->VIsVisible()) {
				(*i)->VOnRender(delta);
			}
		}
		VRenderText();
		m_gui->endFrame();
		m_last_draw = m_current_tick;
	}
}

void HumanView::VOnUpdate(const GameTimerDelta& delta) {
	m_process_manager->UpdateProcesses(delta);
	for(auto& ctrl : m_controllers) {
		ctrl->OnUpdate(delta);
	}
	for (ScreenElementList::iterator i = m_screen_elements.begin(); i != m_screen_elements.end(); ++i) {
		(*i)->VOnUpdate(delta);
	}
	m_scene->recalculateGlobalTransforms();
}

EngineViewType HumanView::VGetType() {
	return EngineViewType::GameView_Human;
}

EngineViewId HumanView::VGetId() const {
	return m_view_id;
}

void HumanView::VOnAttach(EngineViewId vid, ActorId aid) {
	m_view_id = vid;
	if (aid != INVALID_ACTOR_ID) {
		m_actor = Application::Get().GetGameLogic()->VGetActor(aid);
	}
}

void HumanView::VPushElement(std::shared_ptr<IScreenElement> pElement) {
	m_screen_elements.push_front(pElement);
}

void HumanView::VRemoveElement(std::shared_ptr<IScreenElement> pElement) {
	m_screen_elements.remove(pElement);
}

void HumanView::VActivateScene(bool is_active) {
	
}

void HumanView::VCanDraw(bool is_can_draw) {
	m_can_draw = is_can_draw;
}

void HumanView::TogglePause(bool active) {}

void HumanView::VSetControlledActor(std::shared_ptr<Actor> actor) {
	m_actor = actor;
	if (m_actor.expired()) {
		m_keyboard_handlers.clear();
		m_pointer_handlers.clear();
		m_pFree_camera_controller->SetObject(nullptr);
		//m_keyboard_handlers.push_back(m_pFreeCameraController);
		//m_pointer_handlers.push_back(m_pFreeCameraController);
		//if (auto camera = m_camera.lock()) {
		//	camera->SetTarget(nullptr);
		//}
		return;
	}
	else {
		m_keyboard_handlers.clear();
		m_pointer_handlers.clear();
		m_pFree_camera_controller->SetObject(m_actor.lock());
		m_keyboard_handlers.push_back(m_pFree_camera_controller);
		m_pointer_handlers.push_back(m_pFree_camera_controller);

		//m_camera->SetTarget(m_pTeapot);
	}
}

std::shared_ptr<CameraComponent> HumanView::VGetCamera() {
	if (!m_camera.expired()) {
		return m_camera.lock();
	}
	return nullptr;
}

void HumanView::VSetCameraByName(std::string camera_name) {
	WeakActorPtr weak_camera_actor = Application::Get().GetGameLogic()->VGetActorByName(camera_name);
	if (auto camera_actor = weak_camera_actor.lock()) {
		auto weak_camera_component = camera_actor->GetComponent<CameraComponent>();
		if (auto camera_component = weak_camera_component.lock()) {
			m_camera = camera_component;
		}
	}
}

std::shared_ptr<Scene> HumanView::VGetScene() {
	return m_scene;
}

const std::string& HumanView::VGetName() {
	return g_name;
}

void HumanView::MouseMotionDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Mouse_Motion> pCastEventData = std::static_pointer_cast<EvtData_Mouse_Motion>(pEventData);
	ImGui::GetIO().MousePos = ImVec2(pCastEventData->GetX(), pCastEventData->GetY());
	for (auto& handler : m_pointer_handlers) {
		handler->VOnPointerMove(pCastEventData->GetX(), pCastEventData->GetY(), 1);
	}
}

void HumanView::MouseButtonPressDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Mouse_Button_Pressed> pCastEventData = std::static_pointer_cast<EvtData_Mouse_Button_Pressed>(pEventData);
	const ImGuiMouseButton_ imguiButton = (pCastEventData->GetMouseButton() == MouseButtonSide::Left)
                                              ? ImGuiMouseButton_Left
                                              : (pCastEventData->GetMouseButton() == MouseButtonSide::Right ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
	ImGuiIO& io                         = ImGui::GetIO();
    io.MousePos                         = ImVec2((float)pCastEventData->GetX(), (float)pCastEventData->GetY());
    io.MouseDown[imguiButton]           = true;

	io.KeyShift = pCastEventData->GetShift();
	io.KeyCtrl = pCastEventData->GetControl();

	for (auto& handler : m_pointer_handlers) {
		handler->VOnPointerButtonDown(pCastEventData->GetX(), pCastEventData->GetY(), 1, pCastEventData->GetMouseButton());
	}
};

void HumanView::MouseButtonReleaseDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Mouse_Button_Pressed> pCastEventData = std::static_pointer_cast<EvtData_Mouse_Button_Pressed>(pEventData);
	const ImGuiMouseButton_ imguiButton = (pCastEventData->GetMouseButton() == MouseButtonSide::Left)
                                              ? ImGuiMouseButton_Left
                                              : (pCastEventData->GetMouseButton() == MouseButtonSide::Right ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
	ImGuiIO& io                         = ImGui::GetIO();
    io.MousePos                         = ImVec2((float)pCastEventData->GetX(), (float)pCastEventData->GetY());
    io.MouseDown[imguiButton]           = false;

	io.KeyShift = pCastEventData->GetShift();
	io.KeyCtrl = pCastEventData->GetControl();

	for (auto& handler : m_pointer_handlers) {
		handler->VOnPointerButtonUp(pCastEventData->GetX(), pCastEventData->GetY(), 1, pCastEventData->GetMouseButton());
	}
};

void HumanView::KeyPressDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Key_Pressed_Event> pCastEventData = std::static_pointer_cast<EvtData_Key_Pressed_Event>(pEventData);
	for (auto& handler : m_keyboard_handlers) {
		handler->VOnKeyDown(pCastEventData->GetWindowKeyCode(), pCastEventData->GetCharCode());
	}
}

void HumanView::KeyReleaseDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Key_Released_Event> pCastEventData = std::static_pointer_cast<EvtData_Key_Released_Event>(pEventData);
	for (auto& handler : m_keyboard_handlers) {
		handler->VOnKeyUp(pCastEventData->GetWindowKeyCode(), pCastEventData->GetCharCode());
	}
}

void HumanView::MouseWheelDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Mouse_Wheel> pCastEventData = std::static_pointer_cast<EvtData_Mouse_Wheel>(pEventData);
	ImGuiIO& io                         = ImGui::GetIO();
	io.MouseWheel = pCastEventData->GetWheelDelta();
}

void HumanView::RegisterAllDelegates() {
	IEventManager* pGlobalEventManager = IEventManager::Get();
    pGlobalEventManager->VAddListener({ connect_arg<&HumanView::MouseMotionDelegate>, this }, EvtData_Mouse_Motion::sk_EventType);
	pGlobalEventManager->VAddListener({ connect_arg<&HumanView::MouseButtonPressDelegate>, this }, EvtData_Mouse_Button_Pressed::sk_EventType);
	pGlobalEventManager->VAddListener({ connect_arg<&HumanView::MouseButtonReleaseDelegate>, this }, EvtData_Mouse_Button_Released::sk_EventType);
	pGlobalEventManager->VAddListener({ connect_arg<&HumanView::KeyPressDelegate>, this }, EvtData_Key_Pressed_Event::sk_EventType);
	pGlobalEventManager->VAddListener({ connect_arg<&HumanView::KeyReleaseDelegate>, this }, EvtData_Key_Released_Event::sk_EventType);
	pGlobalEventManager->VAddListener({ connect_arg<&HumanView::MouseWheelDelegate>, this }, EvtData_Mouse_Wheel::sk_EventType);
};

void HumanView::RemoveAllDelegates() {
	IEventManager* pGlobalEventManager = IEventManager::Get();
	pGlobalEventManager->VRemoveListener({ connect_arg<&HumanView::MouseMotionDelegate>, this }, EvtData_Mouse_Motion::sk_EventType);
	pGlobalEventManager->VRemoveListener({ connect_arg<&HumanView::MouseButtonPressDelegate>, this }, EvtData_Mouse_Button_Pressed::sk_EventType);
	pGlobalEventManager->VRemoveListener({ connect_arg<&HumanView::MouseButtonReleaseDelegate>, this }, EvtData_Mouse_Button_Released::sk_EventType);
	pGlobalEventManager->VRemoveListener({ connect_arg<&HumanView::KeyPressDelegate>, this }, EvtData_Key_Pressed_Event::sk_EventType);
	pGlobalEventManager->VRemoveListener({ connect_arg<&HumanView::KeyReleaseDelegate>, this }, EvtData_Key_Released_Event::sk_EventType);
	pGlobalEventManager->VRemoveListener({ connect_arg<&HumanView::MouseWheelDelegate>, this }, EvtData_Mouse_Wheel::sk_EventType);
};

bool HumanView::VLoadGameDelegate(const pugi::xml_node& pLevel_data) {
	using namespace std::literals;
	pugi::xml_node scene_config_node = pLevel_data.child("Scene");
	if (scene_config_node) {
		for (pugi::xml_node node = scene_config_node.first_child(); node; node = node.next_sibling()) {
			std::string param_name = node.name();

			if (param_name == "Camera"s) {
				std::string camera_name = node.child("SelectName").text().as_string();
				VSetCameraByName(camera_name);
			}

			if (param_name == "BackgroundColor"s) {
				glm::vec3 default_color = { 1.0f, 1.0f, 1.0f };
				glm::vec3 bg_color = colorfromattr3f(node, default_color);
			}

			if (param_name == "Fog"s) {

				glm::vec3 default_fog_color = { 1.0f, 1.0f, 1.0f };
				glm::vec3 fog_color = colorfromattr3f(node.child("FogColor"), default_fog_color);
				float fog_range = node.child("FogRange").text().as_float();
				float fog_start = node.child("FogStart").text().as_float();
				SceneConfig sc_cfg = {};
				sc_cfg.FogColor = fog_color;
				sc_cfg.FogStart = fog_start;
				sc_cfg.FogRange = fog_range;
			}
		}
	}
	VPushElement(m_scene);

	if(std::shared_ptr<CameraComponent> cam = m_camera.lock()) {
		VSetControlledActor(cam->GetOwner());
	}

	m_scene->VOnRestore();

	return true;
}