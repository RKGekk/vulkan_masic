#include "particle_anchored_spring.h"

ParticleAnchoredSpring::ParticleAnchoredSpring(glm::vec3 anchorPlace, float stiffness, float restDistance) {
	anchor = anchorPlace;
	springConstant = stiffness;
	restLength = restDistance;
}

void ParticleAnchoredSpring::updateForce(Particle *particle, float duration) {

	const float zero = std::numeric_limits<float>::epsilon() + 0.0001f;

	glm::vec3 force;
	particle->getPosition(&force);
	force -= anchor;

	float magnitude = glm::length(force);
	if( magnitude <= zero) {
		return;
	}

	magnitude = (restLength - magnitude) * springConstant;

	force = glm::normalize(force);
	force *= magnitude;
	particle->addForce(force);
}