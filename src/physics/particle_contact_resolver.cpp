#include "particle_contact_resolver.h"

ParticleContactResolver::ParticleContactResolver(unsigned iterations) : iterations(iterations) {

}

void ParticleContactResolver::setIterations(unsigned iterations) {

    ParticleContactResolver::iterations = iterations;
}

void ParticleContactResolver::resolveContacts(ParticleContact *contactArray, unsigned numContacts, float duration) {

	unsigned i;
	iterationsUsed = 0;
	while(iterationsUsed < iterations) {

		float max = std::numeric_limits<float>::max();
		unsigned maxIndex = numContacts;
		for(i = 0; i < numContacts; i++) {
			float sepVel = contactArray[i].calculateSeparatingVelocity();
			if(sepVel < max && (sepVel < 0.0f || contactArray[i].penetration > 0.0f)) {
				max = sepVel;
				maxIndex = i;
			}
		}

		if(maxIndex == numContacts) {
			break;
		}

		contactArray[maxIndex].resolve(duration);

		glm::vec3 *move = contactArray[maxIndex].particleMovement;
        for (i = 0; i < numContacts; i++) {
            if (contactArray[i].particle[0] == contactArray[maxIndex].particle[0]) {
                contactArray[i].penetration -= glm::dot(move[0], contactArray[i].contactNormal);
            }
            else if (contactArray[i].particle[0] == contactArray[maxIndex].particle[1]) {
                contactArray[i].penetration -= glm::dot(move[1], contactArray[i].contactNormal);
            }
            if (contactArray[i].particle[1]) {
                if (contactArray[i].particle[1] == contactArray[maxIndex].particle[0]) {
                    contactArray[i].penetration += glm::dot(move[0], contactArray[i].contactNormal);
                }
                else if (contactArray[i].particle[1] == contactArray[maxIndex].particle[1]) {
                    contactArray[i].penetration += glm::dot(move[1], contactArray[i].contactNormal);
                }
            }
        }

		iterationsUsed++;
	}
}

/*
void ParticleContactResolver::resolveContacts(ParticleContact *contactArray, unsigned numContacts, float duration) {

	unsigned i;
	iterationsUsed = 0;
	while(iterationsUsed < iterations) {

		float max = std::numeric_limits<float>::max();
		unsigned maxIndex = numContacts;
		for(i = 0; i < numContacts; i++) {
			float sepVel = contactArray[i].calculateSeparatingVelocity();
			if(sepVel < max && (sepVel < 0.0f || contactArray[i].penetration > 0.0f)) {
				max = sepVel;
				maxIndex = i;
			}
		}

		if(maxIndex == numContacts) {
			break;
		}

		contactArray[maxIndex].resolve(duration);
		iterationsUsed++;
	}
}
*/