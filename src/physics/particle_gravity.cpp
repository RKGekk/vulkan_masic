#include "particle_gravity.h"

ParticleGravity::ParticleGravity(const glm::vec3 &gravityG) {
	gravity = gravityG;
}

void ParticleGravity::updateForce(Particle *particle, float duration) {

	if (!particle->hasFiniteMass()) {
		return;
	}

	particle->addForce(gravity * particle->getMass());
}