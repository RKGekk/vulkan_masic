#include "particle_rod.h"

unsigned ParticleRod::addContact(ParticleContact *contact, unsigned limit) const {

	float currentLen = currentLength();
	if(currentLen < length) {
		return 0;
	}

	contact->particle[0] = particle[0];
	contact->particle[1] = particle[1];

	glm::vec3 normal = particle[1]->getPosition() - particle[0]->getPosition();
	normal = glm::normalize(normal);

	if(currentLen > length) {

		contact->contactNormal = normal;
		contact->penetration = currentLen - length;
	}
	else {

		contact->contactNormal = normal * -1.0f;
		contact->penetration = length - currentLen;
	}
	
	contact->restitution = 0.0f;

	return 1;
}