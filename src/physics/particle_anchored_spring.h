#pragma once

#include "particle.h"
#include "particle_force_generator.h"
#include "glm/glm.hpp"

class ParticleAnchoredSpring : public ParticleForceGenerator {

protected:

	glm::vec3 anchor;
	float springConstant;
	float restLength;

public:

	ParticleAnchoredSpring(glm::vec3 anchorPlace, float stiffness, float restDistance);
	ParticleAnchoredSpring() = default;

	virtual void updateForce(Particle *particle, float duration);
};