#include "particle_fake_spring.h"

ParticleFakeSpring::ParticleFakeSpring(glm::vec3 anchorPlace, float stiffness, float dampSpeed) {
	anchor = anchorPlace;
	springConstant = stiffness;
	damping = dampSpeed;
}

void ParticleFakeSpring::updateForce(Particle *particle, float duration) {

	const float zero = std::numeric_limits<float>::epsilon() + 0.0001f;

	if(!particle->hasFiniteMass()) {
		return;
	}

	glm::vec3 position;
	particle->getPosition(&position);
	position -= anchor;

	float gamma = 0.5f * glm::sqrt(4.0f * springConstant - damping * damping);
	if(glm::abs(gamma) <= zero ) {
		return;
	}

	glm::vec3 c = position * (damping / (2.0f * gamma)) + particle->getVelocity() * (1.0f / gamma);
	glm::vec3 target = position * glm::cos(gamma * duration) + c * glm::sin(gamma * duration);
	target *= glm::exp(-0.5f * duration * damping);
	glm::vec3 accel = (target - position) * (1.0f / (duration * duration)) - particle->getVelocity() * (1.0f / duration);

	particle->addForce(accel * particle->getMass());
}