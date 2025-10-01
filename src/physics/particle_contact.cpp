#include "particle_contact.h"

void ParticleContact::resolve(float duration) {

	resolveVelocity(duration);
	resolveInterpenetration(duration);
}

float ParticleContact::calculateSeparatingVelocity() const {

	glm::vec3 relativeVelocity = particle[0]->getVelocity();
	if(particle[1]) {
		relativeVelocity -= particle[1]->getVelocity();
	}

	return glm::dot(relativeVelocity, contactNormal);
}

void ParticleContact::resolveVelocity(float duration) {

	float separatingVelocity = calculateSeparatingVelocity();

	if(separatingVelocity > 0.0f) {
		return;
	}

	float newSeparaiongVelocity = -separatingVelocity * restitution;

	glm::vec3 accCausedVelocity = particle[0]->getAcceleration();
	if(particle[1]) {
		accCausedVelocity -= particle[1]->getAcceleration();
	}

	float accCausedSepVelocity = glm::dot(accCausedVelocity, contactNormal) * duration;
	if(accCausedSepVelocity < 0.0f) {
		newSeparaiongVelocity += restitution * accCausedSepVelocity;
		if(newSeparaiongVelocity < 0.0f) {
			newSeparaiongVelocity = 0.0f;
		}
	}

	float deltaVelocity = newSeparaiongVelocity - separatingVelocity;

	float totalInversMass = particle[0]->getInverseMass();
	if(particle[1]) {
		totalInversMass += particle[1]->getInverseMass();
	}

	if(totalInversMass <= 0.0f) {
		return;
	}

	float impulse = deltaVelocity / totalInversMass;
	glm::vec3 impulsePerIMass = contactNormal * impulse;

	particle[0]->setVelocity(particle[0]->getVelocity() + impulsePerIMass * particle[0]->getInverseMass());
	if(particle[1]) {
		particle[1]->setVelocity(particle[1]->getVelocity() + impulsePerIMass * -particle[1]->getInverseMass());
	}

}


/*
void ParticleContact::resolveVelocity(float duration) {

	float separatingVelocity = calculateSeparatingVelocity();

	if(separatingVelocity > 0.0f) {
		return;
	}

	float newSeparaiongVelocity = -separatingVelocity * restitution;
	float deltaVelocity = newSeparaiongVelocity - separatingVelocity;

	float totalInversMass = particle[0]->getInverseMass();
	if(particle[1]) {
		totalInversMass += particle[1]->getInverseMass();
	}

	if(totalInversMass > 0.0f) {
		return;
	}

	float impulse = deltaVelocity / totalInversMass;
	glm::vec3 impulsePerMass = contactNormal * impulse;

	particle[0]->setVelocity(particle[0]->getVelocity() + impulsePerMass * particle[0]->getInverseMass());
	if(particle[1]) {
		particle[1]->setVelocity(particle[1]->getVelocity() + impulsePerMass * -particle[1]->getInverseMass());
	}

}
*/

void ParticleContact::resolveInterpenetration(float duration) {

	if(penetration <= 0) {
		return;
	}

	float totalInvertMass = particle[0]->getMass();
	if(particle[1]) {
		totalInvertMass += particle[1]->getMass();
	}

	if(totalInvertMass <= 0.0f) {
		return;
	}

	glm::vec3 movePerIMass = contactNormal * (penetration / totalInvertMass);
	particleMovement[0] = movePerIMass * particle[0]->getInverseMass();
	if(particle[1]) {
		particleMovement[1] = movePerIMass * -particle[1]->getInverseMass();
	}
	else {
		particleMovement[1] = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	particle[0]->setPosition(particle[0]->getPosition() + particleMovement[0]);
	if(particle[1]) {
		particle[1]->setPosition(particle[1]->getPosition() + particleMovement[1]);
	}
}