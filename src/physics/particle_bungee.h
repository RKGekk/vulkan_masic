#pragma once

#include "particle.h"
#include "particle_force_generator.h"
#include "glm/glm.hpp"

class ParticleBungee : public ParticleForceGenerator {

private:
	Particle *other;
	float springConstant;
	float restLength;

public:
	ParticleBungee(Particle *anchoreParticle, float stiffness, float restDistance);
	ParticleBungee() = default;

	virtual void updateForce(Particle *particle, float duration);
};