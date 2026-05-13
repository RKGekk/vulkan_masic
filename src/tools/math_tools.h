#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

// angularVelocity: vec3 representing rotation speed around X, Y, Z axes (radians/sec)
// currentQuat: the orientation at this keyframe
glm::quat calculateTangent(glm::vec3 angular_velocity, glm::quat current_quat);

// q_tangent: the 4D tangent output from a spline or derivative
// q_current: the current orientation (normalized)
glm::vec3 quatTangentToVec3(glm::quat q_tangent, glm::quat q_current);