#pragma once

#include <math.h>
#include <limits>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class CollisionPlane {
public:

	glm::vec3 direction;
	float offset;
};