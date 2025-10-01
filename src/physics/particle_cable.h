#pragma once

#include "particle.h"
#include "particle_contact_generator.h"
#include "particle_link.h"
#include "glm/glm.hpp"

class ParticleCable : public ParticleLink {

public:
	float maxLength;
	float restitution;

public:
	virtual unsigned addContact(ParticleContact *contact, unsigned limit) const;

};