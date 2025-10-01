#include "particle.h"

void Particle::integrate(float duration) {

	const float zero = std::numeric_limits<float>::epsilon() + 0.0001f;

	if(inverseMass <= zero) {
		return;
	}

	position += velocity * duration;

	glm::vec3 resultAcc = acceleration;
	resultAcc += forceAccum * inverseMass;

	velocity += resultAcc * duration;
	velocity *= powf(damping, duration);

	clearAccumulator();
}

void Particle::setMass(const float mass) {
	inverseMass = 1.0f / mass;
}

float Particle::getMass() const {
	if(inverseMass <= zero) {
		return maxMass;
	}
	else {
		return 1.0f / inverseMass;
	}
}

void Particle::setRadius(const float rad) {
	radius = rad;
}

float Particle::getRadius() const {
	return radius;
}


void Particle::setInverseMass(const float invMass) {
	inverseMass = invMass; 
}

float Particle::getInverseMass() const {
	return inverseMass;
}

bool Particle::hasFiniteMass() const {
    return inverseMass >= zero;
}

void Particle::setDamping(const float dmp) {
    damping = dmp;
}

float Particle::getDamping() const {
    return damping;
}

void Particle::setPosition(const glm::vec3 &pos) {
    position = pos;
}

void Particle::setPosition(const float x, const float y, const float z) {
    position.x = x;
    position.y = y;
    position.z = z;
}

void Particle::getPosition(glm::vec3 *pos) const {
    *pos = position;
}

glm::vec3 Particle::getPosition() const {
    return position;
}

void Particle::setVelocity(const glm::vec3 &vel) {
    Particle::velocity = vel;
}

void Particle::setVelocity(const float x, const float y, const float z) {
    velocity.x = x;
    velocity.y = y;
    velocity.z = z;
}

void Particle::getVelocity(glm::vec3 *vel) const {
    *vel = velocity;
}

glm::vec3 Particle::getVelocity() const {
    return velocity;
}

void Particle::setAcceleration(const glm::vec3 &acc) {
    acceleration = acc;
}

void Particle::setAcceleration(const float x, const float y, const float z) {
    acceleration.x = x;
    acceleration.y = y;
    acceleration.z = z;
}

void Particle::getAcceleration(glm::vec3 *acc) const {
    *acc = Particle::acceleration;
}

glm::vec3 Particle::getAcceleration() const {
    return acceleration;
}

void Particle::clearAccumulator() {
    forceAccum = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Particle::addForce(const glm::vec3 &force) {
    forceAccum += force;
}