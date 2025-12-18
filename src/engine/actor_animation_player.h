#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <pugixml.hpp>

#include "../actors/actor.h"
#include "../events/ievent_data.h"
#include "../tools/game_timer.h"

class ActorAnimationPlayer {
public:
	struct KeyframeTranslation {
		KeyframeTranslation();
		~KeyframeTranslation();

		float TimePos;
		glm::vec3 Translation;
	};
	friend bool operator<(const KeyframeTranslation& kf1, const KeyframeTranslation& kf2);

	struct KeyframeScale {
		KeyframeScale();
		~KeyframeScale();

		float TimePos;
		glm::vec3 Scale;
	};
	friend bool operator<(const KeyframeScale& kf1, const KeyframeScale& kf2);

	struct KeyframeRotation {
		KeyframeRotation();
		~KeyframeRotation();

		float TimePos;
		glm::quat RotationQuat;

		friend bool operator<(const KeyframeRotation& kf1, const KeyframeRotation& kf2);
	};
	friend bool operator<(const KeyframeRotation& kf1, const KeyframeRotation& kf2);

	struct ActorAnimation {
		void Interpolate(float t, glm::mat4x4& transform) const;

		// ASC sorted by time
		std::vector<KeyframeTranslation> TranslationKeyframes;
		std::vector<KeyframeScale> ScaleKeyframes;
		std::vector<KeyframeRotation> RotationKeyframes;
	};

	using AnimMap = std::unordered_map<StrongActorPtr, ActorAnimation>;

	enum class AnimState {
		Playing,
		Stoped
	};

	ActorAnimationPlayer();
	~ActorAnimationPlayer();

	bool Initialize(const pugi::xml_node& pLevel_data);
	void Update(const GameTimerDelta& delta);

	void Pause();
	void Stop();
	void Play();
	void SetDuration(float t);
	void SetDuration(const GameTimerDelta& duration);

	float GetTotalAnimationTime();
	float GetCurrentAnimationTime();
	const AnimMap& GetAnimMap() const;
	
private:
	void AddActorAnimation(const pugi::xml_node& pAnim_data);

	AnimMap m_actors_animation_map;
	GameTimerDelta m_time;
	AnimState m_animation_state;
	float m_total_animation_time;
};