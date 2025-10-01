#pragma once

#include "rigid_body.h"

class IntersectionTests;
class CollisionDetector;

class CollisionPrimitive {
public:

	friend class IntersectionTests;
	friend class CollisionDetector;

	RigidBody *body;

	glm::mat4 offset;

	void calculateInternals();

	glm::vec3 getAxis(unsigned index) const {
		return glm::vec3(transform[0][index], transform[1][index], transform[2][index]);
	}

	const glm::mat4& getTransform() const {
		return transform;
	}

protected:
	glm::mat4 transform;
};