#include "intersection_tests.h"

bool IntersectionTests::sphereAndHalfSpace(const CollisionSphere &sphere, const CollisionPlane &plane) {

	float ballDistance = glm::dot(plane.direction, sphere.getAxis(3)) - sphere.radius;

	return ballDistance <= plane.offset;
}

bool IntersectionTests::sphereAndSphere(const CollisionSphere &one,	const CollisionSphere &two) {
	
	glm::vec3 midline = one.getAxis(3) - two.getAxis(3);

	return glm::length(midline) < (one.radius + two.radius);
}

bool IntersectionTests::boxAndBox(const CollisionBox &one, const CollisionBox &two) {
	
	glm::vec3 toCentre = two.getAxis(3) - one.getAxis(3);

	return (
		TEST_OVERLAP(one.getAxis(0)) &&
		TEST_OVERLAP(one.getAxis(1)) &&
		TEST_OVERLAP(one.getAxis(2)) &&

		TEST_OVERLAP(two.getAxis(0)) &&
		TEST_OVERLAP(two.getAxis(1)) &&
		TEST_OVERLAP(two.getAxis(2)) &&

		TEST_OVERLAP(one.getAxis(0) * two.getAxis(0)) &&
		TEST_OVERLAP(one.getAxis(0) * two.getAxis(1)) &&
		TEST_OVERLAP(one.getAxis(0) * two.getAxis(2)) &&
		TEST_OVERLAP(one.getAxis(1) * two.getAxis(0)) &&
		TEST_OVERLAP(one.getAxis(1) * two.getAxis(1)) &&
		TEST_OVERLAP(one.getAxis(1) * two.getAxis(2)) &&
		TEST_OVERLAP(one.getAxis(2) * two.getAxis(0)) &&
		TEST_OVERLAP(one.getAxis(2) * two.getAxis(1)) &&
		TEST_OVERLAP(one.getAxis(2) * two.getAxis(2))
	);
}
#undef TEST_OVERLAP

bool IntersectionTests::boxAndHalfSpace(const CollisionBox &box, const CollisionPlane &plane) {

	float projectedRadius = transformToAxis(box, plane.direction);
	float boxDistance = glm::dot(plane.direction, box.getAxis(3)) - projectedRadius;

	return boxDistance <= plane.offset;
}