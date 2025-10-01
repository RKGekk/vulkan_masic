#pragma once

#include "particle.h"
#include "particle_force_generator.h"
#include "glm/glm.hpp"

class ParticleFakeSpring : public ParticleForceGenerator {

protected:

	glm::vec3 anchor;
	float springConstant;
	float damping;

public:

	ParticleFakeSpring(glm::vec3 anchorPlace, float stiffness, float dampSpeed);
	ParticleFakeSpring() = default;

	virtual void updateForce(Particle *particle, float duration);
};