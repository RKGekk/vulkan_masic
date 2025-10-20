#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "actor_animation_player.h"
#include "base_engine_state.h"
#include "iengine_logic.h"
#include "iengine_view.h"
#include "level_manager.h"
#include "../procs/process.h"
#include "../procs/process_manager.h"
#include "../scene/scene.h"
#include "../scene/camera_node.h"
#include "../tools/mt_random.h"
#include "human_view.h"

class ActorFactory;
class LevelManager;
class CameraComponent;

class BaseEngineLogic : IEngineLogic {
	friend class Engine;

public:
	BaseEngineLogic();
	virtual ~BaseEngineLogic();
	bool Init();

	ActorId GetNewActorID();

	MTRandom& GetRNG();

	virtual void VAddView(std::shared_ptr<IEngineView> pView, ActorId actorId = INVALID_ACTOR_ID);
	virtual void VRemoveView(std::shared_ptr<IEngineView> pView);

	virtual StrongActorPtr VCreateActor(const std::string& actor_resource, const pugi::xml_node& overrides, const ActorId servers_actorId = INVALID_ACTOR_ID) override;
	virtual void VDestroyActor(const ActorId actorId) override;

	virtual WeakActorPtr VGetActor(const ActorId actorId) override;
	virtual WeakActorPtr VGetActorByName(const std::string& actor_name);
	virtual const std::unordered_set<ActorId>& VGetActorsByComponent(ComponentId cid);
	virtual bool VCheckActorsExistByComponent(ComponentId cid);

	virtual void VModifyActor(const ActorId actorId, const pugi::xml_node& overrides);
	virtual void VMoveActor(const ActorId id, const glm::mat4x4& mat) override;

	std::string GetActorXml(const ActorId id);

	std::shared_ptr<CameraComponent> GetActiveCamera();

	std::shared_ptr<ActorAnimationPlayer> GetAnimationPlayer();

	const LevelManager& GetLevelManager();
	virtual std::shared_ptr<IEnginePhysics> VGetGamePhysics() override;
	virtual bool VLoadGame(const std::string& level_resource) override;
	virtual bool VLoadGame(const std::string& level_resource, std::shared_ptr<HumanView> pHuman_view);
	virtual void VOnUpdate(const GameTimerDelta& delta) override;
	virtual void VChangeState(BaseEngineState new_state) override;
	const BaseEngineState GetState() const;

	std::shared_ptr<HumanView> GetHumanView();
	std::shared_ptr<HumanView> GetHumanViewByName(std::string name);
	const GameViewList& GetViews();

	void AttachProcess(StrongProcessPtr pProcess);

	void RequestDestroyActorDelegate(IEventDataPtr pEventData);
	void MoveActorDelegate(IEventDataPtr pEventData);
	void RequestNewActorDelegate(IEventDataPtr pEventData);
	void RequestStartGameDelegate(IEventDataPtr pEventData);
	void EnvironmentLoadedDelegate(IEventDataPtr pEventData);
	void SphereParticleContactDelegate(IEventDataPtr pEventData);

protected:
	virtual std::unique_ptr<ActorFactory> VCreateActorFactory();
	virtual bool VLoadGameDelegate(const pugi::xml_node& pLevelData);

	void RegisterAllDelegates();
	virtual void VRegisterEvents();
	void RemoveAllDelegates();

	using ActorMap = std::unordered_map<ActorId, StrongActorPtr>;
	using ComponentsMap = std::unordered_map<ComponentId, std::unordered_set<ActorId>>;
	using ActorNamesMap = std::unordered_map<std::string, StrongActorPtr>;

	GameClockDuration m_life_time;

	GameViewList m_game_views;
	std::unique_ptr<ProcessManager> m_process_manager;
	std::unique_ptr<ActorFactory> m_actor_factory;
	std::shared_ptr<IEnginePhysics> m_physics;
	std::unique_ptr<LevelManager> m_level_manager;
	std::shared_ptr<ActorAnimationPlayer> m_animation_player;
	MTRandom m_random;
	ActorMap m_actors;
	ComponentsMap m_components;
	ActorNamesMap m_actors_names;
	ActorId m_last_actor_id;
	BaseEngineState m_state;
	std::shared_ptr<CameraComponent> m_active_camera;
};