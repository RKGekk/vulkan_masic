#include "rigid_body.h"

static inline void _checkInverseInertiaTensor(const glm::mat3 &iitWorld) {
	// TODO: Perform a validity check in an assert.
}

static inline void _transformInertiaTensor(glm::mat3 &iitWorld, const glm::quat &q, const glm::mat3 &iitBody, const glm::mat4 &rotmat) {

	float t4 = rotmat[0][0] * iitBody[0][0] + rotmat[0][1] * iitBody[1][0] + rotmat[0][2] * iitBody[2][0];
	float t9 = rotmat[0][0] * iitBody[0][1] + rotmat[0][1] * iitBody[1][1] + rotmat[0][2] * iitBody[2][1];
	float t14 = rotmat[0][0] * iitBody[0][2] + rotmat[0][1] * iitBody[1][2] + rotmat[0][2] * iitBody[2][2];
	float t28 = rotmat[1][0] * iitBody[0][0] + rotmat[1][1] * iitBody[1][0] + rotmat[1][2] * iitBody[2][0];
	float t33 = rotmat[1][0] * iitBody[0][1] + rotmat[1][1] * iitBody[1][1] + rotmat[1][2] * iitBody[2][1];
	float t38 = rotmat[1][0] * iitBody[0][2] + rotmat[1][1] * iitBody[1][2] + rotmat[1][2] * iitBody[2][2];
	float t52 = rotmat[2][0] * iitBody[0][0] + rotmat[2][1] * iitBody[1][0] + rotmat[2][2] * iitBody[2][0];
	float t57 = rotmat[2][0] * iitBody[0][1] + rotmat[2][1] * iitBody[1][1] + rotmat[2][2] * iitBody[2][1];
	float t62 = rotmat[2][0] * iitBody[0][2] + rotmat[2][1] * iitBody[1][2] + rotmat[2][2] * iitBody[2][2];

	iitWorld[0][0] = t4 * rotmat[0][0] + t9 * rotmat[0][1] + t14 * rotmat[0][2];
	iitWorld[0][1] = t4 * rotmat[1][0] + t9 * rotmat[1][1] + t14 * rotmat[1][2];
	iitWorld[0][2] = t4 * rotmat[2][0] + t9 * rotmat[2][1] + t14 * rotmat[2][2];
	iitWorld[1][0] = t28 * rotmat[0][0] + t33 * rotmat[0][1] + t38 * rotmat[0][2];
	iitWorld[1][1] = t28 * rotmat[1][0] + t33 * rotmat[1][1] + t38 * rotmat[1][2];
	iitWorld[1][2] = t28 * rotmat[2][0] + t33 * rotmat[2][1] + t38 * rotmat[2][2];
	iitWorld[2][0] = t52 * rotmat[0][0] + t57 * rotmat[0][1] + t62 * rotmat[0][2];
	iitWorld[2][1] = t52 * rotmat[1][0] + t57 * rotmat[1][1] + t62 * rotmat[1][2];
	iitWorld[2][2] = t52 * rotmat[2][0] + t57 * rotmat[2][1] + t62 * rotmat[2][2];
}

/*
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
}*/

void RigidBody::calculateDerivedData() {

	_orientation = glm::normalize(_orientation);
	_calculateTransformMatrix(_transformMatrix, _position, _orientation);
	_transformInertiaTensor(_inverseInertiaTensorWorld, _orientation, _inverseInertiaTensor, _transformMatrix);
}

void RigidBody::setInertiaTensor(const glm::mat4 &inertiaTensor) {
	_inverseInertiaTensor = glm::inverse(inertiaTensor);
}

void RigidBody::integrate(float duration) {
	if (!_isAwake) return;

	_lastFrameAcceleration = _acceleration;
	_lastFrameAcceleration += _forceAccum * _inverseMass;

	glm::vec3 angularAcceleration = _torqueAccum * _inverseInertiaTensorWorld;

	_velocity += _lastFrameAcceleration * duration;

	_rotation += angularAcceleration * duration;

	_velocity *= pow(_linearDamping, duration);
	_rotation *= pow(_angularDamping, duration);

	_position += _velocity * duration;

	glm::quat q(0.0f, _rotation.x * duration, _rotation.y * duration, _rotation.z * duration);
	q *= _orientation;
	_orientation.w += q.w * 0.5f;
	_orientation.x += q.x * 0.5f;
	_orientation.y += q.y * 0.5f;
	_orientation.z += q.z * 0.5f;

	calculateDerivedData();

	clearAccumulators();

	if (_canSleep) {
		float currentMotion = glm::dot(_velocity, _velocity) + glm::dot(_rotation, _rotation);

		float bias = pow(0.5, duration);
		_motion = bias * _motion + (1.0f - bias) * currentMotion;

		if (_motion < sleepEpsilon) {
			setAwake(false);
		}
		else if (_motion > 10.0f * sleepEpsilon) {
			_motion = 10.0f * sleepEpsilon;
		}
	}
}

