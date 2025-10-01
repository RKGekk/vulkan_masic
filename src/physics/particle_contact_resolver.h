#pragma once

#include "particle.h"
#include "particle_contact.h"
#include "glm/glm.hpp"

class ParticleContactResolver {

protected:
	unsigned iterations;
	unsigned iterationsUsed;

public:
	ParticleContactResolver(unsigned iter);

	void setIterations(unsigned iter);
	void resolveContacts(ParticleContact *contactArray, unsigned numContacts, float duration);

};