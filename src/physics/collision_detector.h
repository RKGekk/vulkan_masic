#pragma once

#include "collision_sphere.h"
#include "collision_plane.h"
#include "collision_data.h"
#include "collision_box.h"
#include "intersection_tests.h"

static inline float penetrationOnAxis(const CollisionBox &one, const CollisionBox &two, const glm::vec3 &axis, const glm::vec3 &toCentre) {

	float oneProject = transformToAxis(one, axis);
	float twoProject = transformToAxis(two, axis);

	float distance = abs(glm::dot(toCentre, axis));

	return oneProject + twoProject - distance;
}

static inline bool tryAxis(const CollisionBox &one, const CollisionBox &two, glm::vec3 axis, const glm::vec3& toCentre, unsigned index, float& smallestPenetration,	unsigned &smallestCase) {
	
	
	if (glm::length(axis) < 0.0001){
		return true;
	}

	axis = glm::normalize(axis);

	float penetration = penetrationOnAxis(one, two, axis, toCentre);

	if (penetration < 0.0f) {
		return false;
	}

	if (penetration < smallestPenetration) {
		smallestPenetration = penetration;
		smallestCase = index;
	}

	return true;
}

static inline glm::vec3 contactPoint(const glm::vec3 &pOne,	const glm::vec3 &dOne, float oneSize, const glm::vec3 &pTwo, const glm::vec3 &dTwo, float twoSize, bool useOne) {

	glm::vec3 toSt;
	glm::vec3 cOne;
	glm::vec3 cTwo;

	float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
	float denom, mua, mub;

	smOne = glm::length(dOne);
	smOne *= smOne;

	smTwo = glm::length(dTwo);
	smTwo *= smTwo;
	dpOneTwo = glm::dot(dTwo, dOne);

	toSt = pOne - pTwo;
	dpStaOne = glm::dot(dOne, toSt);
	dpStaTwo = glm::dot(dTwo, toSt);

	denom = smOne * smTwo - dpOneTwo * dpOneTwo;

	if (abs(denom) < 0.0001f) {
		return useOne ? pOne : pTwo;
	}

	mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
	mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

	if (mua > oneSize || mua < -oneSize || mub > twoSize || mub < -twoSize) {
		return useOne ? pOne : pTwo;
	}
	else {

		cOne = pOne + dOne * mua;
		cTwo = pTwo + dTwo * mub;

		return cOne * 0.5f + cTwo * 0.5f;
	}
}

#define CHECK_OVERLAP(axis, index) \
    if (!tryAxis(one, two, (axis), toCentre, (index), pen, best)) return 0;

class CollisionDetector {
public:

	static unsigned sphereAndHalfSpace(const CollisionSphere &sphere, const CollisionPlane &plane, CollisionData *data);
	static unsigned sphereAndTruePlane(const CollisionSphere &sphere, const CollisionPlane &plane, CollisionData *data);
	static unsigned sphereAndSphere(const CollisionSphere &one, const CollisionSphere &two, CollisionData *data);
	static unsigned boxAndHalfSpace(const CollisionBox &box, const CollisionPlane &plane, CollisionData *data);
	static unsigned boxAndBox(const CollisionBox &one, const CollisionBox &two, CollisionData *data);
	static unsigned boxAndPoint(const CollisionBox &box, const glm::vec3 &point, CollisionData *data);
	static unsigned boxAndSphere(const CollisionBox &box, const CollisionSphere &sphere, CollisionData *data);
};