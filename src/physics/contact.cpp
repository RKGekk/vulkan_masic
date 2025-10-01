#include "contact.h"

void Contact::setBodyData(RigidBody* one, RigidBody *two, float friction, float restitution) {
	Contact::body[0] = one;
	Contact::body[1] = two;
	Contact::friction = friction;
	Contact::restitution = restitution;
}

void Contact::matchAwakeState() {

	if (!body[1]) return;

	bool body0awake = body[0]->getAwake();
	bool body1awake = body[1]->getAwake();

	if (body0awake ^ body1awake) {
		if (body0awake) body[1]->setAwake();
		else body[0]->setAwake();
	}
}

void Contact::swapBodies() {
	contactNormal *= -1;

	RigidBody *temp = body[0];
	body[0] = body[1];
	body[1] = temp;
}

inline void Contact::calculateContactBasis() {
	glm::vec3 contactTangent[2];

	if (abs(contactNormal.x) > abs(contactNormal.y)) {

		const float s = 1.0f / sqrt(contactNormal.z * contactNormal.z + contactNormal.x * contactNormal.x);

		contactTangent[0].x = contactNormal.z * s;
		contactTangent[0].y = 0.0f;
		contactTangent[0].z = -contactNormal.x * s;

		contactTangent[1].x = contactNormal.y * contactTangent[0].x;
		contactTangent[1].y = contactNormal.z * contactTangent[0].x - contactNormal.x * contactTangent[0].z;
		contactTangent[1].z = -contactNormal.y * contactTangent[0].x;
	}
	else {

		const float s = 1.0f / sqrt(contactNormal.z * contactNormal.z + contactNormal.y * contactNormal.y);

		contactTangent[0].x = 0.0f;
		contactTangent[0].y = -contactNormal.z * s;
		contactTangent[0].z = contactNormal.y * s;

		contactTangent[1].x = contactNormal.y * contactTangent[0].z - contactNormal.z * contactTangent[0].y;
		contactTangent[1].y = -contactNormal.x * contactTangent[0].z;
		contactTangent[1].z = contactNormal.x * contactTangent[0].y;
	}

	contactToWorld[0] = contactNormal;
	contactToWorld[1] = contactTangent[0];
	contactToWorld[2] = contactTangent[1];
}

glm::vec3 Contact::calculateLocalVelocity(unsigned bodyIndex, float duration) {
	RigidBody *thisBody = body[bodyIndex];

	glm::vec3 velocity = glm::cross(thisBody->getRotation(), relativeContactPosition[bodyIndex]);
	velocity += thisBody->getVelocity();

	glm::vec3 contactVelocity = velocity * contactToWorld;

	glm::vec3 accVelocity = thisBody->getLastFrameAcceleration() * duration;

	accVelocity = accVelocity * contactToWorld;

	accVelocity.x = 0.0f;
	contactVelocity += accVelocity;

	return contactVelocity;
}


void Contact::calculateDesiredDeltaVelocity(float duration) {

	const static float velocityLimit = 0.25f;

	float velocityFromAcc = 0.0f;

	if (body[0]->getAwake()) {
		velocityFromAcc += glm::dot(body[0]->getLastFrameAcceleration() * duration, contactNormal);
	}

	if (body[1] && body[1]->getAwake()) {
		velocityFromAcc -= glm::dot(body[1]->getLastFrameAcceleration() * duration, contactNormal);
	}

	float thisRestitution = restitution;
	if (abs(contactVelocity.x) < velocityLimit) {
		thisRestitution = 0.0f;
	}

	desiredDeltaVelocity = -contactVelocity.x - thisRestitution * (contactVelocity.x - velocityFromAcc);
}


void Contact::calculateInternals(float duration) {

	if (!body[0]) {
		swapBodies();
	}
	assert(body[0]);

	calculateContactBasis();

	relativeContactPosition[0] = contactPoint - body[0]->getPosition();
	if (body[1]) {
		relativeContactPosition[1] = contactPoint - body[1]->getPosition();
	}

	contactVelocity = calculateLocalVelocity(0, duration);
	if (body[1]) {
		contactVelocity -= calculateLocalVelocity(1, duration);
	}

	calculateDesiredDeltaVelocity(duration);
}

