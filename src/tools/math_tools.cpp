#include "math_tools.h"

// angularVelocity: vec3 representing rotation speed around X, Y, Z axes (radians/sec)
// currentQuat: the orientation at this keyframe
glm::quat calculateTangent(glm::vec3 angular_velocity, glm::quat current_quat) {
    
    // 1. Convert vec3 velocity to a "pure" quaternion (w = 0)
    glm::quat omega(0.0f, angular_velocity.x, angular_velocity.y, angular_velocity.z);
    
    // 2. Apply the kinematic formula(derivative of currentQuat): T = 0.5 * omega * q
    return 0.5f * omega * current_quat;
}

// q_tangent: the 4D tangent output from a spline or derivative
// q_current: the current orientation (normalized)
glm::vec3 quatTangentToVec3(glm::quat q_tangent, glm::quat q_current) {
    // 1. Calculate omega = 2 * dq * inv(q)
    // For unit quaternions, conjugate is the inverse
    glm::quat omegaQuat = 2.0f * q_tangent * glm::conjugate(q_current);
    
    // 2. The vec3 tangent (angular velocity) is the imaginary part
    return glm::vec3(omegaQuat.x, omegaQuat.y, omegaQuat.z);
}