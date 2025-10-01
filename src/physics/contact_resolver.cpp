#include "contact_resolver.h"

ContactResolver::ContactResolver(unsigned iterations, float velocityEpsilon, float positionEpsilon) {
	setIterations(iterations, iterations);
	setEpsilon(velocityEpsilon, positionEpsilon);
}

ContactResolver::ContactResolver(unsigned velocityIterations, unsigned positionIterations, float velocityEpsilon, float positionEpsilon) {
	setIterations(velocityIterations);
	setEpsilon(velocityEpsilon, positionEpsilon);
}

void ContactResolver::setIterations(unsigned iterations) {
	setIterations(iterations, iterations);
}

void ContactResolver::setIterations(unsigned velocityIterations,
	unsigned positionIterations) {
	ContactResolver::velocityIterations = velocityIterations;
	ContactResolver::positionIterations = positionIterations;
}

void ContactResolver::setEpsilon(float velocityEpsilon, float positionEpsilon) {
	ContactResolver::velocityEpsilon = velocityEpsilon;
	ContactResolver::positionEpsilon = positionEpsilon;
}

void ContactResolver::resolveContacts(Contact *contacts, unsigned numContacts, float duration) {

	if (numContacts == 0) return;
	if (!isValid()) return;

	prepareContacts(contacts, numContacts, duration);

	adjustPositions(contacts, numContacts, duration);

	adjustVelocities(contacts, numContacts, duration);
}

void ContactResolver::prepareContacts(Contact* contacts, unsigned numContacts, float duration) {

	Contact* lastContact = contacts + numContacts;
	for (Contact* contact = contacts; contact < lastContact; contact++) {

		contact->calculateInternals(duration);
	}
}

void ContactResolver::adjustVelocities(Contact *c, unsigned numContacts, float duration) {
	glm::vec3 velocityChange[2], rotationChange[2];
	glm::vec3 deltaVel;

	velocityIterationsUsed = 0;
	while (velocityIterationsUsed < velocityIterations) {
		
		float max = velocityEpsilon;
		unsigned index = numContacts;
		for (unsigned i = 0; i < numContacts; i++) {
			if (c[i].desiredDeltaVelocity > max) {
				max = c[i].desiredDeltaVelocity;
				index = i;
			}
		}
		if (index == numContacts) break;

		c[index].matchAwakeState();

		c[index].applyVelocityChange(velocityChange, rotationChange);

		for (unsigned i = 0; i < numContacts; i++) {
			for (unsigned b = 0; b < 2; b++) if (c[i].body[b]) {
				for (unsigned d = 0; d < 2; d++) {
					if (c[i].body[b] == c[index].body[d]) {

						deltaVel = velocityChange[d] + glm::cross(rotationChange[d], c[i].relativeContactPosition[b]);

						c[i].contactVelocity += glm::transpose(c[i].contactToWorld) * deltaVel * (b ? -1.0f : 1.0f);
						c[i].calculateDesiredDeltaVelocity(duration);
					}
				}
			}
		}
		velocityIterationsUsed++;
	}
}

void ContactResolver::adjustPositions(Contact *c, unsigned numContacts, float duration) {
	unsigned i, index;
	glm::vec3 linearChange[2], angularChange[2];
	float max;
	glm::vec3 deltaPosition;

	positionIterationsUsed = 0;
	while (positionIterationsUsed < positionIterations) {

		max = positionEpsilon;
		index = numContacts;
		for (i = 0; i < numContacts; i++) {
			if (c[i].penetration > max) {
				max = c[i].penetration;
				index = i;
			}
		}
		if (index == numContacts) {
			break;
		}

		c[index].matchAwakeState();
		c[index].applyPositionChange(linearChange, angularChange, max);

		for (i = 0; i < numContacts; i++) {
			for (unsigned b = 0; b < 2; b++) if (c[i].body[b]) {
				for (unsigned d = 0; d < 2; d++) {
					if (c[i].body[b] == c[index].body[d]) {
						deltaPosition = linearChange[d] + glm::cross(angularChange[d], c[i].relativeContactPosition[b]);

						c[i].penetration += glm::dot(deltaPosition, c[i].contactNormal) * (b ? 1.0f : -1.0f);
					}
				}
			}
		}
		positionIterationsUsed++;
	}
}