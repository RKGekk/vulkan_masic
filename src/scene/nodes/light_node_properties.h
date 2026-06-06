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

struct LightNodeProperties {
    glm::vec3 strength;
    float falloff_start;    // point/spot light only
    glm::vec3 direction;    // directional/spot light only
    float falloff_end;      // point/spot light only
    glm::vec3 position;     // point light only
    float spot_power;       // spot light only
    float outer_angle;      // spot light only
    float inner_angle;      // spot light only
};