void Contact::applyVelocityChange(glm::vec3 velocityChange[2], glm::vec3 rotationChange[2]) {

	const float zero = std::numeric_limits<float>::epsilon() + 0.0001f;

	glm::mat3 inverseInertiaTensor[2];
	body[0]->getInverseInertiaTensorWorld(&inverseInertiaTensor[0]);
	if (body[1]) {
		body[1]->getInverseInertiaTensorWorld(&inverseInertiaTensor[1]);
	}

	glm::vec3 impulseContact;

	float frictionMagnitude = glm::length(friction);
	if (frictionMagnitude <= zero) {

		impulseContact = calculateFrictionlessImpulse(inverseInertiaTensor);
	}
	else {

		impulseContact = calculateFrictionImpulse(inverseInertiaTensor);
	}

	glm::vec3 impulse = contactToWorld * impulseContact;

	glm::vec3 impulsiveTorque = glm::cross(relativeContactPosition[0], impulse);
	rotationChange[0] = inverseInertiaTensor[0] * impulsiveTorque;
	velocityChange[0] = glm::vec3();
	velocityChange[0] += impulse * body[0]->getInverseMass();

	body[0]->addVelocity(velocityChange[0]);
	body[0]->addRotation(rotationChange[0]);

	if (body[1]) {

		glm::vec3 impulsiveTorque = glm::cross(impulse, relativeContactPosition[1]);
		rotationChange[1] = inverseInertiaTensor[1] * impulsiveTorque;
		velocityChange[1] = glm::vec3();
		velocityChange[1] += impulse * -body[1]->getInverseMass();

		body[1]->addVelocity(velocityChange[1]);
		body[1]->addRotation(rotationChange[1]);
	}
}

inline glm::vec3 Contact::calculateFrictionlessImpulse(glm::mat3 * inverseInertiaTensor) {
	glm::vec3 impulseContact;

	glm::vec3 deltaVelWorld = relativeContactPosition[0] * contactNormal;
	deltaVelWorld = inverseInertiaTensor[0] * deltaVelWorld;
	deltaVelWorld = deltaVelWorld * relativeContactPosition[0];

	float deltaVelocity = glm::dot(deltaVelWorld, contactNormal);

	deltaVelocity += body[0]->getInverseMass();

	if (body[1]) {

		glm::vec3 deltaVelWorld = relativeContactPosition[1] * contactNormal;
		deltaVelWorld = inverseInertiaTensor[1] * deltaVelWorld;
		deltaVelWorld = deltaVelWorld * relativeContactPosition[1];

		deltaVelocity += glm::dot(deltaVelWorld, contactNormal);

		deltaVelocity += body[1]->getInverseMass();
	}

	impulseContact.x = desiredDeltaVelocity / deltaVelocity;
	impulseContact.y = 0;
	impulseContact.z = 0;
	return impulseContact;
}

inline glm::vec3 Contact::calculateFrictionImpulse(glm::mat3 * inverseInertiaTensor) {
	glm::vec3 impulseContact;
	float inverseMass = body[0]->getInverseMass();

	glm::mat3 impulseToTorque;
	impulseToTorque[0][0] = 0.0f;
	impulseToTorque[1][1] = 0.0f;
	impulseToTorque[2][2] = 0.0f;

	impulseToTorque[0][1] = -relativeContactPosition[0].z;
	impulseToTorque[0][2] = relativeContactPosition[0].y;
	impulseToTorque[1][0] = relativeContactPosition[0].z;
	impulseToTorque[1][2] = -relativeContactPosition[0].x;
	impulseToTorque[2][0] = -relativeContactPosition[0].y;
	impulseToTorque[2][1] = relativeContactPosition[0].x;

	glm::mat3 deltaVelWorld = impulseToTorque;
	deltaVelWorld *= inverseInertiaTensor[0];
	deltaVelWorld *= impulseToTorque;
	deltaVelWorld *= -1;

	if (body[1]) {

		impulseToTorque[0][0] = 0.0f;
		impulseToTorque[1][1] = 0.0f;
		impulseToTorque[2][2] = 0.0f;

		impulseToTorque[0][1] = -relativeContactPosition[1].z;
		impulseToTorque[0][2] = relativeContactPosition[1].y;
		impulseToTorque[1][0] = relativeContactPosition[1].z;
		impulseToTorque[1][2] = -relativeContactPosition[1].x;
		impulseToTorque[2][0] = -relativeContactPosition[1].y;
		impulseToTorque[2][1] = relativeContactPosition[1].x;

		glm::mat3 deltaVelWorld2 = impulseToTorque;
		deltaVelWorld2 *= inverseInertiaTensor[1];
		deltaVelWorld2 *= impulseToTorque;
		deltaVelWorld2 *= -1;

		deltaVelWorld += deltaVelWorld2;

		inverseMass += body[1]->getInverseMass();
	}

	glm::mat3 deltaVelocity = glm::transpose(contactToWorld);
	deltaVelocity *= deltaVelWorld;
	deltaVelocity *= contactToWorld;

	deltaVelocity[0][0] += inverseMass;
	deltaVelocity[1][1] += inverseMass;
	deltaVelocity[2][2] += inverseMass;

	glm::mat3 impulseMatrix = glm::inverse(deltaVelocity);

	glm::vec3 velKill(desiredDeltaVelocity, -contactVelocity.y, -contactVelocity.z);

	impulseContact = impulseMatrix * velKill;

	float planarImpulse = sqrt(impulseContact.y*impulseContact.y + impulseContact.z*impulseContact.z);
	if (planarImpulse > impulseContact.x * friction) {

		impulseContact.y /= planarImpulse;
		impulseContact.z /= planarImpulse;

		impulseContact.x = deltaVelocity[0][0] + deltaVelocity[0][1] * friction*impulseContact.y + deltaVelocity[0][2] * friction*impulseContact.z;
		impulseContact.x = desiredDeltaVelocity / impulseContact.x;
		impulseContact.y *= friction * impulseContact.x;
		impulseContact.z *= friction * impulseContact.x;
	}
	return impulseContact;
}

