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

	RegisterAllDelegates();
	m_base_game_state = BaseEngineState::BGS_Initializing;

	m_scene = std::make_shared<ScreenElementScene>();

	if (m_bShow_debug_ui) {
		m_test_menu_ui = std::make_shared<TestMenuUI>();
		VPushElement(m_test_menu_ui);

		m_actor_menu_ui = std::make_shared<ActorMenuUI>();
		VPushElement(m_actor_menu_ui);

		m_node_menu_ui = std::make_shared<NodeMenuUI>();
		VPushElement(m_node_menu_ui);

		m_gui = std::make_shared<ImGUIDrawable>();
    	m_gui->init(device, renderer.getRenderTarget(), renderer.getSwapchain()->getMaxFrames());
		renderer.addDrawable(m_gui);
	}
	
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
	ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)pCastEventData->GetX(), (float)pCastEventData->GetY());
    io.MouseDown[imguiButton] = true;

	io.KeyShift = pCastEventData->GetShift();
	io.KeyCtrl = pCastEventData->GetControl();
	io.KeyAlt = pCastEventData->GetAlt();

	for (auto& handler : m_pointer_handlers) {
		handler->VOnPointerButtonDown(pCastEventData->GetX(), pCastEventData->GetY(), 1, pCastEventData->GetMouseButton());
	}
};

void HumanView::MouseButtonReleaseDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Mouse_Button_Pressed> pCastEventData = std::static_pointer_cast<EvtData_Mouse_Button_Pressed>(pEventData);
	const ImGuiMouseButton_ imguiButton = (pCastEventData->GetMouseButton() == MouseButtonSide::Left)
                                              ? ImGuiMouseButton_Left
                                              : (pCastEventData->GetMouseButton() == MouseButtonSide::Right ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
	ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)pCastEventData->GetX(), (float)pCastEventData->GetY());
    io.MouseDown[imguiButton] = false;

	io.KeyShift = pCastEventData->GetShift();
	io.KeyCtrl = pCastEventData->GetControl();
	io.KeyAlt = pCastEventData->GetAlt();

	for (auto& handler : m_pointer_handlers) {
		handler->VOnPointerButtonUp(pCastEventData->GetX(), pCastEventData->GetY(), 1, pCastEventData->GetMouseButton());
	}
};

