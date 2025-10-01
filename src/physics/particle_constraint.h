#pragma once

#include "particle.h"
#include "particle_contact_generator.h"
#include "glm/glm.hpp"

class ParticleConstraint : public ParticleContactGenerator {

public:
	Particle *particle;
	glm::vec3 anchor;

protected:
	float currentLength() const;

public:
	virtual unsigned addContact(ParticleContact *contact, unsigned limit) const = 0;

};