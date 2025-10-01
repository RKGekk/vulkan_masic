#include "sphere_contacts.h"

void SphereContacts::init(ParticleWorld::Particles *particles) {
    SphereContacts::particles = particles;
}

unsigned SphereContacts::addContact(ParticleContact *contact, unsigned limit) const {

    unsigned count = 0;
    for (ParticleWorld::Particles::iterator p1 = particles->begin(); p1 != particles->end(); p1++) {
		for (ParticleWorld::Particles::iterator p2 = p1; p2 != particles->end(); p2++) {
			if(p2 != p1) {
				glm::vec3 particle1Pos = (*p1)->getPosition();
				glm::vec3 particle2Pos = (*p2)->getPosition();
				float rad1 = (*p1)->getRadius();
				float rad2 = (*p2)->getRadius();

				glm::vec3 contactTrace = particle1Pos - particle2Pos;
				float distance = glm::length(contactTrace);

				if (distance < (rad1 + rad2)) {
				    contact->contactNormal = glm::normalize(contactTrace);
				    contact->particle[0] = *p1;
				    contact->particle[1] = *p2;
				    contact->penetration = (rad1 + rad2) - distance;
				    contact->restitution = 0.2f;
				    contact++;
				    count++;
				}

				if (count >= limit) return count;
			}
		}
    }
    return count;
}