ImGuiKey decodeWindowKey(WindowKey wnd_key) {
	switch (wnd_key) {
		case WindowKey::None : return ImGuiKey_None;
		case WindowKey::LButton : return ImGuiKey_MouseLeft;
		case WindowKey::RButton : return ImGuiKey_MouseRight;
		case WindowKey::Cancel : return ImGuiKey_Escape;
		case WindowKey::MButton : return ImGuiKey_MouseMiddle;
		case WindowKey::XButton1 : return ImGuiKey_GamepadStart;
		case WindowKey::XButton2 : return ImGuiKey_GamepadBack;
		case WindowKey::Back : return ImGuiKey_GamepadBack;
		case WindowKey::Tab : return ImGuiKey_Tab;
		case WindowKey::Clear : return ImGuiKey_Backspace;
		case WindowKey::Enter : return ImGuiKey_Enter;
		case WindowKey::ShiftKey : return ImGuiMod_Shift;
		case WindowKey::ControlKey : return ImGuiMod_Ctrl;
		case WindowKey::AltKey : return ImGuiMod_Alt;
		case WindowKey::Pause : return ImGuiKey_Pause;
		case WindowKey::CapsLock : return ImGuiKey_CapsLock;
		case WindowKey::KanaMode : return ImGuiKey_None;
		case WindowKey::JunjaMode : return ImGuiKey_None;
		case WindowKey::FinalMode : return ImGuiKey_None;
		case WindowKey::HanjaMode : return ImGuiKey_None;
		case WindowKey::Escape : return ImGuiKey_Escape;
		case WindowKey::IMEConvert : return ImGuiKey_None;
		case WindowKey::IMINoConvert : return ImGuiKey_None;
		case WindowKey::IMEAccept : return ImGuiKey_None;
		case WindowKey::IMIModeChange : return ImGuiKey_None;
		case WindowKey::Space : return ImGuiKey_Space;
		case WindowKey::PageUp : return ImGuiKey_PageUp;
		case WindowKey::PageDown : return ImGuiKey_PageDown;
		case WindowKey::End : return ImGuiKey_End;
		case WindowKey::Home : return ImGuiKey_Home;
		case WindowKey::Left : return ImGuiKey_LeftArrow;
		case WindowKey::Up : return ImGuiKey_UpArrow;
		case WindowKey::Right : return ImGuiKey_RightArrow;
		case WindowKey::Down : return ImGuiKey_DownArrow;
		case WindowKey::Select : return ImGuiKey_None;
		case WindowKey::Print : return ImGuiKey_PrintScreen;
		case WindowKey::Execute : return ImGuiKey_None;
		case WindowKey::PrintScreen : return ImGuiKey_PrintScreen;
		case WindowKey::Insert : return ImGuiKey_Insert;
		case WindowKey::Delete : return ImGuiKey_Delete;
		case WindowKey::Help : return ImGuiKey_None;
		case WindowKey::D0 : return ImGuiKey_0;
		case WindowKey::D1 : return ImGuiKey_1;
		case WindowKey::D2 : return ImGuiKey_2;
		case WindowKey::D3 : return ImGuiKey_3;
		case WindowKey::D4 : return ImGuiKey_4;
		case WindowKey::D5 : return ImGuiKey_5;
		case WindowKey::D6 : return ImGuiKey_6;
		case WindowKey::D7 : return ImGuiKey_7;
		case WindowKey::D8 : return ImGuiKey_8;
		case WindowKey::D9 : return ImGuiKey_9;
		case WindowKey::A : return ImGuiKey_A;
		case WindowKey::B : return ImGuiKey_B;
		case WindowKey::C : return ImGuiKey_C;
		case WindowKey::D : return ImGuiKey_D;
		case WindowKey::E : return ImGuiKey_E;
		case WindowKey::F : return ImGuiKey_F;
		case WindowKey::G : return ImGuiKey_G;
		case WindowKey::H : return ImGuiKey_H;
		case WindowKey::I : return ImGuiKey_I;
		case WindowKey::J : return ImGuiKey_J;
		case WindowKey::K : return ImGuiKey_K;
		case WindowKey::L : return ImGuiKey_L;
		case WindowKey::M : return ImGuiKey_M;
		case WindowKey::N : return ImGuiKey_N;
		case WindowKey::O : return ImGuiKey_O;
		case WindowKey::P : return ImGuiKey_P;
		case WindowKey::Q : return ImGuiKey_Q;
		case WindowKey::R : return ImGuiKey_R;
		case WindowKey::S : return ImGuiKey_S;
		case WindowKey::T : return ImGuiKey_T;
		case WindowKey::U : return ImGuiKey_U;
		case WindowKey::V : return ImGuiKey_V;
		case WindowKey::W : return ImGuiKey_W;
		case WindowKey::X : return ImGuiKey_X;
		case WindowKey::Y : return ImGuiKey_Y;
		case WindowKey::Z : return ImGuiKey_Z;
		case WindowKey::LWin : return ImGuiKey_Menu;
		case WindowKey::RWin : return ImGuiKey_Menu;
		case WindowKey::Apps : return ImGuiKey_None;
		case WindowKey::Sleep : return ImGuiKey_None;
		case WindowKey::NumPad0 : return ImGuiKey_Keypad0;
		case WindowKey::NumPad1 : return ImGuiKey_Keypad1;
		case WindowKey::NumPad2 : return ImGuiKey_Keypad2;
		case WindowKey::NumPad3 : return ImGuiKey_Keypad3;
		case WindowKey::NumPad4 : return ImGuiKey_Keypad4;
		case WindowKey::NumPad5 : return ImGuiKey_Keypad5;
		case WindowKey::NumPad6 : return ImGuiKey_Keypad6;
		case WindowKey::NumPad7 : return ImGuiKey_Keypad7;
		case WindowKey::NumPad8 : return ImGuiKey_Keypad8;
		case WindowKey::NumPad9 : return ImGuiKey_Keypad9;
		case WindowKey::Multiply : return ImGuiKey_KeypadMultiply;
		case WindowKey::Add : return ImGuiKey_KeypadAdd;
		case WindowKey::Separator : return ImGuiKey_Period;
		case WindowKey::Subtract : return ImGuiKey_Minus;
		case WindowKey::Decimal : return ImGuiKey_Comma;
		case WindowKey::Divide : return ImGuiKey_Slash;
		case WindowKey::F1 : return ImGuiKey_F1;
		case WindowKey::F2 : return ImGuiKey_F2;
		case WindowKey::F3 : return ImGuiKey_F3;
		case WindowKey::F4 : return ImGuiKey_F4;
		case WindowKey::F5 : return ImGuiKey_F5;
		case WindowKey::F6 : return ImGuiKey_F6;
		case WindowKey::F7 : return ImGuiKey_F7;
		case WindowKey::F8 : return ImGuiKey_F8;
		case WindowKey::F9 : return ImGuiKey_F9;
		case WindowKey::F10 : return ImGuiKey_F10;
		case WindowKey::F11 : return ImGuiKey_F11;
		case WindowKey::F12 : return ImGuiKey_F12;
		case WindowKey::F13 : return ImGuiKey_F13;
		case WindowKey::F14 : return ImGuiKey_F14;
		case WindowKey::F15 : return ImGuiKey_F15;
		case WindowKey::F16 : return ImGuiKey_F16;
		case WindowKey::F17 : return ImGuiKey_F17;
		case WindowKey::F18 : return ImGuiKey_F18;
		case WindowKey::F19 : return ImGuiKey_F19;
		case WindowKey::F20 : return ImGuiKey_F20;
		case WindowKey::F21 : return ImGuiKey_F21;
		case WindowKey::F22 : return ImGuiKey_F22;
		case WindowKey::F23 : return ImGuiKey_F23;
		case WindowKey::F24 : return ImGuiKey_F24;
		case WindowKey::F25 : return ImGuiKey_None;
		case WindowKey::NumLock : return ImGuiKey_NumLock;
		case WindowKey::ScrollLock : return ImGuiKey_ScrollLock;
		case WindowKey::LShiftKey : return ImGuiMod_Shift;
		case WindowKey::RShiftKey : return ImGuiMod_Shift;
		case WindowKey::LControlKey : return ImGuiMod_Ctrl;
		case WindowKey::RControlKey : return ImGuiMod_Ctrl;
		case WindowKey::LMenu : return ImGuiMod_None;
		case WindowKey::RMenu : return ImGuiMod_None;
		case WindowKey::BrowserBack : return ImGuiKey_None;
		case WindowKey::BrowserForward : return ImGuiKey_None;
		case WindowKey::BrowserRefresh : return ImGuiKey_None;
		case WindowKey::BrowserStop : return ImGuiKey_None;
		case WindowKey::BrowserSearch : return ImGuiKey_None;
		case WindowKey::BrowserFavorites : return ImGuiKey_None;
		case WindowKey::BrowserHome : return ImGuiKey_None;
		case WindowKey::VolumeMute : return ImGuiKey_None;
		case WindowKey::VolumeDown : return ImGuiKey_None;
		case WindowKey::VolumeUp : return ImGuiKey_None;
		case WindowKey::MediaNextTrack : return ImGuiKey_None;
		case WindowKey::MediaPreviousTrack : return ImGuiKey_None;
		case WindowKey::MediaStop : return ImGuiKey_None;
		case WindowKey::MediaPlayPause : return ImGuiKey_None;
		case WindowKey::LaunchMail : return ImGuiKey_None;
		case WindowKey::SelectMedia : return ImGuiKey_None;
		case WindowKey::LaunchApplication1 : return ImGuiKey_None;
		case WindowKey::LaunchApplication2 : return ImGuiKey_None;
		case WindowKey::OemSemicolon : return ImGuiKey_Semicolon;
		case WindowKey::OemPlus : return ImGuiKey_KeypadAdd;
		case WindowKey::OemComma : return ImGuiKey_Comma;
		case WindowKey::OemMinus : return ImGuiKey_KeypadSubtract;
		case WindowKey::OemPeriod : return ImGuiKey_Period;
		case WindowKey::OemSlash : return ImGuiKey_Slash;
		case WindowKey::OemTilde : return ImGuiKey_GraveAccent;
		case WindowKey::OemOpenBrackets : return ImGuiKey_LeftBracket;
		case WindowKey::OemPipe : return ImGuiKey_None;
		case WindowKey::OemCloseBrackets : return ImGuiKey_RightBracket;
		case WindowKey::OemQuotes : return ImGuiKey_None;
		case WindowKey::Oem8 : return ImGuiKey_None;
		case WindowKey::OemBackslash : return ImGuiKey_Backslash;
		case WindowKey::ProcessKey : return ImGuiKey_None;
		case WindowKey::Packet : return ImGuiKey_None;
		case WindowKey::Attn : return ImGuiKey_None;
		case WindowKey::CrSel : return ImGuiKey_None;
		case WindowKey::ExSel : return ImGuiKey_None;
		case WindowKey::EraseEof : return ImGuiKey_None;
		case WindowKey::Play : return ImGuiKey_None;
		case WindowKey::Zoom : return ImGuiKey_None;
		case WindowKey::NoName : return ImGuiKey_None;
		case WindowKey::Pa1 : return ImGuiKey_None;
		case WindowKey::OemClear : return ImGuiKey_None;
		default: return ImGuiKey_None;
	}
}

