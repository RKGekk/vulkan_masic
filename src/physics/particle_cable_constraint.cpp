#include "particle_cable_constraint.h"

unsigned ParticleCableConstraint::addContact(ParticleContact *contact, unsigned limit) const {
    
    float length = currentLength();

    if (length < maxLength) {
        return 0;
    }

    contact->particle[0] = particle;
    contact->particle[1] = 0;

    glm::vec3 normal = anchor - particle->getPosition();
    normal = glm::normalize(normal);
    contact->contactNormal = normal;

    contact->penetration = length-maxLength;
    contact->restitution = restitution;

    return 1;
}