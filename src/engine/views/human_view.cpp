#include "human_view.h"

#include "../../application.h"
#include "../../graphics/vulkan_renderer.h"
#include "../../graphics/vulkan_device.h"
#include "../../tools/memory_utility.h"

const std::string HumanView::g_name = "Level"s;

HumanView::HumanView(std::shared_ptr<ProcessManager> process_manager) {
	m_process_manager = std::move(process_manager);

	m_pointer_radius = 1.0f;
	m_view_id = 0xffffffff;

	m_bShow_ui = false;
	m_bShow_debug_ui = Application::Get().GetApplicationOptions().DebugUI;
	if (m_bShow_debug_ui) {
		m_test_menu_ui = std::make_shared<TestMenuUI>();
		VPushElement(m_test_menu_ui);
	}

	RegisterAllDelegates();
	m_base_game_state = BaseEngineState::BGS_Initializing;
	
	m_scene = std::make_shared<ScreenElementScene>();

	m_current_tick = {};
	m_last_draw = {};

    VulkanRenderer& renderer = Application::GetRenderer();
	std::shared_ptr<VulkanDevice> device = renderer.GetDevice();
	m_gui = std::make_shared<ImGUIDrawable>();
    m_gui->init(device, renderer.getRenderTarget());
	renderer.addDrawable(m_gui);
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
	for (ScreenElementList::iterator i = m_screen_elements.begin(); i != m_screen_elements.end(); ++i) {
		(*i)->VOnUpdate(delta);
	}
	// if (m_pFree_camera_controller) {
	// 	m_pFree_camera_controller->OnUpdate(delta);
	// }
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
	if (m_scene) {}
}

void HumanView::VCanDraw(bool is_can_draw) {
	m_can_draw = is_can_draw;
}

void HumanView::TogglePause(bool active) {}

void HumanView::HandleGameState(BaseEngineState newState) {}

void HumanView::VSetControlledActor(std::shared_ptr<Actor> actor) {
	m_pTeapot = actor;
	if (m_pTeapot.expired()) {
		//m_keyboard_handlers.clear();
		//m_pointer_handlers.clear();
		//m_pFreeCameraController.reset(new MovementController(m_camera, 0, 0, false, true));
		//m_keyboard_handlers.push_back(m_pFreeCameraController);
		//m_pointer_handlers.push_back(m_pFreeCameraController);
		//if (auto camera = m_camera.lock()) {
		//	camera->SetTarget(nullptr);
		//}
		return;
	}
	else {
		//m_keyboard_handlers.clear();
		//m_pointer_handlers.clear();
		//m_pFree_camera_controller.reset();
		//m_keyboard_handlers.push_back(m_pFree_camera_controller);
		//m_pointer_handlers.push_back(m_pFree_camera_controller);

		//m_camera->SetTarget(m_pTeapot);
	}
	m_actor = actor;
}

std::shared_ptr<CameraComponent> HumanView::VGetCamera() {
	if (!m_camera.expired()) {
		return m_camera.lock();
	}
	return nullptr;
}

std::shared_ptr<Scene> HumanView::VGetScene() {
	return std::static_pointer_cast<Scene>(m_scene);
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

const std::string& HumanView::VGetName() {
	return g_name;
}