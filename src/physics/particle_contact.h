#pragma once

#include "particle.h"
#include "glm/glm.hpp"

class ParticleContactResolver;

class ParticleContact {

	friend class ParticleContactResolver;

public:
	Particle	*particle[2];
	float		restitution;
	glm::vec3	contactNormal;
	float		penetration;
    glm::vec3	particleMovement[2];

protected:
	void resolve(float duration);
	float calculateSeparatingVelocity() const;

private:
	void resolveVelocity(float duration);
	void resolveInterpenetration(float duration);
};