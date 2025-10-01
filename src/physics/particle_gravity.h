#pragma once

#include "particle_force_generator.h"
#include "glm/glm.hpp"

class ParticleGravity : public ParticleForceGenerator {

private:
	glm::vec3 gravity;

public:
	ParticleGravity(const glm::vec3 &gravityG);
	ParticleGravity() = default;

	virtual void updateForce(Particle *particle, float duration);
};