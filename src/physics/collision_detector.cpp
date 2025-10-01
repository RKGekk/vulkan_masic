#include "collision_detector.h"

void fillPointFaceBoxBox(const CollisionBox &one, const CollisionBox &two, const glm::vec3 &toCentre, CollisionData *data, unsigned best, float pen) {
	
	Contact* contact = data->contacts;

	glm::vec3 normal = one.getAxis(best);
	if (glm::dot(one.getAxis(best), toCentre) > 0.0f) {
		normal = normal * -1.0f;
	}

	glm::vec3 vertex = two.halfSize;
	if (glm::dot(two.getAxis(0), normal) < 0.0f) vertex.x = -vertex.x;
	if (glm::dot(two.getAxis(1), normal) < 0.0f) vertex.y = -vertex.y;
	if (glm::dot(two.getAxis(2), normal) < 0.0f) vertex.z = -vertex.z;

	contact->contactNormal = normal;
	contact->penetration = pen;
	contact->contactPoint = glm::vec4(vertex, 1.0f) * two.getTransform();
	contact->setBodyData(one.body, two.body, data->friction, data->restitution);
}

unsigned CollisionDetector::sphereAndTruePlane(const CollisionSphere &sphere, const CollisionPlane &plane, CollisionData *data) {

	if (data->contactsLeft <= 0) return 0;

	glm::vec3 position = sphere.getAxis(3);

	float centreDistance = glm::dot(plane.direction, position) - plane.offset;

	if (centreDistance*centreDistance > sphere.radius*sphere.radius) {
		return 0;
	}

	glm::vec3 normal = plane.direction;
	float penetration = -centreDistance;
	if (centreDistance < 0) {
		normal *= -1;
		penetration = -penetration;
	}
	penetration += sphere.radius;

	Contact* contact = data->contacts;
	contact->contactNormal = normal;
	contact->penetration = penetration;
	contact->contactPoint = position - plane.direction * centreDistance;
	contact->setBodyData(sphere.body, NULL, data->friction, data->restitution);

	data->addContacts(1);
	return 1;
}

unsigned CollisionDetector::sphereAndHalfSpace(const CollisionSphere &sphere, const CollisionPlane &plane, CollisionData *data) {

	if (data->contactsLeft <= 0) return 0;

	glm::vec3 position = sphere.getAxis(3);

	float ballDistance = glm::dot(plane.direction, position) - sphere.radius - plane.offset;

	if (ballDistance >= 0) {
		return 0;
	}

	Contact* contact = data->contacts;
	contact->contactNormal = plane.direction;
	contact->penetration = -ballDistance;
	contact->contactPoint = position - plane.direction * (ballDistance + sphere.radius);
	contact->setBodyData(sphere.body, NULL, data->friction, data->restitution);

	data->addContacts(1);
	return 1;
}

unsigned CollisionDetector::sphereAndSphere(const CollisionSphere &one, const CollisionSphere &two, CollisionData *data) {
	
	if (data->contactsLeft <= 0) return 0;

	glm::vec3 positionOne = one.getAxis(3);
	glm::vec3 positionTwo = two.getAxis(3);

	glm::vec3 midline = positionOne - positionTwo;
	float size = glm::length(midline);

	if (size <= 0.0f || size >= one.radius + two.radius) {
		return 0;
	}

	glm::vec3 normal = midline * (1.0f / size);

	Contact* contact = data->contacts;
	contact->contactNormal = normal;
	contact->contactPoint = positionOne + midline * 0.5f;
	contact->penetration = (one.radius + two.radius - size);
	contact->setBodyData(one.body, two.body, data->friction, data->restitution);

	data->addContacts(1);
	return 1;
}