void Contact::applyPositionChange(glm::vec3 linearChange[2], glm::vec3 angularChange[2], float penetration) {

	const float angularLimit = 0.2f;
	float angularMove[2];
	float linearMove[2];

	float totalInertia = 0;
	float linearInertia[2];
	float angularInertia[2];

	for (unsigned i = 0; i < 2; i++) if (body[i]) {
		glm::mat3 inverseInertiaTensor;
		body[i]->getInverseInertiaTensorWorld(&inverseInertiaTensor);

		glm::vec3 angularInertiaWorld = glm::cross(relativeContactPosition[i], contactNormal);
		angularInertiaWorld = angularInertiaWorld * inverseInertiaTensor;
		angularInertiaWorld = glm::cross(angularInertiaWorld, relativeContactPosition[i]);
		angularInertia[i] = glm::dot(angularInertiaWorld, contactNormal);

		linearInertia[i] = body[i]->getInverseMass();

		totalInertia += linearInertia[i] + angularInertia[i];
	}

	for (unsigned i = 0; i < 2; i++) if (body[i]) {

		float sign = (i == 0) ? 1 : -1;
		angularMove[i] = sign * penetration * (angularInertia[i] / totalInertia);
		linearMove[i] = sign * penetration * (linearInertia[i] / totalInertia);

		glm::vec3 projection = relativeContactPosition[i];
		projection += contactNormal * glm::dot(-relativeContactPosition[i], contactNormal);

		float maxMagnitude = angularLimit * glm::length(projection);

		if (angularMove[i] < -maxMagnitude) {
			float totalMove = angularMove[i] + linearMove[i];
			angularMove[i] = -maxMagnitude;
			linearMove[i] = totalMove - angularMove[i];
		}
		else if (angularMove[i] > maxMagnitude) {
			float totalMove = angularMove[i] + linearMove[i];
			angularMove[i] = maxMagnitude;
			linearMove[i] = totalMove - angularMove[i];
		}

		if (abs(angularMove[i]) < 0.0001f) {

			angularChange[i] = glm::vec3();
		}
		else {

			glm::vec3 targetAngularDirection = glm::cross(relativeContactPosition[i], contactNormal);

			glm::mat3 inverseInertiaTensor;
			body[i]->getInverseInertiaTensorWorld(&inverseInertiaTensor);

			angularChange[i] = inverseInertiaTensor * targetAngularDirection * (angularMove[i] / angularInertia[i]);
		}

		linearChange[i] = contactNormal * linearMove[i];

		glm::vec3 pos;
		body[i]->getPosition(&pos);
		pos += contactNormal * linearMove[i];
		body[i]->setPosition(pos);

		glm::quat q;
		body[i]->getOrientation(&q);

		glm::quat q2(0.0f, angularChange[i].x * 1.0f, angularChange[i].y * 1.0f, angularChange[i].z * 1.0f);
		q2 *= q;
		q.w += q2.w * 0.5f;
		q.x += q2.x * 0.5f;
		q.y += q2.y * 0.5f;
		q.z += q2.z * 0.5f;

		body[i]->setOrientation(q);

		if (!body[i]->getAwake()) body[i]->calculateDerivedData();
	}
}