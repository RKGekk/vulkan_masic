#pragma once

#include <memory>
#include <queue>
#include <vector>

#include <DirectXMath.h>

#include "iengine_view.h"
#include "iscreen_element.h"
#include "base_engine_state.h"
#include "../actors/camera_component.h"
#include "screen_element_scene.h"
#include "../procs/process_manager.h"

#include <pugixml.hpp>

class HumanView : public IEngineView {
	friend class Application;
	friend class Engine;

	static const std::string g_name;

public:
	HumanView() {};
	virtual ~HumanView() {};

	bool LoadGame(const pugi::xml_node& pLevelData) { return true; };

	virtual bool VOnRestore() override {return true;};
	virtual bool VOnLostDevice() override {return true;};

	virtual void VOnUpdate(const GameTimerDelta& delta) override {};

	virtual EngineViewType VGetType() override { return EngineViewType::GameView_Human; };
	virtual EngineViewId VGetId() const override { return 0; };

	virtual void VOnAttach(EngineViewId vid, ActorId aid) override {};
	
	virtual void VPushElement(std::shared_ptr<IScreenElement> pElement) {};
	virtual void VRemoveElement(std::shared_ptr<IScreenElement> pElement) {};

	virtual void VActivateScene(bool is_active) {};
	virtual void VCanDraw(bool is_can_draw) {};

	void TogglePause(bool active) {};
	void HandleGameState(BaseEngineState newState) {};

	virtual void VSetControlledActor(std::shared_ptr<Actor> actor) {};
	virtual std::shared_ptr<CameraComponent> VGetCamera() { return nullptr; };
	virtual std::shared_ptr<Scene> VGetScene() { return nullptr; };
	virtual void VSetCameraByName(std::string camera_name) {};

	virtual const std::string& VGetName() override { return m_gameplay_text; };

	void GameStateDelegate(IEventDataPtr pEventData) {};
	void NewSceneNodeComponentDelegate(IEventDataPtr pEventData) {};
	void DestroySceneNodeComponentDelegate(IEventDataPtr pEventData) {};

protected:
	virtual bool VLoadGameDelegate(const pugi::xml_node& pLevelData) { return true; };
	virtual void VRenderText() {};

	EngineViewId m_view_id;
	std::weak_ptr<Actor> m_actor;
	BaseEngineState m_base_game_state;

	GameClockDuration m_current_tick;
	GameClockDuration m_last_draw;
	bool m_run_full_speed;
	bool m_can_draw = true;

	std::shared_ptr<ProcessManager> m_process_manager;
	ScreenElementList m_screen_elements;
	std::shared_ptr<ScreenElementScene> m_scene;
	std::weak_ptr<CameraComponent> m_camera;

	float m_pointer_radius;
	//std::vector<std::shared_ptr<IPointerHandler>> m_pointer_handlers;
	//std::vector<std::shared_ptr<IKeyboardHandler>> m_keyboard_handlers;

	bool m_bShow_ui;
	bool m_bShow_debug_ui;
	std::string m_gameplay_text;
	//std::shared_ptr<ActorMenuUI> m_actor_menu_ui;
	//std::shared_ptr<NodeMenuUI> m_node_menu_ui;
	//std::shared_ptr<AnimMenuUI> m_anim_menu_ui;

	//std::shared_ptr<MovementController> m_pFree_camera_controller;
	std::weak_ptr<Actor> m_pTeapot;

	//glfw::window m_window;
	//std::shared_ptr<GUI> m_gui;

private:
	void RegisterAllDelegates() {};
	void RemoveAllDelegates() {};
};