void RigidBody::setMass(const float mass) {
	assert(abs(mass) >= zero);
	RigidBody::_inverseMass = 1.0f / mass;
}

float RigidBody::getMass() const {
	if (abs(_inverseMass) <= zero) {
		return maxMass;
	}
	else {
		return 1.0f / _inverseMass;
	}
}

void RigidBody::setInverseMass(const float inverseMass) {
	RigidBody::_inverseMass = inverseMass;
}

float RigidBody::getInverseMass() const {
	return _inverseMass;
}

bool RigidBody::hasFiniteMass() const {
	return _inverseMass >= zero;
}

void RigidBody::setInertiaTensor(const glm::mat3 &inertiaTensor) {
	_inverseInertiaTensor = glm::inverse(inertiaTensor);
	_checkInverseInertiaTensor(_inverseInertiaTensor);
}

void RigidBody::getInertiaTensor(glm::mat3 *inertiaTensor) const {
	(*inertiaTensor) = glm::inverse(_inverseInertiaTensor);
}

glm::mat3 RigidBody::getInertiaTensor() const {
	glm::mat3 it;
	getInertiaTensor(&it);
	return it;
}

void RigidBody::getInertiaTensorWorld(glm::mat3 *inertiaTensor) const {
	(*inertiaTensor) = glm::inverse(_inverseInertiaTensorWorld);
}

glm::mat3 RigidBody::getInertiaTensorWorld() const {
	glm::mat3 it;
	getInertiaTensorWorld(&it);
	return it;
}

void RigidBody::setInverseInertiaTensor(const glm::mat3 &inverseInertiaTensor) {
	_checkInverseInertiaTensor(inverseInertiaTensor);
	RigidBody::_inverseInertiaTensor = inverseInertiaTensor;
}

void RigidBody::getInverseInertiaTensor(glm::mat3 *inverseInertiaTensor) const {
	*inverseInertiaTensor = RigidBody::_inverseInertiaTensor;
}

glm::mat3 RigidBody::getInverseInertiaTensor() const {
	return _inverseInertiaTensor;
}

void RigidBody::getInverseInertiaTensorWorld(glm::mat3 *inverseInertiaTensor) const {
	*inverseInertiaTensor = _inverseInertiaTensorWorld;
}

glm::mat3 RigidBody::getInverseInertiaTensorWorld() const {
	return _inverseInertiaTensorWorld;
}

void RigidBody::setDamping(const float linearDamping, const float angularDamping) {
	RigidBody::_linearDamping = linearDamping;
	RigidBody::_angularDamping = angularDamping;
}

void RigidBody::setLinearDamping(const float linearDamping) {
	RigidBody::_linearDamping = linearDamping;
}

float RigidBody::getLinearDamping() const {
	return _linearDamping;
}

void RigidBody::setAngularDamping(const float angularDamping) {
	RigidBody::_angularDamping = angularDamping;
}

float RigidBody::getAngularDamping() const {
	return _angularDamping;
}

void RigidBody::setPosition(const glm::vec3 &position) {
	RigidBody::_position = position;
}

void RigidBody::setPosition(const float x, const float y, const float z) {
	_position.x = x;
	_position.y = y;
	_position.z = z;
}

void RigidBody::getPosition(glm::vec3 *position) const {
	*position = RigidBody::_position;
}

glm::vec3 RigidBody::getPosition() const {
	return _position;
}

void RigidBody::setOrientation(const glm::quat &orientation) {
	RigidBody::_orientation = glm::normalize(orientation);
}

void RigidBody::setOrientation(const float r, const float i, const float j, const float k) {
	_orientation.w = r;
	_orientation.x = i;
	_orientation.y = j;
	_orientation.z = k;
	_orientation = glm::normalize(_orientation);
}

void RigidBody::getOrientation(glm::quat *orientation) const {
	*orientation = RigidBody::_orientation;
}

glm::quat RigidBody::getOrientation() const {
	return _orientation;
}

