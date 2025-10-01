#pragma once

#include "particle.h"
#include "particle_contact_generator.h"
#include "glm/glm.hpp"

class ParticleLink : public ParticleContactGenerator {

public:
	Particle *particle[2];

protected:
	float currentLength() const;

public:
	virtual unsigned addContact(ParticleContact *contact, unsigned limit) const = 0;
};