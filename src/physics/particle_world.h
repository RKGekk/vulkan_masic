#pragma once

#include <vector>

#include "particle.h"
#include "particle_force_generator.h"
#include "particle_force_registry.h"
#include "particle_contact.h"
#include "particle_contact_generator.h"
#include "particle_contact_resolver.h"
#include "glm/glm.hpp"

class ParticleWorld {

public:
	typedef std::vector<Particle *> Particles;
	typedef std::vector<ParticleContactGenerator *> ContactGenerators;

protected:
	Particles				particles;
	bool					calculateIterations;
	ParticleForceRegistry	registry;
	ParticleContactResolver	resolver;
	ContactGenerators		contactGenerators;
	ParticleContact			*contacts;
	unsigned				maxContacts;

public:
	ParticleWorld(unsigned maxContacts, unsigned iterations = 0);
	~ParticleWorld();

	unsigned generateContacts();
	void integrate(float duration);
	void runPhysics(float duration);
	void startFrame();
	Particles& getParticles();
	ContactGenerators& getContactGenerators();
	ParticleForceRegistry& getForceRegistry();
};