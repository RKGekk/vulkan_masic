#pragma once

#include "collision_primitive.h"

class CollisionBox : public CollisionPrimitive {
public:

	glm::vec3 halfSize;
};