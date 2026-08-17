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

enum class KeyFrameInterpolationType {
	LINEAR,
	STEP,
	CUBICSPLINE
};

struct KeyframeMatrixTranslation {
	KeyframeMatrixTranslation();
	~KeyframeMatrixTranslation();

	float TimePos;
	KeyFrameInterpolationType InterpolationType;
	glm::vec3 Translation;
	glm::vec3 inTangent;
	glm::vec3 outTangent;
};
bool operator<(const KeyframeMatrixTranslation& kf1, const KeyframeMatrixTranslation& kf2);

struct KeyframeMatrixScale {
	KeyframeMatrixScale();
	~KeyframeMatrixScale();

	float TimePos;
	KeyFrameInterpolationType InterpolationType;
	glm::vec3 Scale;
};
bool operator<(const KeyframeMatrixScale& kf1, const KeyframeMatrixScale& kf2);

struct KeyframeMatrixRotation {
	KeyframeMatrixRotation();
	~KeyframeMatrixRotation();

	float TimePos;
	KeyFrameInterpolationType InterpolationType;
	glm::quat RotationQuat;
	glm::quat inTangent;
	glm::quat outTangent;
};
bool operator<(const KeyframeMatrixRotation& kf1, const KeyframeMatrixRotation& kf2);

struct MatrixAnimation {
	void InterpolateTime(float t, glm::mat4x4& transform, float blend_factor = 1.0f) const;
	glm::mat4x4 InterpolateTime(float t) const;
	
	void InterpolateNormValue(float v, glm::mat4x4& transform) const;
	glm::mat4x4 InterpolateNormValue(float v) const;

	float GetTotalAnimationTime() const;

	// ASC sorted by time
	std::vector<KeyframeMatrixTranslation> TranslationKeyframes;
	std::vector<KeyframeMatrixScale> ScaleKeyframes;
	std::vector<KeyframeMatrixRotation> RotationKeyframes;
};