void HumanView::KeyPressDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Key_Pressed_Event> pCastEventData = std::static_pointer_cast<EvtData_Key_Pressed_Event>(pEventData);
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) {
		io.KeyShift = pCastEventData->GetShiftButtonPressed();
		io.KeyCtrl = pCastEventData->GetControlButtonPressed();
		io.KeyAlt = pCastEventData->GetAltButtonPressed();

		ImGuiKey key_code = decodeWindowKey(pCastEventData->GetWindowKeyCode());
		io.AddKeyEvent(key_code, true);

		unsigned int key = pCastEventData->GetCharCode();
		io.AddInputCharacter(key);
	}
	else {
		for (auto& handler : m_keyboard_handlers) {
			handler->VOnKeyDown(pCastEventData->GetWindowKeyCode(), pCastEventData->GetCharCode());
		}
	}
}

void HumanView::KeyReleaseDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Key_Released_Event> pCastEventData = std::static_pointer_cast<EvtData_Key_Released_Event>(pEventData);
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) {
		
		io.KeyShift = pCastEventData->GetShiftButtonPressed();
		io.KeyCtrl = pCastEventData->GetControlButtonPressed();
		io.KeyAlt = pCastEventData->GetAltButtonPressed();

		ImGuiKey key_code = decodeWindowKey(pCastEventData->GetWindowKeyCode());
		io.AddKeyEvent(key_code, false);
	}
	else {
		for (auto& handler : m_keyboard_handlers) {
			handler->VOnKeyUp(pCastEventData->GetWindowKeyCode(), pCastEventData->GetCharCode());
		}
	}
}

void HumanView::MouseWheelDelegate(IEventDataPtr pEventData) {
	std::shared_ptr<EvtData_Mouse_Wheel> pCastEventData = std::static_pointer_cast<EvtData_Mouse_Wheel>(pEventData);
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel = pCastEventData->GetWheelDelta();
	io.KeyShift = pCastEventData->GetShift();
	io.KeyCtrl = pCastEventData->GetControl();
	io.KeyAlt = pCastEventData->GetAlt();
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