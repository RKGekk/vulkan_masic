#include "particle_link.h"

float ParticleLink::currentLength() const {

	glm::vec3 relativePos = particle[0]->getPosition() - particle[1]->getPosition();
	return glm::length(relativePos);
}