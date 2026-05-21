#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../tools/game_timer.h"

struct KeyframeMatrixTranslation {
	KeyframeMatrixTranslation();
	~KeyframeMatrixTranslation();

	float TimePos;
	glm::vec3 Translation;
	glm::vec3 Tangent;
};
bool operator<(const KeyframeMatrixTranslation& kf1, const KeyframeMatrixTranslation& kf2);

struct KeyframeMatrixScale {
	KeyframeMatrixScale();
	~KeyframeMatrixScale();

	float TimePos;
	glm::vec3 Scale;
};
bool operator<(const KeyframeMatrixScale& kf1, const KeyframeMatrixScale& kf2);

struct KeyframeMatrixRotation {
	KeyframeMatrixRotation();
	~KeyframeMatrixRotation();

	float TimePos;
	glm::quat RotationQuat;
};
bool operator<(const KeyframeMatrixRotation& kf1, const KeyframeMatrixRotation& kf2);

struct MatrixAnimation {
	using AnimationName = std::string;
	enum class AnimState {
		Playing,
		Stoped,
        Paused
	};

	void InterpolateTime(float t, glm::mat4x4& transform) const;
	void InterpolateCurrentTime(glm::mat4x4& transform) const;
	void InterpolateNormValue(float v, glm::mat4x4& transform) const;
	float GetTotalAnimationTime() const;

	// ASC sorted by time
	std::vector<KeyframeMatrixTranslation> TranslationKeyframes;
	std::vector<KeyframeMatrixScale> ScaleKeyframes;
	std::vector<KeyframeMatrixRotation> RotationKeyframes;

	AnimationName Name;
	AnimState AnimationState;
	GameTimerDelta CurrentTime;
};