void RigidBody::getOrientation(glm::mat3 *matrix) const {
	glm::vec3 a1 = (*matrix)[0];
	glm::vec3 a2 = (*matrix)[1];
	glm::vec3 a3 = (*matrix)[2];

	float mt[9];

	mt[0] = a1[0];
	mt[1] = a1[1];
	mt[2] = a1[2];

	mt[3] = a2[0];
	mt[4] = a2[1];
	mt[5] = a2[2];

	mt[6] = a3[0];
	mt[7] = a3[1];
	mt[8] = a3[2];

	getOrientation(mt);

	(*matrix)[0][0] = mt[0];
	(*matrix)[0][1] = mt[1];
	(*matrix)[0][2] = mt[2];

	(*matrix)[1][0] = mt[3];
	(*matrix)[1][1] = mt[4];
	(*matrix)[1][2] = mt[5];

	(*matrix)[2][0] = mt[6];
	(*matrix)[2][1] = mt[7];
	(*matrix)[2][2] = mt[8];
}

void RigidBody::getOrientation(float matrix[9]) const {
	matrix[0] = _transformMatrix[0][0];
	matrix[1] = _transformMatrix[0][1];
	matrix[2] = _transformMatrix[0][2];

	matrix[3] = _transformMatrix[1][0];
	matrix[4] = _transformMatrix[1][1];
	matrix[5] = _transformMatrix[1][2];

	matrix[6] = _transformMatrix[2][0];
	matrix[7] = _transformMatrix[2][1];
	matrix[8] = _transformMatrix[2][2];
}

void RigidBody::getTransform(glm::mat4 *transform) const {

	(*transform)[0][0] = _transformMatrix[0][0];
	(*transform)[0][1] = _transformMatrix[0][1];
	(*transform)[0][2] = _transformMatrix[0][2];
	(*transform)[0][3] = _transformMatrix[0][3];

	(*transform)[1][0] = _transformMatrix[1][0];
	(*transform)[1][1] = _transformMatrix[1][1];
	(*transform)[1][2] = _transformMatrix[1][2];
	(*transform)[1][3] = _transformMatrix[1][3];

	(*transform)[2][0] = _transformMatrix[2][0];
	(*transform)[2][1] = _transformMatrix[2][1];
	(*transform)[2][2] = _transformMatrix[2][2];
	(*transform)[2][3] = _transformMatrix[2][3];

	(*transform)[3][0] = _transformMatrix[3][0];
	(*transform)[3][1] = _transformMatrix[3][1];
	(*transform)[3][2] = _transformMatrix[3][2];
	(*transform)[3][3] = _transformMatrix[3][3];
}

void RigidBody::getTransform(float matrix[16]) const {

	matrix[0] = _transformMatrix[0][0];
	matrix[1] = _transformMatrix[0][1];
	matrix[2] = _transformMatrix[0][2];
	matrix[3] = _transformMatrix[0][3];

	matrix[4] = _transformMatrix[1][0];
	matrix[5] = _transformMatrix[1][1];
	matrix[6] = _transformMatrix[1][2];
	matrix[7] = _transformMatrix[1][3];

	matrix[8] = _transformMatrix[2][0];
	matrix[9] = _transformMatrix[2][1];
	matrix[10] = _transformMatrix[2][2];
	matrix[11] = _transformMatrix[2][3];

	matrix[12] = _transformMatrix[3][0];
	matrix[13] = _transformMatrix[3][1];
	matrix[14] = _transformMatrix[3][2];
	matrix[15] = _transformMatrix[3][3];
}

void RigidBody::getGLTransform(float matrix[16]) const {
	matrix[0] = (float)_transformMatrix[0][0];
	matrix[1] = (float)_transformMatrix[1][0];
	matrix[2] = (float)_transformMatrix[2][0];
	matrix[3] = 0;

	matrix[4] = (float)_transformMatrix[0][1];
	matrix[5] = (float)_transformMatrix[1][1];
	matrix[6] = (float)_transformMatrix[2][1];
	matrix[7] = 0;

	matrix[8] = (float)_transformMatrix[0][2];
	matrix[9] = (float)_transformMatrix[1][2];
	matrix[10] = (float)_transformMatrix[2][2];
	matrix[11] = 0;

	matrix[12] = (float)_transformMatrix[0][3];
	matrix[13] = (float)_transformMatrix[1][3];
	matrix[14] = (float)_transformMatrix[2][3];
	matrix[15] = 1;
}

