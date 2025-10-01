#include "particle_drag.h"

ParticleDrag::ParticleDrag(float koef1, float koef2) {
	k1 = koef1;
	k2 = koef2;
}

void ParticleDrag::updateForce(Particle *particle, float duration) {

	const float zero = std::numeric_limits<float>::epsilon() + 0.0001f;

	glm::vec3 force;
	particle->getVelocity(&force);

	float dragKoeff = glm::length(force);
	if(dragKoeff <= zero) {
		return;
	}

	dragKoeff = k1 * dragKoeff + k2 * dragKoeff * dragKoeff;

	force = glm::normalize(force);
	force *= -dragKoeff;

	particle->addForce(force);
}