#pragma once

#include "particle.h"
#include "particle_contact_generator.h"
#include "particle_link.h"
#include "glm/glm.hpp"

class ParticleRod : public ParticleLink {

public:
	float length;

public:
	virtual unsigned addContact(ParticleContact *contact, unsigned limit) const;

};