unsigned CollisionDetector::boxAndBox(const CollisionBox &one, const CollisionBox &two, CollisionData *data) {

	glm::vec3 toCentre = two.getAxis(3) - one.getAxis(3);

	float pen = std::numeric_limits<float>::max();
	unsigned best = 0xffffff;

	CHECK_OVERLAP(one.getAxis(0), 0);
	CHECK_OVERLAP(one.getAxis(1), 1);
	CHECK_OVERLAP(one.getAxis(2), 2);

	CHECK_OVERLAP(two.getAxis(0), 3);
	CHECK_OVERLAP(two.getAxis(1), 4);
	CHECK_OVERLAP(two.getAxis(2), 5);

	unsigned bestSingleAxis = best;

	CHECK_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(0)), 6);
	CHECK_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(1)), 7);
	CHECK_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(2)), 8);
	CHECK_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(0)), 9);
	CHECK_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(1)), 10);
	CHECK_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(2)), 11);
	CHECK_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(0)), 12);
	CHECK_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(1)), 13);
	CHECK_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(2)), 14);

	assert(best != 0xffffff);

	if (best < 3) {

		fillPointFaceBoxBox(one, two, toCentre, data, best, pen);
		data->addContacts(1);
		return 1;
	}
	else if (best < 6) {

		fillPointFaceBoxBox(two, one, toCentre*-1.0f, data, best - 3, pen);
		data->addContacts(1);
		return 1;
	}
	else {

		best -= 6;
		unsigned oneAxisIndex = best / 3;
		unsigned twoAxisIndex = best % 3;
		glm::vec3 oneAxis = one.getAxis(oneAxisIndex);
		glm::vec3 twoAxis = two.getAxis(twoAxisIndex);
		glm::vec3 axis = glm::cross(oneAxis, twoAxis);
		axis = glm::normalize(axis);

		if (glm::dot(axis, toCentre) > 0.0f){
			axis = axis * -1.0f;
		}

		glm::vec3 ptOnOneEdge = one.halfSize;
		glm::vec3 ptOnTwoEdge = two.halfSize;
		for (unsigned i = 0; i < 3; i++) {
			if (i == oneAxisIndex) {
				ptOnOneEdge[i] = 0;
			}
			else if (glm::dot(one.getAxis(i), axis) > 0.0f) {
				ptOnOneEdge[i] = -ptOnOneEdge[i];
			}

			if (i == twoAxisIndex) {
				ptOnTwoEdge[i] = 0;
			}
			else if (glm::dot(two.getAxis(i), axis) < 0.0f) {
				ptOnTwoEdge[i] = -ptOnTwoEdge[i];
			}
		}

		ptOnOneEdge = glm::vec4(ptOnOneEdge, 1.0f) * one.transform;
		ptOnTwoEdge = glm::vec4(ptOnTwoEdge, 1.0f) * two.transform;

		glm::vec3 vertex = contactPoint(ptOnOneEdge, oneAxis, one.halfSize[oneAxisIndex], ptOnTwoEdge, twoAxis, two.halfSize[twoAxisIndex], bestSingleAxis > 2);

		Contact* contact = data->contacts;

		contact->penetration = pen;
		contact->contactNormal = axis;
		contact->contactPoint = vertex;
		contact->setBodyData(one.body, two.body, data->friction, data->restitution);
		data->addContacts(1);
		return 1;
	}
	return 0;
}
#undef CHECK_OVERLAP

unsigned CollisionDetector::boxAndPoint(const CollisionBox &box, const glm::vec3 &point, CollisionData *data) {
	
	glm::vec3 relPt = glm::inverse(box.transform) * glm::vec4(point, 1.0f);

	glm::vec3 normal;

	float min_depth = box.halfSize.x - abs(relPt.x);
	if (min_depth < 0) {
		return 0;
	}
	normal = box.getAxis(0) * ((relPt.x < 0.0f) ? -1.0f : 1.0f);

	float depth = box.halfSize.y - abs(relPt.y);
	if (depth < 0) {
		return 0;
	}
	else if (depth < min_depth) {
		min_depth = depth;
		normal = box.getAxis(1) * ((relPt.y < 0.0f) ? -1.0f : 1.0f);
	}

	depth = box.halfSize.z - abs(relPt.z);
	if (depth < 0) {
		return 0;
	}
	else if (depth < min_depth) {
		min_depth = depth;
		normal = box.getAxis(2) * ((relPt.z < 0.0f) ? -1.0f : 1.0f);
	}

	Contact* contact = data->contacts;
	contact->contactNormal = normal;
	contact->contactPoint = point;
	contact->penetration = min_depth;

	contact->setBodyData(box.body, NULL, data->friction, data->restitution);

	data->addContacts(1);
	return 1;
}

