#include "particle_cable.h"

unsigned ParticleCable::addContact(ParticleContact *contact, unsigned limit) const {

	float length = currentLength();
	if(length < maxLength) {
		return 0;
	}

	contact->particle[0] = particle[0];
	contact->particle[1] = particle[1];

	glm::vec3 normal = particle[1]->getPosition() - particle[0]->getPosition();
	normal = glm::normalize(normal);

	contact->contactNormal = normal;
	contact->penetration = length - maxLength;
	contact->restitution = restitution;

	return 1;
}