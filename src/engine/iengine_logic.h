#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../actors/actor.h"
#include "iengine_physics.h"
#include "base_engine_state.h"

class IEngineLogic {
public:
	virtual WeakActorPtr VGetActor(const ActorId id) = 0;
	virtual StrongActorPtr VCreateActor(const pugi::xml_node& actor_data, const ActorId servers_actorId = INVALID_ACTOR_ID) = 0;
	virtual void VDestroyActor(const ActorId actorId) = 0;

	virtual const std::shared_ptr<Scene>& VGetScene() = 0;

	virtual bool VLoadGame(const std::string& level_resource) = 0;

	virtual void VOnUpdate(const GameTimerDelta& delta) = 0;
	virtual void VChangeState(const BaseEngineState new_state) = 0;

	virtual void VMoveActor(const ActorId id, const glm::mat4x4& mat) = 0;

	virtual std::shared_ptr<IEnginePhysics> VGetGamePhysics() = 0;
};