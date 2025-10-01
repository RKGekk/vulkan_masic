#pragma once

#include "particle.h"
#include "particle_force_generator.h"
#include "glm/glm.hpp"

class ParticleSpring : public ParticleForceGenerator {

private:
	Particle *other;
	float springConstant;
	float restLength;

public:
	ParticleSpring(Particle *anchoreParticle, float stiffness, float restDistance);
	ParticleSpring() = default;

	virtual void updateForce(Particle *particle, float duration);
};