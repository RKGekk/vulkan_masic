#pragma once

#include <math.h>
#include <limits>

#include "glm/glm.hpp"

class Particle {

protected:

	const float zero = std::numeric_limits<float>::epsilon() + 0.0001f;
	const float maxMass = std::numeric_limits<float>::max();

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 forceAccum;

	float radius;
	float damping;
	float inverseMass;

public:
	void integrate(float duration);

	void setMass(const float mass);
	float getMass() const;

	void setRadius(const float mass);
	float getRadius() const;

	void setInverseMass(const float inverseMass);
	float getInverseMass() const;

	bool hasFiniteMass() const;


	void setDamping(const float damping);
    float getDamping() const;

	void setPosition(const glm::vec3 &position);
    void setPosition(const float x, const float y, const float z);
    void getPosition(glm::vec3 *position) const;
	glm::vec3 getPosition() const;

    void setVelocity(const glm::vec3 &velocity);
    void setVelocity(const float x, const float y, const float z);
    void getVelocity(glm::vec3 *velocity) const;
    glm::vec3 getVelocity() const;

    void setAcceleration(const glm::vec3 &acceleration);
    void setAcceleration(const float x, const float y, const float z);
    void getAcceleration(glm::vec3 *acceleration) const;
    glm::vec3 getAcceleration() const;

    void clearAccumulator();

    void addForce(const glm::vec3 &force);
};