#include "particle_force_registry.h"

void ParticleForceRegistry::updateForces(float duration) {

	Registry::iterator i = registrations.begin();
	Registry::iterator end = registrations.end();
	for(;i != end; i++) {
		i->forceGenerator->updateForce(i->particle, duration);
	}
}

void ParticleForceRegistry::add(Particle* particle, ParticleForceGenerator *forceGenerator) {

    ParticleForceRegistry::ParticleForceRegistration registration;
    registration.particle = particle;
    registration.forceGenerator = forceGenerator;
    registrations.push_back(registration);
}

void ParticleForceRegistry::remove(Particle *particle, ParticleForceGenerator *forceGenerator) {

    //registrations.
	Registry::iterator i = registrations.begin();
	Registry::iterator end = registrations.end();
	for(;i != end; i++) {
		ParticleForceRegistration pfr = *i;
		if(pfr.particle == particle && pfr.forceGenerator == forceGenerator) {
			registrations.erase(i);
			return;
		}
	}
}

void ParticleForceRegistry::clear() {
	registrations.clear();
}