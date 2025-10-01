#pragma once

#include <vector>

#include "particle.h"
#include "particle_force_generator.h"

class ParticleForceRegistry {

protected:
	struct ParticleForceRegistration {
		Particle				*particle;
		ParticleForceGenerator	*forceGenerator;
	};

	typedef std::vector<ParticleForceRegistration> Registry;
	Registry registrations;

public:
	void add(Particle *particle, ParticleForceGenerator *forceGenerator);
	void remove(Particle *particle, ParticleForceGenerator *forceGenerator);
	void clear();
	void updateForces(float duration);
};