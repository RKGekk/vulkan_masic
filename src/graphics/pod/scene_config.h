#pragma once

#include "../../tools/glm_define.h"

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct SceneConfig {
	glm::vec3 FogColor;
	float FogStart;
	float FogRange;
};