#include "ground_contacts.h"

void GroundContacts::init(ParticleWorld::Particles *particles) {
    GroundContacts::particles = particles;
}

unsigned GroundContacts::addContact(ParticleContact *contact, unsigned limit) const {

    unsigned count = 0;
    for (ParticleWorld::Particles::iterator p = particles->begin(); p != particles->end(); p++) {

        float y = (*p)->getPosition().y;
        if (y < 0.0f) {
            contact->contactNormal = glm::vec3(0.0f, 1.0f, 0.0f);
            contact->particle[0] = *p;
            contact->particle[1] = NULL;
            contact->penetration = -y;
            contact->restitution = 0.2f;
            contact++;
            count++;
        }

        if (count >= limit) return count;
    }
    return count;
}