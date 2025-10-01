#pragma once

#include "particle.h"
#include "particle_contact.h"
#include "particle_contact_generator.h"
#include "particle_world.h"
#include "glm/glm.hpp"

class GroundContacts : public ParticleContactGenerator {

private:
	ParticleWorld::Particles *particles;

public:
	void init(ParticleWorld::Particles *particles);

	virtual unsigned addContact(ParticleContact *contact, unsigned limit) const;
};