unsigned CollisionDetector::boxAndSphere(const CollisionBox &box, const CollisionSphere &sphere, CollisionData *data) {

	glm::vec3 centre = sphere.getAxis(3);
	glm::vec3 relCentre = glm::inverse(box.transform) * glm::vec4(centre, 1.0f);

	if (abs(relCentre.x) - sphere.radius > box.halfSize.x ||
		abs(relCentre.y) - sphere.radius > box.halfSize.y ||
		abs(relCentre.z) - sphere.radius > box.halfSize.z) {
		return 0;
	}

	glm::vec3 closestPt(0, 0, 0);
	float dist;

	dist = relCentre.x;
	if (dist > box.halfSize.x) dist = box.halfSize.x;
	if (dist < -box.halfSize.x) dist = -box.halfSize.x;
	closestPt.x = dist;

	dist = relCentre.y;
	if (dist > box.halfSize.y) dist = box.halfSize.y;
	if (dist < -box.halfSize.y) dist = -box.halfSize.y;
	closestPt.y = dist;

	dist = relCentre.z;
	if (dist > box.halfSize.z) dist = box.halfSize.z;
	if (dist < -box.halfSize.z) dist = -box.halfSize.z;
	closestPt.z = dist;

	dist = glm::length(closestPt - relCentre);
	dist *= dist;
	if (dist > sphere.radius * sphere.radius) return 0;

	glm::vec3 closestPtWorld = box.transform * glm::vec4(closestPt, 1.0f);

	Contact* contact = data->contacts;
	contact->contactNormal = (closestPtWorld - centre);
	contact->contactNormal = glm::normalize(contact->contactNormal);
	contact->contactPoint = closestPtWorld;
	contact->penetration = sphere.radius - sqrt(dist);
	contact->setBodyData(box.body, sphere.body, data->friction, data->restitution);

	data->addContacts(1);
	return 1;
}

unsigned CollisionDetector::boxAndHalfSpace(const CollisionBox &box, const CollisionPlane &plane, CollisionData *data) {

	if (data->contactsLeft <= 0) return 0;

	if (!IntersectionTests::boxAndHalfSpace(box, plane)) {
		return 0;
	}

	static float mults[8][3] = { {1,1,1}, {-1,1,1}, {1,-1,1}, {-1,-1,1}, {1,1,-1}, {-1,1,-1}, {1,-1,-1}, {-1,-1,-1} };

	Contact* contact = data->contacts;
	unsigned contactsUsed = 0;
	for (unsigned i = 0; i < 8; i++) {

		glm::vec3 vertexPos(mults[i][0], mults[i][1], mults[i][2]);

		vertexPos.x *= box.halfSize.x;
		vertexPos.y *= box.halfSize.y;
		vertexPos.z *= box.halfSize.z;

		vertexPos = glm::vec4(vertexPos, 1.0f) * box.transform;

		float vertexDistance = glm::dot(vertexPos, plane.direction);

		if (vertexDistance <= plane.offset) {

			contact->contactPoint = plane.direction;
			contact->contactPoint *= (vertexDistance - plane.offset);
			contact->contactPoint += vertexPos;
			contact->contactNormal = plane.direction;
			contact->penetration = plane.offset - vertexDistance;

			contact->setBodyData(box.body, NULL, data->friction, data->restitution);

			contact++;
			contactsUsed++;
			if (contactsUsed == (unsigned)data->contactsLeft){
				return contactsUsed;
			}
		}
	}

	data->addContacts(contactsUsed);
	return contactsUsed;
}