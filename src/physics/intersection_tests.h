#pragma once

#include "collision_sphere.h"
#include "collision_plane.h"
#include "collision_box.h"

static inline float transformToAxis(const CollisionBox &box, const glm::vec3 &axis) {
	return
		box.halfSize.x * abs(glm::dot(axis, box.getAxis(0))) +
		box.halfSize.y * abs(glm::dot(axis, box.getAxis(1))) +
		box.halfSize.z * abs(glm::dot(axis, box.getAxis(2)));
}

static inline bool overlapOnAxis(const CollisionBox &one, const CollisionBox &two, const glm::vec3 &axis, const glm::vec3 &toCentre) {

	float oneProject = transformToAxis(one, axis);
	float twoProject = transformToAxis(two, axis);

	float distance = abs(glm::dot(toCentre, axis));

	return (distance < oneProject + twoProject);
}

#define TEST_OVERLAP(axis) overlapOnAxis(one, two, (axis), toCentre)

class IntersectionTests {
public:

	static bool sphereAndHalfSpace(const CollisionSphere &sphere, const CollisionPlane &plane);
	static bool sphereAndSphere(const CollisionSphere &one, const CollisionSphere &two);
	static bool boxAndBox(const CollisionBox &one, const CollisionBox &two);
	static bool boxAndHalfSpace(const CollisionBox &box, const CollisionPlane &plane);
};