glm::mat4 RigidBody::getTransform() const {
	return _transformMatrix;
}


glm::vec3 RigidBody::getPointInLocalSpace(const glm::vec3 &point) const {

	glm::mat4 invT = glm::inverse(_transformMatrix);
	glm::vec4 res = invT * glm::vec4(point, 1.0f);
	return glm::vec3(res.x, res.y, res.z);
}

glm::vec3 RigidBody::getPointInWorldSpace(const glm::vec3 &point) const {
	glm::vec4 res = _transformMatrix * glm::vec4(point, 1.0f);
	return glm::vec3(res.x, res.y, res.z);
}

glm::vec3 RigidBody::getDirectionInLocalSpace(const glm::vec3 &direction) const {
	glm::mat4 invT = glm::inverse(_transformMatrix);
	glm::vec4 res = invT * glm::vec4(direction, 1.0f);
	return glm::vec3(res.x, res.y, res.z);
}

glm::vec3 RigidBody::getDirectionInWorldSpace(const glm::vec3 &direction) const {
	glm::vec4 res = _transformMatrix * glm::vec4(direction, 1.0f);
	return glm::vec3(res.x, res.y, res.z);
}


void RigidBody::setVelocity(const glm::vec3 &velocity) {
	RigidBody::_velocity = velocity;
}

void RigidBody::setVelocity(const float x, const float y, const float z) {
	_velocity.x = x;
	_velocity.y = y;
	_velocity.z = z;
}

void RigidBody::getVelocity(glm::vec3 *velocity) const {
	*velocity = RigidBody::_velocity;
}

glm::vec3 RigidBody::getVelocity() const {
	return _velocity;
}

void RigidBody::addVelocity(const glm::vec3 &deltaVelocity) {
	_velocity += deltaVelocity;
}

void RigidBody::setRotation(const glm::vec3 &rotation) {
	RigidBody::_rotation = rotation;
}

void RigidBody::setRotation(const float x, const float y, const float z) {
	_rotation.x = x;
	_rotation.y = y;
	_rotation.z = z;
}

void RigidBody::getRotation(glm::vec3 *rotation) const {
	*rotation = RigidBody::_rotation;
}

glm::vec3 RigidBody::getRotation() const {
	return _rotation;
}

void RigidBody::addRotation(const glm::vec3 &deltaRotation) {
	_rotation += deltaRotation;
}

void RigidBody::setAwake(const bool awake) {
	if (awake) {
		_isAwake = true;

		// Add a bit of motion to avoid it falling asleep immediately.
		_motion = sleepEpsilon * 2.0f;
	}
	else {
		_isAwake = false;
		_velocity = glm::vec3();
		_rotation = glm::vec3();
	}
}

void RigidBody::setCanSleep(const bool canSleep) {
	RigidBody::_canSleep = canSleep;

	if (!canSleep && !_isAwake) {
		setAwake();
	}
}


void RigidBody::getLastFrameAcceleration(glm::vec3 *acceleration) const {
	*acceleration = _lastFrameAcceleration;
}

glm::vec3 RigidBody::getLastFrameAcceleration() const {
	return _lastFrameAcceleration;
}

void RigidBody::clearAccumulators() {
	_forceAccum = glm::vec3();
	_torqueAccum = glm::vec3();
}

void RigidBody::addForce(const glm::vec3 &force) {
	_forceAccum += force;
	_isAwake = true;
}

void RigidBody::addForceAtBodyPoint(const glm::vec3 &force, const glm::vec3 &point) {
	glm::vec3 pt = getPointInWorldSpace(point);
	addForceAtPoint(force, pt);
}

void RigidBody::addForceAtPoint(const glm::vec3 &force, const glm::vec3 &point) {

	glm::vec3 pt = point;
	pt -= _position;

	_forceAccum += force;
	_torqueAccum += pt * force;

	_isAwake = true;
}

void RigidBody::addTorque(const glm::vec3 &torque) {
	_torqueAccum += torque;
	_isAwake = true;
}

void RigidBody::setAcceleration(const glm::vec3 &acceleration) {
	RigidBody::_acceleration = acceleration;
}

void RigidBody::setAcceleration(const float x, const float y, const float z) {
	_acceleration.x = x;
	_acceleration.y = y;
	_acceleration.z = z;
}

void RigidBody::getAcceleration(glm::vec3 *acceleration) const {
	*acceleration = RigidBody::_acceleration;
}

glm::vec3 RigidBody::getAcceleration() const {
	return _acceleration;
}