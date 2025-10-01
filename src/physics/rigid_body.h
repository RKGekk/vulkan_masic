#pragma once

#include <math.h>
#include <limits>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

static inline void _calculateTransformMatrix(glm::mat4 &transformMatrix, const glm::vec3 &position, const glm::quat &orientation) {
	transformMatrix[0][0] = 1.0f - 2.0f * orientation.y * orientation.y - 2.0f * orientation.z * orientation.z;
	transformMatrix[0][1] = 2.0f * orientation.x * orientation.y - 2.0f * orientation.w * orientation.z;
	transformMatrix[0][2] = 2.0f * orientation.x * orientation.z + 2.0f * orientation.w * orientation.y;
	transformMatrix[0][3] = position.x;

	transformMatrix[1][0] = 2.0f * orientation.x * orientation.y + 2.0f * orientation.w * orientation.z;
	transformMatrix[1][1] = 1.0f - 2.0f * orientation.x * orientation.x - 2.0f * orientation.z * orientation.z;
	transformMatrix[1][2] = 2.0f * orientation.y * orientation.z - 2.0f * orientation.w * orientation.x;
	transformMatrix[1][3] = position.y;

	transformMatrix[2][0] = 2.0f * orientation.x * orientation.z - 2.0f * orientation.w * orientation.y;
	transformMatrix[2][1] = 2.0f * orientation.y * orientation.z + 2.0f * orientation.w * orientation.x;
	transformMatrix[2][2] = 1.0f - 2.0f * orientation.x * orientation.x - 2.0f * orientation.y * orientation.y;
	transformMatrix[2][3] = position.z;

	transformMatrix[3][0] = 0.0f;
	transformMatrix[3][1] = 0.0f;
	transformMatrix[3][2] = 0.0f;
	transformMatrix[3][3] = 1.0f;
}

class RigidBody {

protected:

	const float zero = std::numeric_limits<float>::epsilon() + 0.0001f;
	const float maxMass = std::numeric_limits<float>::max();
	const float sleepEpsilon = 0.3f;

	float _inverseMass;
	glm::mat3 _inverseInertiaTensor;
	float _linearDamping;
	float _angularDamping;
	glm::vec3 _position;
	glm::quat _orientation;
	glm::vec3 _velocity;
	glm::vec3 _rotation;
	glm::mat3 _inverseInertiaTensorWorld;
	float _motion;
	bool _isAwake;
	bool _canSleep;
	glm::mat4 _transformMatrix;
	glm::vec3 _forceAccum;
	glm::vec3 _torqueAccum;
	glm::vec3 _acceleration;
	glm::vec3 _lastFrameAcceleration;

public:
	void calculateDerivedData();
	void setInertiaTensor(const glm::mat4 &inertiaTensor);
	void integrate(float duration);
	void setMass(const float mass);
	float getMass() const;
	void setInverseMass(const float inverseMass);
	float getInverseMass() const;
	bool hasFiniteMass() const;
	void setInertiaTensor(const glm::mat3 &inertiaTensor);
	void getInertiaTensor(glm::mat3 *inertiaTensor) const;
	glm::mat3 getInertiaTensor() const;
	void getInertiaTensorWorld(glm::mat3 *inertiaTensor) const;
	glm::mat3 getInertiaTensorWorld() const;
	void setInverseInertiaTensor(const glm::mat3 &inverseInertiaTensor);
	void getInverseInertiaTensor(glm::mat3 *inverseInertiaTensor) const;
	glm::mat3 getInverseInertiaTensor() const;
	void getInverseInertiaTensorWorld(glm::mat3 *inverseInertiaTensor) const;
	glm::mat3 getInverseInertiaTensorWorld() const;
	void setDamping(const float linearDamping, const float angularDamping);
	void setLinearDamping(const float linearDamping);
	float getLinearDamping() const;
	void setAngularDamping(const float angularDamping);
	float getAngularDamping() const;
	void setPosition(const glm::vec3 &position);
	void setPosition(const float x, const float y, const float z);
	void getPosition(glm::vec3 *position) const;
	glm::vec3 getPosition() const;
	void setOrientation(const glm::quat &orientation);
	void setOrientation(const float r, const float i, const float j, const float k);
	void getOrientation(glm::quat *orientation) const;
	glm::quat getOrientation() const;
	void getOrientation(glm::mat3 *matrix) const;
	void getOrientation(float matrix[9]) const;
	void getTransform(glm::mat4 *transform) const;
	void getTransform(float matrix[16]) const;
	void getGLTransform(float matrix[16]) const;
	glm::mat4 getTransform() const;
	glm::vec3 getPointInLocalSpace(const glm::vec3 &point) const;
	glm::vec3 getPointInWorldSpace(const glm::vec3 &point) const;
	glm::vec3 getDirectionInLocalSpace(const glm::vec3 &direction) const;
	glm::vec3 getDirectionInWorldSpace(const glm::vec3 &direction) const;
	void setVelocity(const glm::vec3 &velocity);
	void setVelocity(const float x, const float y, const float z);
	void getVelocity(glm::vec3 *velocity) const;
	glm::vec3 getVelocity() const;
	void addVelocity(const glm::vec3 &deltaVelocity);
	void setRotation(const glm::vec3 &rotation);
	void setRotation(const float x, const float y, const float z);
	void getRotation(glm::vec3 *rotation) const;
	glm::vec3 getRotation() const;
	void addRotation(const glm::vec3 &deltaRotation);
	bool getAwake() const {
		return _isAwake;
	}
	void setAwake(const bool awake = true);
	bool getCanSleep() const {
		return _canSleep;
	}
	void setCanSleep(const bool canSleep = true);
	void getLastFrameAcceleration(glm::vec3 *linearAcceleration) const;
	glm::vec3 getLastFrameAcceleration() const;
	void clearAccumulators();
	void addForce(const glm::vec3 &force);
	void addForceAtPoint(const glm::vec3 &force, const glm::vec3 &point);
	void addForceAtBodyPoint(const glm::vec3 &force, const glm::vec3 &point);
	void addTorque(const glm::vec3 &torque);
	void setAcceleration(const glm::vec3 &acceleration);
	void setAcceleration(const float x, const float y, const float z);
	void getAcceleration(glm::vec3 *acceleration) const;
	glm::vec3 getAcceleration() const;
};