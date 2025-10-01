#include "particle_bungee.h"

ParticleBungee::ParticleBungee(Particle *anchoParticle, float stiffness, float restDistance) {
	other = anchoParticle;
	springConstant = stiffness;
	restLength = restDistance;
}

void ParticleBungee::updateForce(Particle *particle, float duration) {

	const float zero = std::numeric_limits<float>::epsilon() + 0.0001f;

	glm::vec3 force;
	particle->getPosition(&force);
	force -= other->getPosition();

	float magnitude = glm::length(force);
	if( magnitude <= zero || magnitude <= restLength) {
		return;
	}

	magnitude = glm::abs(magnitude) - restLength;
	magnitude *= springConstant;

	force = glm::normalize(force);
	force *= -magnitude;

	particle->addForce(force);
}