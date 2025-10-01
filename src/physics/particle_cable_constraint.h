#pragma once

#include "particle.h"
#include "particle_contact_generator.h"
#include "particle_constraint.h"
#include "glm/glm.hpp"

class ParticleCableConstraint : public ParticleConstraint {

public:
	float maxLength;
	float restitution;

public:
	virtual unsigned addContact(ParticleContact *contact, unsigned limit) const;
};